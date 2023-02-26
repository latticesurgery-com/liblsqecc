/**
 * Utility toolkit for the lsqecc package
 */

#ifndef LSQECC_LSTK_HPP
#define LSQECC_LSTK_HPP


#define LSTK_NOOP static_cast<void>(0)
#define LSTK_UNUSED(X) static_cast<void>(X)

#include <vector>
#include <string>
#include <queue>
#include <string_view>
#include <functional>
#include <optional>
#include <stdexcept>
#include <chrono>
#include <sstream>

#define LSTK_UNREACHABLE throw std::logic_error(std::string{"Meant to be unreachable: "}+__FILE__+":"+std::to_string(__LINE__))
#define LSTK_NOT_IMPLEMENTED throw std::logic_error(std::string{"Not implemented: "}+__FILE__+":"+std::to_string(__LINE__))

namespace lstk
{

using bool8 = uint8_t;


using FloatSeconds = std::chrono::duration<double, std::ratio<1>>;

template<class T>
using delayed_init = std::optional<T>;


// Timing helpers

template<
        class result_t   = FloatSeconds,
        class clock_t    = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const &start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

template<
        class clock_t    = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds
>
double seconds_since(std::chrono::time_point<clock_t, duration_t> const &start)
{
    return std::chrono::duration_cast<FloatSeconds>(clock_t::now() - start).count();
}

inline auto now()
{
    return std::chrono::steady_clock::now();
}


// Parsing helpers

static inline std::vector<std::string_view> split_on(std::string_view s, char delim)
{
    std::vector<std::string_view> ret;

    auto accum_begin = s.begin();
    auto accum_end = s.begin();

    auto push_accum_to_ret = [&]()
    {
        ret.emplace_back(accum_begin, static_cast<size_t>(std::distance(accum_begin, accum_end)));
    };

    while (accum_end != s.end())
    {
        if (*accum_end == delim)
        {
            push_accum_to_ret();
            accum_end++; // skip the delim
            accum_begin = accum_end;
        } else
            accum_end++;
    }

    if (accum_begin != accum_end)
        push_accum_to_ret();

    // Add a trailing empty string when finishing on a delimiter
    else if(accum_begin > s.begin() && *(accum_end-1)== delim) ret.push_back("");

    return ret;
}

static inline std::vector<std::string_view> split_on(std::string_view s, std::string_view delim)
{
    std::vector<std::string_view> ret;

    auto accum_begin = s.begin();
    auto accum_end = s.begin();

    auto push_accum_to_ret = [&]()
    {
        ret.emplace_back(accum_begin, static_cast<size_t>(std::distance(accum_begin, accum_end)));
    };

    while (accum_end != s.end())
    {
        if (std::equal(delim.begin(), delim.end(), accum_end))
        {
            push_accum_to_ret();
            accum_end += delim.size(); // skip the delim
            accum_begin = accum_end;
        } else
            accum_end++;
    }

    if (accum_begin != accum_end)
        push_accum_to_ret();

    // Add a trailing empty string when finishing on a delimiter
    else if(accum_begin > s.begin() && std::equal(delim.begin(), delim.end(), accum_end)) ret.push_back("");

    return ret;
}


static inline std::vector<std::string> split_on_get_strings(std::string_view s, char delim)
{
    std::vector<std::string> ret;
    auto sub_strs = split_on(s, delim);
    for (const auto &item : sub_strs)
        ret.emplace_back(item);
    return ret;
}

static inline bool contains(std::string_view s, char target)
{
    return s.find(target) != std::string_view::npos;
}

// String manipulation

template <typename, typename = void>
struct has_ostream_operator : std::false_type {};

template <typename T>
struct has_ostream_operator<T, decltype(void(std::declval<std::ostream&>() << std::declval<const T&>()))>
        : std::true_type {};

template<class Stringifyable>
std::string cat(Stringifyable &&s)
{
    if constexpr (std::is_arithmetic_v<std::remove_reference_t<Stringifyable>>)
        return std::to_string(s);
    else if constexpr(has_ostream_operator<Stringifyable>::value)
    {
        std::stringstream ss;
        ss << s;
        return ss.str();
    }
    else
        return std::string{s};
}

template<class Stringifyable, class ...Args>
std::string cat(Stringifyable &&s, Args &&... args)
{
    return cat(std::forward<Stringifyable>(s)) + cat(std::forward<Args>(args)...);
}


static inline std::string str_replace(std::string_view s, char target, std::string_view replace_with)
{
    if(!contains(s, target))
        return std::string{s};
    std::stringstream out;
    while(contains(s,target))
    {
        out << s.substr(0,s.find(target)) << replace_with;
        s = s.substr(s.find(target)+1,s.length());
    }
    return out.str();
}

// Vector operations

template<class T>
void vector_extend(std::vector<T> &target, const std::vector<T> &extend_with)
{
    target.insert(target.end(), extend_with.begin(), extend_with.end());
}

template<class T>
T queue_pop(std::queue<T>& queue)
{
    T v{std::move(queue.front())};
    queue.pop();
    return v;
}

// Leaves source empty
template<class T>
void queue_extend(std::queue<T>& target, std::queue<T>& source)
{
    while(!source.empty()) target.push(queue_pop(source));
}


template<class IterableOfStringifiables>
std::string join(IterableOfStringifiables iterable_of_stringifiables, std::string_view separator)
{
    std::string acc;
    bool begin = true;
    for(const auto& v: iterable_of_stringifiables)
    {
        if(begin) acc = v;
        else acc = lstk::cat(acc, separator, v);
        begin = false;
    }
    return acc;
}

}


// Variant visitor helper
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;



} // namespace lstk

#define LSTK_THROW(exception_name, args) \
throw exception_name{lstk::cat("Exception at ",__FILE__,":",__LINE__,args)}

#endif //LSQECC_LSTK_HPP
