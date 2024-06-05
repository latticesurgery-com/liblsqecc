#include <lsqecc/patches/slice_stats.hpp>

namespace lsqecc {

std::ostream& operator<<(std::ostream& os, const SliceStats& slice_stats)
{
    auto get_percentage_volume = [&](size_t x) -> double {
                    return static_cast<double>(x) / static_cast<double>(slice_stats.totals.volume) * 100.0;
    };
    os << "Total volume: " << slice_stats.totals.volume << std::endl;
    os << "Distillation volume: " 
        << slice_stats.totals.distillation_volume 
        << " (" << get_percentage_volume(slice_stats.totals.distillation_volume) << "%)" << std::endl;
    os << "Unused routing volume: " 
        << slice_stats.totals.unused_routing_volume 
        << " (" << get_percentage_volume(slice_stats.totals.unused_routing_volume) << "%)" << std::endl;
    os << "Dead volume: "
        << slice_stats.totals.dead
        << " (" << get_percentage_volume(slice_stats.totals.dead) << "%)" << std::endl;
    auto other_volume = slice_stats.totals.volume - slice_stats.totals.unused_routing_volume - slice_stats.totals.distillation_volume - slice_stats.totals.dead;
    os << "Other active volume: " 
        << other_volume
        << " (" << get_percentage_volume(other_volume) << "%)";
    return os;
}

VolumeCounts operator+=(VolumeCounts& lhs, const VolumeCounts& rhs)
{
    lhs.volume += rhs.volume;
    lhs.unused_routing_volume += rhs.unused_routing_volume;
    lhs.distillation_volume += rhs.distillation_volume;
    lhs.dead += rhs.dead;
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
            // Where we reserve tiles for magic states instead of specifying factories,
            //  we do not count PreparedStates as part of the logical volume unless requested
            else if (slice.layout.get().magic_states_reserved() && 
                (cell->type == PatchType::PreparedState))
                counts.distillation_volume += 1;
            else if (cell->type == PatchType::Dead)
                counts.dead += 1;
        }
    }
    return counts;
}


} // namespace lsqecc