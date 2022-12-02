#include <lsqecc/dependency_dag/lli_dag.hpp>

namespace lsqecc {

LLIDag make_dag_from_lli_stream(std::unique_ptr<LSInstructionStream>&& lli_stream)
{
    LLIDag lli_dag;

    while(lli_stream->has_next_instruction())
        lli_dag.push(lli_stream->get_next_instruction());

    return lli_dag;
}


}
