#ifndef LSQECC_SLICE_HPP
#define LSQECC_SLICE_HPP



#include <lsqecc/patches/patches.hpp>
#include <lsqecc/layout/layout.hpp>

#include <queue>
#include <functional>

namespace lsqecc {

template <class SliceImpl> // CRTP Interface
struct Slice{

public:

    bool has_patch(PatchId id) const
    {
        return static_cast<const SliceImpl*>(this)->has_patch(id);
    }

    Slice<SliceImpl>& get_patch_by_id_mut(PatchId id)
    {
        return static_cast<Slice<SliceImpl>&>(static_cast<SliceImpl*>(this)->get_patch_by_id_mut(id));
    }

    const Slice<SliceImpl>& get_patch_by_id(PatchId id) const
    {
        return static_cast<const Slice<SliceImpl>&>(static_cast<const SliceImpl*>(this)->get_patch_by_id(id));
    }

    void delete_qubit_patch(PatchId id)
    {
        static_cast<SliceImpl*>(this)->delete_qubit_patch(id);
    }

    const SingleCellOccupiedByPatch& get_single_cell_occupied_by_patch_by_id(PatchId id) const
    {
        return static_cast<const SliceImpl*>(this)->get_single_cell_occupied_by_patch_by_id(id);
    }

    SingleCellOccupiedByPatch& get_single_cell_occupied_by_patch_by_id_mut(PatchId id)
    {
        return static_cast<SliceImpl*>(this)->get_single_cell_occupied_by_patch_by_id_mut(id);
    }


    bool is_cell_free(const Cell& cell) const
    {
        return static_cast<const SliceImpl*>(this);
    }

    std::vector<Cell> get_neigbours_within_slice(const Cell& cell) const
    {
        return static_cast<const SliceImpl*>(this)->get_neigbours_within_slice();
    }

    static Slice<SliceImpl> make_blank_slice(const Layout& layout)
    {
        return static_cast<Slice<SliceImpl>>(SliceImpl::make_blank_slice(layout));
    }

private:
    Slice<SliceImpl>() = default;
    friend SliceImpl;
};


}


#endif //LSQECC_SLICE_HPP
