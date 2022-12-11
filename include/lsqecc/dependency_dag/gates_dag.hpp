#ifndef LSQECC_DEPENDENCY_DAG_GATES
#define LSQECC_DEPENDENCY_DAG_GATES

#include <lsqecc/dependency_dag/dependency_dag.hpp>

#include <lsqecc/gates/parse_gates.hpp>

namespace lsqecc {

using GatesDag = DependencyDag<gates::Gate>;

GatesDag make_dag_from_instruction_stream(std::unique_ptr<GateStream>&& gate_stream);

}


#endif