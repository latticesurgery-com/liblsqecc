#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/gates/postgres_gate_stream.hpp>

#include <argparse/argparse.h>


int main(int argc, const char* argv[])
{
    using namespace lsqecc;
    std::string prog_name{argv[0]};
        argparse::ArgumentParser parser(prog_name, "Slice LS-Instructions");
        parser.add_argument()
                .names({"-p", "--port"})
                .description("PostgreDB connection port")
                .required(false);


    std::string port = parser.exists("p") ? parser.get<std::string>("p") : "55556";

    PandoraPostgresGateStream pgs = PandoraPostgresGateStream{"127.0.0.1", port, "postgres"};
    while(pgs.has_next_gate()) {
        std::cout << pgs.get_next_gate() << std::endl;
    }

    // return lsqecc::run_slicer_program(argc, argv, std::cin, std::cout, std::cerr);
}

