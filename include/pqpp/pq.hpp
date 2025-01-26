/*
 * Code from https://github.com/Overv/pqpp with some edits for includes
*/

#ifndef PQ_HPP
#define PQ_HPP

#include <libpq-fe.h>

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>
#include <tuple>
#include <functional>

#include <sys/select.h>

namespace pq {
    using std::string;
    using std::vector;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::runtime_error;
    using std::function;
    using std::map;
    using std::to_string;
    using std::nullptr_t;
    using std::move;
    using std::tuple;
    using std::get;
    using std::make_tuple;

    // Container for PostgreSQL row values that allows for easy conversions
    class value {
    public:
        value() {}
        value(const string& val, bool null) : val(val), null(null) {}

        bool is_null() const {
            return null;
        }
        
        const string& str() const {
            return val;
        }

        template<typename T> T get() const {
            return static_cast<T>(val);
        }

        template <typename T> operator T() const {
            return get<T>();
        }

    private:
        string val;
        bool null;
    };

    template<> int value::get<int>() const {
        return std::stoi(val);
    }

    template<> long value::get<long>() const {
        return std::stol(val);
    }

    template<> long long value::get<long long>() const {
        return std::stoll(val);
    }

    template<> unsigned long value::get<unsigned long>() const {
        return std::stoul(val);
    }

    template<> unsigned long long value::get<unsigned long long>() const {
        return std::stoull(val);
    }

    template<> double value::get<double>() const {
        return std::stod(val);
    }

    template<> long double value::get<long double>() const {
        return std::stold(val);
    }

    template<> float value::get<float>() const {
        return std::stof(val);
    }

    template<> bool value::get<bool>() const {
        return val != "false" && val != "";
    }

    // Notification
    class notification {
    public:
        notification(string channel, string payload) : channel(channel), payload(payload) {}

        const string& get_channel() const { return channel; }
        const string& get_payload() const { return payload; }

    private:
        string channel, payload;
    };

    // Prepared statement
    class prepared_statement {
    public:
        prepared_statement(const string& name, int parameters) : name(name), parameters(parameters) {}

        const string& get_name() const { return name; }
        int get_parameters() const { return parameters; }

    private:
        string name;
        int parameters;
    };

    // Row is represented as hash table with column names
    typedef map<string, value> row_t;

    class connection {
    public:
        connection(const string& connInfo) {
            conn = shared_ptr<PGconn>(PQconnectdb(connInfo.c_str()), [=](PGconn* conn) {
                PQfinish(conn);
            });

            if (PQstatus(conn.get()) != CONNECTION_OK) {
                throw runtime_error(PQerrorMessage(conn.get()));
            }
        }
        
        connection(const vector<tuple<string, string>>& params) {
            connect_params(params);
        }
        
        connection(const string& host, const string& db, const string& user, const string& pass) {
            vector<tuple<string, string>> params = {
                make_tuple("host", host),
                make_tuple("dbname", db),
                make_tuple("user", user),
                make_tuple("password", pass)
            };
            
            connect_params(params);
        }

        template<typename... Args>
        vector<row_t> exec(const string& query, Args... param_args) {
            vector<value> args = make_value_list(param_args...);
            vector<const char*> pq_args = make_pq_args(args);

            auto tmp = PQexecParams(conn.get(), query.c_str(), pq_args.size(), nullptr, pq_args.data(), nullptr, nullptr, 0);
            auto res = unique_ptr<PGresult, function<void(PGresult*)>>(tmp, [=](PGresult* res) {
                PQclear(res);
            });

            return get_rows(move(res));
        }

        template<typename... Args>
        vector<row_t> exec(const prepared_statement& stmt, Args... param_args) {
            vector<value> args = make_value_list(param_args...);
            vector<const char*> pq_args = make_pq_args(args);

            auto tmp = PQexecPrepared(conn.get(), stmt.get_name().c_str(), stmt.get_parameters(), pq_args.data(), nullptr, nullptr, 0);
            auto res = unique_ptr<PGresult, function<void(PGresult*)>>(tmp, [=](PGresult* res) {
                PQclear(res);
            });

            return get_rows(move(res));
        }

        prepared_statement prepare(const string& name, const string& query, int parameters) {
            auto tmp = PQprepare(conn.get(), name.c_str(), query.c_str(), parameters, nullptr);
            auto res = unique_ptr<PGresult, function<void(PGresult*)>>(tmp, [=](PGresult* res) {
                PQclear(res);
            });

            if (PQresultStatus(res.get()) == PGRES_COMMAND_OK) {
                return prepared_statement(name, parameters);
            } else {
                throw runtime_error(PQerrorMessage(conn.get()));
            }
        }

