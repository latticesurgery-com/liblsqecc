
#include <lsqecc/pipelines/slicer.hpp>



int main(int argc, const char* argv[])
{
    return lsqecc::run_slicer_program(argc, argv, std::cin, std::cout, std::cerr);
}

