#ifndef LSQECC_WAVE_SCHEDULER
#define LSQECC_WAVE_SCHEDULER


#include <cstdint>
#include <vector>

#include <lsqecc/layout/layout.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/patches/dense_patch_computation.hpp>

namespace lsqecc {


class WaveScheduler
{
public:
	
	WaveScheduler(LSInstructionStream&& stream, bool local_instructions, const Layout& layout);
	
	bool done() const { return current_wave_.high_priority_heads.empty() && current_wave_.heads.empty(); }
	void schedule_wave(DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res);
	
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
		std::vector<InstructionID> high_priority_heads;
				
		void clear()
		{
			heads.clear();
			high_priority_heads.clear();
		}
		
		size_t size() const { return heads.size() + high_priority_heads.size(); }
	};
	
	void schedule_instructions(const std::vector<InstructionID>& instruction_ids, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res);
	void schedule_dependent_instructions(InstructionID instruction_id, const std::vector<LSInstruction>& followup_instructions, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res);
	
	bool is_immediate(const LSInstruction& instruction);
	bool try_schedule_immediately(InstructionID instruction_id, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res);
	
	bool local_instructions_;
	const Layout& layout_;
	CustomDPRouter router_;
	
	std::vector<InstructionRecord> records_;
	std::vector<uint8_t> dependency_counts_;
	
	Wave current_wave_, next_wave_;
};

}


#endif //LSQECC_WAVE_SCHEDULER