        vector<notification> get_notifications(bool wait = false) {
            // Poll server for notifications
            PQconsumeInput(conn.get());

            // Get latest notifications
            vector<notification> notifications;

            unique_ptr<PGnotify, function<void(PGnotify*)>> raw_notification(nullptr, [=](PGnotify* notification) {
                PQfreemem(notification);
            });

            do {
                raw_notification.reset(PQnotifies(conn.get()));

                if (raw_notification != nullptr) {
                    notifications.push_back(notification(raw_notification->relname, raw_notification->extra));
                }
            } while (raw_notification != nullptr);

            // If there were no notifications, wait for them if requested
            if (notifications.size() == 0 && wait) {
                auto sock = PQsocket(conn.get());

                fd_set mask;
                FD_ZERO(&mask);
                FD_SET(sock, &mask);

                select(sock + 1, &mask, nullptr, nullptr, nullptr);

                // Try again
                return get_notifications(wait);
            }

            return notifications;
        }

    private:
        shared_ptr<PGconn> conn;

        // Warning: result is only valid as long as arguments is in scope
        vector<const char*> make_pq_args(const vector<value>& arguments) {
            vector<const char*> pq_args;

            for (auto& val : arguments) {
                if (val.is_null()) {
                    pq_args.push_back(nullptr);
                } else {
                    pq_args.push_back(val.str().c_str());
                }
            }

            return pq_args;
        }

        vector<row_t> get_rows(unique_ptr<PGresult, function<void(PGresult*)>>&& res) {
            int status = PQresultStatus(res.get());

            if (status == PGRES_COMMAND_OK) {
                return vector<row_t>();
            } else if (status == PGRES_TUPLES_OK) {
                // Determine columns in result
                map<string, int> columnIds;

                int columnCount = PQnfields(res.get());
                for (int c = 0; c < columnCount; c++) {
                    columnIds.emplace(PQfname(res.get(), c), c);
                }

                // Extract rows
                vector<row_t> rows;

                int rowCount = PQntuples(res.get());
                for (int r = 0; r < rowCount; r++) {
                    row_t row;

                    for (auto& column : columnIds) {
                        string val = PQgetvalue(res.get(), r, column.second);
                        bool isNull = PQgetisnull(res.get(), r, column.second);

                        row.emplace(column.first, value(val, isNull));
                    }

                    rows.push_back(row);
                }

                return rows;
            } else {
                throw runtime_error(PQerrorMessage(conn.get()));
            }
        }
        
        void connect_params(const vector<tuple<string, string>>& params) {
            vector<const char*> keywords;
            vector<const char*> values;
            
            for (auto& tuple : params) {
                keywords.push_back(get<0>(tuple).c_str());
                values.push_back(get<1>(tuple).c_str());
            }
            
            keywords.push_back(nullptr);
            values.push_back(nullptr);
            
            conn = shared_ptr<PGconn>(PQconnectdbParams(keywords.data(), values.data(), false), [=](PGconn* conn) {
                PQfinish(conn);
            });
            
            if (PQstatus(conn.get()) != CONNECTION_OK) {
                throw runtime_error(PQerrorMessage(conn.get()));
            }
        }

        // Helper for parameterized queries
        static void make_value_list(vector<value>& list) {}

        template<typename T, typename... Args>
        static void make_value_list(vector<value>& list, T n, Args... rest) {
            list.push_back(value(to_string(n), false));
            make_value_list(list, rest...);
        }

        template<typename... Args>
        static void make_value_list(vector<value>& list, const char* str, Args... rest) {
            list.push_back(value(str, false));
            make_value_list(list, rest...);
        }

        template<typename... Args>
        static void make_value_list(vector<value>& list, const string& str, Args... rest) {
            list.push_back(value(str, false));
            make_value_list(list, rest...);
        }

        template<typename... Args>
        static void make_value_list(vector<value>& list, nullptr_t null, Args... rest) {
            list.push_back(value("", true));
            make_value_list(list, rest...);
        }

        template<typename... Args>
        static vector<value> make_value_list(Args... rest) {
            vector<value> list;
            make_value_list(list, rest...);
            return list;
        }

        static vector<value> make_value_list(const vector<string>& args) {
            vector<value> list;

            for (auto& str : args) {
                list.push_back(value(str, false));
            }

            return list;
        }
    };
}

#endif
