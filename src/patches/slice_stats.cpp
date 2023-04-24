#include <lsqecc/patches/slice_stats.hpp>

namespace lsqecc {


VolumeCounts operator+=(VolumeCounts& lhs, const VolumeCounts& rhs)
{
    lhs.volume += rhs.volume;
    lhs.unused_routing_volume += rhs.unused_routing_volume;
    lhs.distillation_volume += rhs.distillation_volume;
    return lhs;
}


VolumeCounts compute_volume_counts(const DenseSlice& slice)
{
    VolumeCounts counts;
    counts.volume = (slice.layout.get().furthest_cell().row+1) * (slice.layout.get().furthest_cell().col+1);
    for (const auto& row : slice.cells) 
    {
        for (const auto& cell : row)
        {
            if (!cell.has_value())
                counts.unused_routing_volume += 1; 
            else if (cell->type == PatchType::Distillation) 
                counts.distillation_volume += 1;
        }
    }
    return counts;
}


} // namespace lsqecc