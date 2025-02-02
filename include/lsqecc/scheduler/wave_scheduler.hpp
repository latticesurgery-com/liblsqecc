#ifndef LSQECC_WAVE_SCHEDULER
#define LSQECC_WAVE_SCHEDULER


#include <cstdint>
#include <vector>

#include <lstk/lstk.hpp>

#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/patches/dense_patch_computation.hpp>

namespace lsqecc {

struct WaveStats
{
	size_t wave_size; // number of instructions in wave
	size_t applied_wave_size; // number of instructions in wave that were actually applied
};

class WaveScheduler
{
public:
	
	WaveScheduler(LSInstructionStream&& stream, bool local_instructions, bool allow_twists, bool gen_op_ids, const Layout& layout, std::unique_ptr<Router> router, PipelineMode pipeline_mode);
	
	bool done() const { return current_wave_.proximate_heads_.empty() && current_wave_.heads.empty(); }
	WaveStats schedule_wave(DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res);
	
private:
	
	using InstructionID = uint32_t;
	
	struct InstructionRecord
	{
		LSInstruction instruction;
		std::vector<InstructionID> dependents;
	};

	struct Wave
	{
		std::vector<InstructionID> heads;
		std::vector<InstructionID> proximate_heads_;
		std::vector<InstructionID> deferred_to_end;
				
		void clear()
		{
			heads.clear();
			proximate_heads_.clear();
			deferred_to_end.clear();
		}

		std::vector<InstructionID> get_deferred() const
		{
			return deferred_to_end;
		}
		
		size_t size() const { return heads.size() + proximate_heads_.size(); }
	};
	
	// returns number of instruction_ids that were applied
	size_t schedule_instructions(const std::vector<InstructionID>& instruction_ids, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res, bool proximate, bool deferred);
	void schedule_dependent_instructions(InstructionID instruction_id, const std::vector<LSInstruction>& followup_instructions, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res, bool proximate);
	
	bool is_immediate(const LSInstruction& instruction);
	bool try_schedule_immediately(InstructionID instruction_id, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res, bool proximate);
	
	bool local_instructions_;
	bool allow_twists_;
	bool gen_op_ids_;
	const Layout& layout_;
	std::unique_ptr<Router> router_;
	
	std::vector<InstructionRecord> records_;
	std::vector<uint8_t> dependency_counts_;
	
	Wave current_wave_, next_wave_;
};

}


#endif //LSQECC_WAVE_SCHEDULER
