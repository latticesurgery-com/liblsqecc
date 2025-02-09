#include <lsqecc/scheduler/wave_scheduler.hpp>

#include <algorithm>
#include <cassert>
#include <unordered_map>


namespace lsqecc {


WaveScheduler::WaveScheduler(LSInstructionStream&& stream, bool local_instructions, bool allow_twists, bool gen_op_ids, const Layout& layout, std::unique_ptr<Router> router, PipelineMode pipeline_mode):
	local_instructions_(local_instructions),
	allow_twists_(allow_twists),
	gen_op_ids_(gen_op_ids),
	layout_(layout)
{
	router_ = std::move(router);
	if (pipeline_mode == PipelineMode::EDPC)
	{
		router_->set_EDPC();
	}	
	
	std::unordered_map<PatchId, InstructionID> patch_id_to_last_instruction;
	
	while (stream.has_next_instruction())
	{
		auto instruction = stream.get_next_instruction();
		InstructionID instruction_id = uint32_t(records_.size());
		assert(records_.size() < UINT32_MAX);
		records_.push_back({instruction, {}});
		
		auto operating_patches = instruction.get_patch_dependencies();
		assert(operating_patches.size() <= UINT8_MAX);
		
		uint8_t dependency_count = 0;
		
		bool is_head = true;
		
		for (auto patch_id : operating_patches) {
			auto p = patch_id_to_last_instruction.try_emplace(patch_id, 0);
			auto& last_instruction_on_patch = p.first->second;
			
			if (!p.second) // there was a previous instruction on this patch
			{
				++dependency_count;
				records_[last_instruction_on_patch].dependents.push_back(instruction_id);
				is_head = false;
			}
			
			last_instruction_on_patch = instruction_id;
		}
		
		dependency_counts_.push_back(dependency_count);
		
		if (is_head)
		{
			current_wave_.heads.push_back(instruction_id);	
		}
	}
}
	
WaveStats WaveScheduler::schedule_wave(DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res)
{
	size_t applied_count = 0;

	// std::cerr << "PROXIMATE HEADS" << std::endl;
	applied_count += schedule_instructions(current_wave_.proximate_heads_, slice, instruction_visitor, res, true, false);
	// std::cerr << "HEADS" << std::endl;
	applied_count += schedule_instructions(current_wave_.heads, slice, instruction_visitor, res, false, false);

	if (router_->get_EDPC())
	{
		/* After the routing of edge-disjoint paths for all BellBasedCNOTs in a wave, they are deferred because their specific decomposition depends on the others.
		   Once all BellBasedCNOTs in a wave have been seen, they are each decomposed into local instructions and then deferred once more (decomposition algorithm requires slice to have EDPC info retained).
		   Finally, on the third encounter, the first layer of instructions is scheduled, and each instruction reschedules itself.
		*/

		// std::cerr << "DEFERRED ROUND 1" << std::endl;
		applied_count += schedule_instructions(current_wave_.get_deferred(), slice, instruction_visitor, res, false, true);

		// Reset boundaries for data qubits
		for (const auto& bdry : slice.marked_rough_boundaries_EDPC)
		{
			bdry.get().boundary_type = BoundaryType::Rough;
		}
		for (const auto& bdry : slice.marked_smooth_boundaries_EDPC)
		{
			bdry.get().boundary_type = BoundaryType::Smooth;
		}

		slice.marked_rough_boundaries_EDPC.clear();
		slice.marked_smooth_boundaries_EDPC.clear();

		// std::cerr << "DEFERRED ROUND 2" << std::endl;
		applied_count += schedule_instructions(current_wave_.get_deferred(), slice, instruction_visitor, res, false, true);
	}
	
	WaveStats wave_stats = { .wave_size = current_wave_.size(), .applied_wave_size = applied_count };
	
	std::swap(current_wave_, next_wave_);
    next_wave_.clear();
    
	// std::cerr << "END OF WAVE" << std::endl;
    return wave_stats;
}

size_t WaveScheduler::schedule_instructions(const std::vector<InstructionID>& instruction_ids, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res, bool proximate, bool deferred)
{
	size_t applied_count = 0;
	
	for (auto instruction_id : instruction_ids)
	{
		assert(dependency_counts_[instruction_id] == 0);
		
		auto& instruction = records_[instruction_id].instruction;

		// std::cerr << "Applying instruction: " << instruction << std::endl;
		auto application_result = try_apply_instruction_direct_followup(slice, instruction, local_instructions_, allow_twists_, gen_op_ids_, layout_, *router_);
		// std::cerr << "Applied instruction: " << instruction << std::endl;

		if (!application_result.maybe_error)
		{   
			++applied_count;
		    ++res.ls_instructions_count_;
		    instruction_visitor(instruction);

			if (application_result.followup_instructions.size() == 1 && application_result.followup_instructions[0] == instruction) // instruction has rescheduled itself
				next_wave_.proximate_heads_.push_back(instruction_id);
			else
			{
				assert(!deferred); // Applied deferred instructions should always fall into the case above
				schedule_dependent_instructions(instruction_id, application_result.followup_instructions, slice, instruction_visitor, res, proximate);
			}
		}
		else
		{
			// This should only be touched in the case of EDPC compilation where local compilation cannot be performed until the whole EDP set is constructed
			if (application_result.followup_instructions.size() == 1 && application_result.followup_instructions[0] == instruction)
			{
				assert(router_->get_EDPC());
				current_wave_.deferred_to_end.push_back(instruction_id);
				continue;
			}

			// Deferred instructions get decomposed into local instructions and return an error.
			else if (deferred)
			{
				assert(router_->get_EDPC());
				continue;
			}

			if (proximate)
                throw std::runtime_error{lstk::cat(
                    "Could not apply proximate instruction:\n",
                    instruction,"\n",
                    "Caused by:\n",
                    application_result.maybe_error->what())};
            
		    if (instruction.wait_at_most_for == 0)
	            throw std::runtime_error{lstk::cat(
	                "Could not apply instruction after max retries:\n",
	                instruction,"\n",
	                "Caused by:\n",
	                application_result.maybe_error->what())};
		    
		    --instruction.wait_at_most_for;
		    next_wave_.heads.push_back(instruction_id);
		}
	}
	
	return applied_count;
}
	
void WaveScheduler::schedule_dependent_instructions(InstructionID instruction_id, const std::vector<LSInstruction>& followup_instructions, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res, bool proximate)
{
	auto& dependents = records_[instruction_id].dependents;
	
	if (followup_instructions.empty())
	{
		for (auto dependent : dependents)
		{
			assert(dependency_counts_[dependent] > 0);
			--dependency_counts_[dependent];
			if (dependency_counts_[dependent] == 0)
			{
				if (proximate)
				{
					auto& instruction = records_[dependent].instruction;
					if (is_immediate(instruction))
						current_wave_.heads.push_back(dependent);
					else
						next_wave_.heads.push_back(dependent);
				}
					
				else
				{
					if (!try_schedule_immediately(dependent, slice, instruction_visitor, res, proximate))
						next_wave_.heads.push_back(dependent);
				}
			}
		}
	}
	else
	{
		auto dependency_delta = followup_instructions.size() - 1;
		if (dependency_delta != 0)
		{
			for (auto dependent : dependents)
			{
				assert(dependency_counts_[dependent] + dependency_delta <= UINT8_MAX);
				dependency_counts_[dependent] += dependency_delta;
			}
		}
		
	    for (auto&& followup : followup_instructions)
	    {
	    	auto followup_id = records_.size();
	    	assert(records_.size() < UINT32_MAX);
	    	records_.push_back({followup, dependents});
	    	dependency_counts_.push_back(0);
	    	
	    	if (!try_schedule_immediately(followup_id, slice, instruction_visitor, res, proximate))
	    		next_wave_.proximate_heads_.push_back(followup_id);
	    }
	}
}
	
bool WaveScheduler::is_immediate(const LSInstruction& instruction)
{
	if (std::get_if<SinglePatchMeasurement>(&instruction.operation))
		return true;
	else if (std::get_if<MultiPatchMeasurement>(&instruction.operation))
		return true;
	else if (std::get_if<PatchInit>(&instruction.operation))
		return true;
	else if (std::get_if<BellPairInit>(&instruction.operation))
		return true;
	else if (std::get_if<MagicStateRequest>(&instruction.operation))
		return true;
	else if (std::get_if<YStateRequest>(&instruction.operation))
		return true;
	else if (auto* op = std::get_if<SingleQubitOp>(&instruction.operation))
	{
		switch(op->op)
		{
			case SingleQubitOp::Operator::X: return true;
			case SingleQubitOp::Operator::Y: return true;
			case SingleQubitOp::Operator::Z: return true;
			case SingleQubitOp::Operator::H: return false;
			case SingleQubitOp::Operator::S: return false;
			default: LSTK_UNREACHABLE;
		}
	}
	else if (std::get_if<RotateSingleCellPatch>(&instruction.operation))
		return true;
	else if (std::get_if<BusyRegion>(&instruction.operation))
		return false;
	else if (std::get_if<PatchReset>(&instruction.operation))
		return true;
	else if (std::get_if<BellBasedCNOT>(&instruction.operation))
		return true;
	else
		LSTK_UNREACHABLE;
}
	
bool WaveScheduler::try_schedule_immediately(InstructionID instruction_id, DenseSlice& slice, LSInstructionVisitor instruction_visitor, DensePatchComputationResult& res, bool proximate)
{
	auto& instruction = records_[instruction_id].instruction;
	
	if (!is_immediate(instruction))
		return false;
	
	// std::cerr << "Applying dependent: " << instruction << std::endl;
	auto application_result = try_apply_instruction_direct_followup(slice, instruction, local_instructions_, allow_twists_, gen_op_ids_, layout_, *router_);
	// std::cerr << "Applied dependent: " << instruction << std::endl;

	if (application_result.maybe_error)
	{
		// This is the case that compilation of an EDP set is deferred to the end
		if (application_result.followup_instructions.size() == 1 && application_result.followup_instructions[0] == instruction)
		{
			assert(router_->get_EDPC());
			current_wave_.deferred_to_end.push_back(instruction_id);
			return true;
		}

		// Otherwise, report that instruction could not be placed
		return false;
	}
	else
	{
		++res.ls_instructions_count_;
		instruction_visitor(instruction);
		
		if (application_result.followup_instructions.size() == 1 && application_result.followup_instructions[0] == instruction) // instruction has rescheduled itself
			next_wave_.proximate_heads_.push_back(instruction_id);
		else
			schedule_dependent_instructions(instruction_id, application_result.followup_instructions, slice, instruction_visitor, res, proximate);
		
		return true;
	}
}


}

