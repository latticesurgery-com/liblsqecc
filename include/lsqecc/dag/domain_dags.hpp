#pragma once

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/gates/parse_gates.hpp>

#include <lsqecc/dag/dependency_dag.hpp>

namespace lsqecc::dag {


DependencyDag<LSInstruction> full_dependency_dag_from_instruction_stream(LSInstructionStream& instruction_stream);


DependencyDag<gates::Gate> full_dependency_dag_from_gate_stream(GateStream& gate_stream);



}