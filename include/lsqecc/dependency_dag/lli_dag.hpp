#ifndef LSQECC_DEPENDENCY_DAG_LLI
#define LSQECC_DEPENDENCY_DAG_LLI

#include <lsqecc/dependency_dag/dependency_dag.hpp>
#include <lsqecc/dependency_dag/dag_printer.hpp>

#include <lsqecc/ls_instructions/ls_instructions.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>

namespace lsqecc {

using LLIDag = DependencyDag<LSInstruction>;

LLIDag make_dag_from_instruction_stream(std::unique_ptr<LSInstructionStream>&& lli_stream);

}


#endif