#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/gates/clifford_plus_t_conversion_stream.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/boundary_rotation_injection_stream.hpp>
#include <lsqecc/ls_instructions/teleported_s_gate_injection_stream.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/layout/dynamic_layouts/compact_layout.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/dense_patch_computation.hpp>
#include <lsqecc/patches/slice_variant.hpp>

#include <lstk/lstk.hpp>

#include <argparse/argparse.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <chrono>


#define CONSOLE_HELP_NEWLINE_ALIGN "\n                           "

namespace lsqecc
{


    const inline double k_default_precision_log_ten_negative = 10;

    std::string file_to_string(std::string fname)
    {
        if(!std::filesystem::exists(fname))
            throw std::logic_error{std::string{"File not found:"}+fname};


        std::ifstream file(fname);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return std::string{buffer.str()};
    }


    enum class OutputFormatMode
    {
        Progress, NoProgress, Machine
    };


    // TODO: Resume by getting the slicer to accept stdin as input


    int run_slicer_program(
            int argc, const char* argv[],
            std::istream& in_stream,
            std::ostream& out_stream,
            std::ostream& err_stream)
    {
        std::string prog_name{argv[0]};
        argparse::ArgumentParser parser(prog_name, "Slice LS-Instructions");
        parser.add_argument()
                .names({"-i", "--input"})
                .description("File with input. If not provided will read LS Instructions from stdin")
                .required(false);
        parser.add_argument()
                .names({"-q", "--qasm"})
                .description("File name of file with QASM. When not provided will read as LLI (not QASM)")
                .required(false);
        parser.add_argument()
                .names({"-l", "--layout"})
                .description("File name of file with layout spec. Defaults to simple layout if none is provided")
                .required(false);
        parser.add_argument()
                .names({"-o", "--output"})
                .description("File name of output. When not provided outputs to stdout")
                .required(false);
        parser.add_argument()
                .names({"-f", "--output-format"})
                .description("Requires -o, STDOUT output format: progress, noprogress, machine")
                .required(false);
        parser.add_argument()
                .names({"-t", "--timeout"})
                .description("Set a timeout in seconds after which stop producing slices")
                .required(false);
        parser.add_argument()
                .names({"-r", "--router"})
                .description("Set a router: graph_search (default), graph_search_cached")
                .required(false);
        parser.add_argument()
                .names({"-g", "--graph-search"})
                .description("Set a graph search provider: custom (default), boost (not always available)")
                .required(false);
        parser.add_argument()
                .names({"--graceful"})
                .description("If there is an error when slicing, print the error and terminate")
                .required(false);
        parser.add_argument()
                .names({"--printlli"})
                .description("Output LLI instead of JSONs")
                .required(false);
        parser.add_argument()
                .names({"--noslices"})
                .description("Do the slicing but don't write the slices out")
                .required(false);
        parser.add_argument()
                .names({"--cnotcorrections"})
                .description("Add Xs and Zs to correct the the negative outcomes: never (default), always") // TODO add random
                .required(false);
        parser.add_argument()
                .names({"--compactlayout"})
                .description("Uses Litinski's compact layout, incompatible with -l")
                .required(false);
        #ifdef USE_GRIDSYNTH
        parser.add_argument()
                .names({"--rzprecision"})
                .description("Float to define the precision when Gridsynth is compiled. The precision is given by the " CONSOLE_HELP_NEWLINE_ALIGN
                             "negative power of ten of this value (I.e. precision=10^(-rzprecision)). Defaults to 10.")
                .required(false);
        #endif // USE_GRIDSYNTH
        
        parser.enable_help();

        auto err = parser.parse(argc, argv);
        if (err)
        {
            err_stream << err << std::endl;
            parser.print_help();
            return -1;
        }
        if (parser.exists("help"))
        {
            parser.print_help();
            return 0;
        }

        std::reference_wrapper<std::ostream> write_slices_stream = std::ref(out_stream);
        std::unique_ptr<std::ostream> _ofstream_store;
        if(parser.exists("o"))
        {
            _ofstream_store = std::make_unique<std::ofstream>(parser.get<std::string>("o"));
            write_slices_stream = std::ref(*_ofstream_store);
        }

        auto output_format_mode = (parser.exists("o") || parser.exists("noslices")) ? OutputFormatMode::Progress : OutputFormatMode::NoProgress;
        if(parser.exists("f"))
        {
            auto mode_arg = parser.get<std::string>("f");
            if (mode_arg=="progress")
                output_format_mode = OutputFormatMode::Progress;
            else if (mode_arg=="noprogres")
                output_format_mode = OutputFormatMode::NoProgress;
            else if (mode_arg=="machine")
                output_format_mode = OutputFormatMode::Machine;
            else
            {
                err_stream << "Unknown output format mode " << mode_arg << std::endl;
                return -1;
            }

            if(!parser.exists("o") && !parser.exists("noslices"))
            {
                err_stream << "-f requires -o or --noslices" << std::endl;
                return -1;
            }
        }

        std::reference_wrapper<std::istream> input_file_stream = std::ref(in_stream);
        std::unique_ptr<std::ifstream> _file_to_read_store;
        if(parser.exists("i"))
        {
            _file_to_read_store = std::make_unique<std::ifstream>(parser.get<std::string>("i"));
            if(!std::filesystem::exists(parser.get<std::string>("i")) || _file_to_read_store->fail()){
                err_stream << "Could not open instruction file: " << parser.get<std::string>("i") <<std::endl;
                return -1;
            }
            input_file_stream = std::ref(*_file_to_read_store);
        }

        IdGenerator id_generator;
        std::unique_ptr<LSInstructionStream> instruction_stream;
        std::unique_ptr<GateStream> gate_stream;

        if(!parser.exists("q"))
        {
            instruction_stream = std::make_unique<LSInstructionStreamFromFile>(input_file_stream.get());
            id_generator.set_start(*std::max(instruction_stream->core_qubits().begin(),
                                             instruction_stream->core_qubits().end()));
        }
        if(parser.exists("q"))
        {
            gate_stream = std::make_unique<GateStreamFromFile>(input_file_stream.get());
            gate_stream = std::make_unique<CliffordPlusTConversionStream>(
                std::move(gate_stream),
                parser.exists("rzprecision") ? parser.get<double>("rzprecision") : k_default_precision_log_ten_negative
            );

            CNOTCorrectionMode cnot_correction_mode = CNOTCorrectionMode::NEVER;
            if(parser.exists("cnotcorrections"))
            {
                if(parser.get<std::string>("cnotcorrections") == "always")
                    cnot_correction_mode = CNOTCorrectionMode::ALWAYS;
                else if(parser.get<std::string>("cnotcorrections") == "never")
                    cnot_correction_mode = CNOTCorrectionMode::NEVER;
                else
                {
                    err_stream << "Unknown CNOT correction mode: " << parser.get<std::string>("cnotcorrections") <<std::endl;
                    return -1;
                }
            }
            id_generator.set_start(gate_stream->get_qreg().size);
            instruction_stream = std::make_unique<LSInstructionStreamFromGateStream>(*gate_stream, cnot_correction_mode, id_generator);
        }

        if(parser.exists("printlli"))
        {
            print_all_ls_instructions_to_string(out_stream, std::move(instruction_stream));
            return 0;
        }

        std::unique_ptr<Layout> layout;
        if (parser.exists("compactlayout"))
        {
            layout = make_compact_layout(instruction_stream->core_qubits().size());
            instruction_stream = std::make_unique<TeleportedSGateInjectionStream>(std::move(instruction_stream), id_generator);
            instruction_stream = std::make_unique<BoundaryRotationInjectionStream>(std::move(instruction_stream), *layout);

        }
        else if(parser.exists("l"))
            layout = std::make_unique<LayoutFromSpec>(file_to_string(parser.get<std::string>("l")));
        else
            layout = std::make_unique<SimpleLayout>(instruction_stream->core_qubits().size());

        auto timeout = parser.exists("t") ?
                       std::make_optional(std::chrono::seconds{parser.get<uint32_t>("t")})
                                          : std::nullopt;


        std::unique_ptr<Router> router = std::make_unique<NaiveDijkstraRouter>();
        if(parser.exists("r"))
        {
            auto router_name = parser.get<std::string>("r");
            if(router_name =="graph_search") //TODO change to djikstra
                LSTK_NOOP;// Already set
            else if(router_name=="graph_search_cached")
                router = std::make_unique<CachedNaiveDijkstraRouter>();
            else
            {
                err_stream <<"Unknown router: "<< router_name << std::endl;
                err_stream << "Choices are: naive, naive_cached." << std::endl;
                return -1;
            }
        }

        router->set_graph_search_provider(GraphSearchProvider::Custom);
        if(parser.exists("g"))
        {
            auto router_name = parser.get<std::string>("r");
            if(router_name =="custom")
                LSTK_NOOP;// Already set
            else if(router_name=="boost")
                router->set_graph_search_provider(GraphSearchProvider::Boost);
            else
            {
                err_stream<<"Unknown router: "<< router_name <<std::endl;
                err_stream << "Choices are: custom, boost." << std::endl;
                return -1;
            }
        }


        using SliceVisitorFunction = std::function<void(const SliceVariant & slice)>;

        auto no_op_visitor = [](const SliceVariant& s) -> void {LSTK_UNUSED(s);};

        SliceVisitorFunction slice_printing_visitor{no_op_visitor};
        bool is_first_slice = true;
        if(!parser.exists("noslices"))
        {
            slice_printing_visitor = [&write_slices_stream, &is_first_slice](const SliceVariant & s){
                if(is_first_slice)
                {
                    write_slices_stream.get() << "[\n" << std::visit(
                            [&](const auto& slice){ return slice_to_json(slice).dump(3);}, s);
                    is_first_slice = false;
                }
                else
                    write_slices_stream.get() << ",\n" << std::visit(
                            [&](const auto& slice){ return slice_to_json(slice).dump(3);},s);
            };
        }

        size_t slice_counter = 0;

        SliceVisitorFunction visitor_with_progress{slice_printing_visitor};

        auto gave_update_at = lstk::now();
        if(output_format_mode == OutputFormatMode::Progress)
        {
            visitor_with_progress = [&](const SliceVariant & s)
            {
                slice_printing_visitor(s);
                slice_counter++;
                if(lstk::seconds_since(gave_update_at)>=1)
                {
                    out_stream << "Slice count: " << slice_counter << "\r" << std::flush;
                    gave_update_at = std::chrono::steady_clock::now();
                }
            };
        }

        auto start = lstk::now();

        std::unique_ptr<PatchComputationResult> computation_result;

        if (!parser.exists("a") || (parser.exists("a") && parser.get<std::string>("a") == "dense"))
        {
            computation_result = std::make_unique<DensePatchComputationResult>(run_through_dense_slices(
                    std::move(*instruction_stream),
                    *layout,
                    *router,
                    timeout,
                    [&](const DenseSlice& s){visitor_with_progress(s);},
                    parser.exists("graceful")
            ));
        } else
        {
            err_stream << "Invalid patch repr: " << parser.get<std::string>("a") << std::endl;
            return 1;
        }


        if(parser.exists("o") || parser.exists("noslices"))
        {
            if (output_format_mode == OutputFormatMode::Machine)
            {
                out_stream << computation_result->ls_instructions_count() << ","
                           << computation_result->slice_count() << ","
                           << lstk::seconds_since(start) << std::endl;
            } else if (output_format_mode == OutputFormatMode::Progress)
            {
                out_stream << "LS Instructions read  " << computation_result->ls_instructions_count() << std::endl;
                out_stream << "Slices " << computation_result->slice_count() << std::endl;
                out_stream << "Made patch computation. Took " << lstk::seconds_since(start) << "s." << std::endl;
            }
        }
        if(!parser.exists("noslices"))
            write_slices_stream.get() << "]" <<std::endl;

        return 0;
    }



    std::string run_slicer_program_from_strings(std::string command_line, std::string standard_input)
    {
        std::vector<std::string> args = lstk::split_on_get_strings(command_line, ' ');
        std::vector<const char*> c_args;

        c_args.push_back("slicer_from_strings"); // Placeholder name to occupy the program name's place in argv
        for (const auto &arg : args)
        {
            c_args.push_back(arg.c_str());
        }

        std::istringstream input{standard_input};
        std::ostringstream output;
        std::ostringstream err;

        int exit_code = 100;
        try
        {
            exit_code = run_slicer_program(static_cast<int>(c_args.size()), c_args.data(), input, output, err);
        }
        catch (const std::exception& e)
        {
            err << "Compiler exception: " << e.what() << std::endl;
        }

        nlohmann::json json_res = {
                {"output", output.str()},
                {"err",err.str()},
                {"exit_code", exit_code}
        };

        return json_res.dump(3);
    }




}