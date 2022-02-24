/**
 * Utility toolkit for the lsqecc package
 */

#ifndef LSQECC_LSTK_HPP
#define LSQECC_LSTK_HPP


#define LSTK_NOOP static_cast<void>(0)


namespace lstk {

using FloatSeconds = std::chrono::duration<double, std::ratio<1>>;

template<
        class result_t   = FloatSeconds,
        class clock_t    = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now()-start);
}

}

#endif //LSQECC_LSTK_HPP
