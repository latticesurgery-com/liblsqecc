#pragma once

#include <cstdint>
#include <cstddef>

#include <lsqecc/patches/dense_slice.hpp>

namespace lsqecc {


struct VolumeCounts
{
    size_t volume = 0;
    size_t unused_routing_volume = 0;
    size_t distillation_volume = 0;
    size_t dead = 0;
};

VolumeCounts operator+=(VolumeCounts& lhs, const VolumeCounts& rhs);

struct SliceStats
{
    VolumeCounts totals;
    // More metrics to go here, e.g. time between magic state requests etc.
};

std::ostream& operator<<(std::ostream& os, const SliceStats& slice_stats);

VolumeCounts compute_volume_counts(const DenseSlice& slice);


} // namespace lsqecc