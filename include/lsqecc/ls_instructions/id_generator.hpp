#ifndef LSQECC_ID_GENERATOR_HPP
#define LSQECC_ID_GENERATOR_HPP

#include <lsqecc/patches/patches.hpp>

namespace lsqecc
{

class IdGenerator
{
public:

    void set_start(PatchId start)
    {
        counter_ = start;
    }

    PatchId new_id()
    {
        return counter_++;
    }

    [[nodiscard]] PatchId last_given_id() const
    {
        return counter_;
    }
private:
    PatchId counter_ = 0;
};

}

#endif //LSQECC_ID_GENERATOR_HPP
