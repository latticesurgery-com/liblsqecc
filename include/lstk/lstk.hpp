/**
 * Utility toolkit for the lsqecc package
 */

#ifndef LSQECC_LSTK_HPP
#define LSQECC_LSTK_HPP


#define LSTK_NOOP static_cast<void>(0)
#define LSTK_UNUSED(X) static_cast<void>(X)

#include <vector>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <chrono>

#define LSTK_UNREACHABLE throw std::logic_error(std::string{"Meant to be unreachable: "}+__FILE__+":"+std::to_string(__LINE__))

namespace lstk {

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
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now()-start);
}

template<
        class clock_t    = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds
>
double seconds_since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<FloatSeconds>(clock_t::now()-start).count();
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

    auto push_accum_to_ret = [&](){
        ret.emplace_back(accum_begin, static_cast<size_t>(std::distance(accum_begin,accum_end)));
    };

    while(accum_end != s.end())
    {
        if(*accum_end == delim)
        {
            push_accum_to_ret();
            accum_end++; // skip the delim
            accum_begin = accum_end;
        }
        else
            accum_end++;
    }

    if(accum_begin!=accum_end)
        push_accum_to_ret();

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


}

#endif //LSQECC_LSTK_HPP
