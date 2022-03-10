#include <emscripten/bind.h>
#include <lsqecc/pipelines/slicer.hpp>

using namespace emscripten;


EMSCRIPTEN_BINDINGS(lsqecc_emscripten) {
        function("run_slicer_program_from_strings", &lsqecc::run_slicer_program_from_strings);
}
