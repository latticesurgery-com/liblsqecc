#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/sparse_patch_computation.hpp>
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


namespace lsqecc
{



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
        Progress, NoProgress, Machine, Slices
    };


    int run_slicer_program(
            int argc, const char* argv[],
            std::istream& in_stream,
            std::ostream& out_stream,
            std::ostream& err_stream)
    {
        std::string prog_name{argv[0]};
        argparse::ArgumentParser parser(prog_name, "Slice LS-Instructions");
        parser.add_argument()
                .names({"-i", "--instructions"})
                .description("File name of file with LS Instructions. If not provided will read LS Instructions from stdin")
                .required(false);
        parser.add_argument()
                .names({"-q", "--qasm"}) // TODO remove and use extension
                .description("File name of file with QASM. If not provided will read LS Instructions (not QASM) from stdin")
                .required(false);
        parser.add_argument()
                .names({"-l", "--layout"})
                .description("File name of file with layout spec. Defaults to simple layout if none is provided")
                .required(false);
        parser.add_argument()
                .names({"-o", "--output"})
                .description("File name of output file to which write a latticesurgery.com JSON of the slices."
                             " By default outputs to stdout")
                .required(false);
        parser.add_argument()
                .names({"-f", "--output-format"})
                .description("Requires, STDOUT output format: slices (default), progress , noprogress, machine,")
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
                .names({"-a", "--slice-repr"})
                .description("Set how slices are represented: dense (default), sparse")
                .required(false);
        parser.add_argument()
                .names({"--graceful"})
                .description("If there is an error when slicing, print the error and terminate")
                .required(false);
        parser.add_argument()
                .names({"--lli"})
                .description("Output LLI instead of JSONs")
                .required(false);
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

        std::reference_wrapper<std::ostream> write_slices_stream = std::ref(std::cout);
        std::unique_ptr<std::ostream> _ofstream_store;
        if(parser.exists("o"))
        {
            _ofstream_store = std::make_unique<std::ofstream>(parser.get<std::string>("o"));
            write_slices_stream = std::ref(*_ofstream_store);
        }

        auto output_format_mode = parser.exists("o") ? OutputFormatMode::Progress : OutputFormatMode::Slices;
        if(parser.exists("f"))
        {
            auto mode_arg = parser.get<std::string>("f");
            if (mode_arg=="slices")
                output_format_mode = OutputFormatMode::Slices;
            else if (mode_arg=="progress")
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

        }
        bool is_writing_slices = parser.exists("o") || output_format_mode == OutputFormatMode::Slices;



        std::ifstream file_stream;
        std::unique_ptr<LSInstructionStream> instruction_stream;
        if(parser.exists("i"))
        {
            if(parser.exists("q")){
                err_stream << "Can only allow one of -i and -q at once" <<std::endl;
                return -1;
            }

            file_stream = std::ifstream(parser.get<std::string>("i"));
            if(file_stream.fail() || !std::filesystem::exists(parser.get<std::string>("i"))){
                err_stream << "Could not open instruction file: " << parser.get<std::string>("i") <<std::endl;
                return -1;
            }

            instruction_stream = std::make_unique<LSInstructionStreamFromFile>(file_stream);
        }
        if(!instruction_stream && !parser.exists("q"))
            instruction_stream = std::make_unique<LSInstructionStreamFromFile>(in_stream);


        std::unique_ptr<GateStream> gate_stream;
        if(parser.exists("q"))
        {
            file_stream = std::ifstream(parser.get<std::string>("q"));
            if(file_stream.fail() || !std::filesystem::exists(parser.get<std::string>("q"))){
                err_stream << "Could not open instruction file: " << parser.get<std::string>("q") <<std::endl;
                return -1;
            }

            gate_stream = std::make_unique<GateStreamFromFile>(file_stream);
            instruction_stream = std::make_unique<LSInstructionStreamFromGateStream>(*gate_stream);
        }

        if(parser.exists("lli"))
        {
            print_all_ls_instructions_to_string(out_stream, std::move(instruction_stream));
            return 0;
        }

        std::unique_ptr<Layout> layout;
        if(parser.exists("l"))
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
        if(is_writing_slices)
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

        SliceVisitorFunction visitor_with_progress{slice_printing_visitor};;

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

        if(parser.exists("a") && parser.get<std::string>("a") == "sparse")
            computation_result = std::make_unique<SparsePatchComputation>(
                std::move(*instruction_stream),
                std::move(layout),
                std::move(router),
                timeout,
                visitor_with_progress,
                parser.exists("graceful")
            );
        else if (!parser.exists("a") || (parser.exists("a") && parser.get<std::string>("a") == "dense"))
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

        if(output_format_mode == OutputFormatMode::Machine)
        {
            out_stream << computation_result->ls_instructions_count() << ","
                       << computation_result->slice_count() << ","
                       << lstk::seconds_since(start) << std::endl;
        }
        else if (output_format_mode == OutputFormatMode::Progress)
        {
            out_stream << "LS Instructions read  " << computation_result->ls_instructions_count() << std::endl;
            out_stream << "Slices " << computation_result->slice_count() << std::endl;
            out_stream << "Made patch computation. Took " << lstk::seconds_since(start) << "s." << std::endl;
        }

        if(is_writing_slices)
            write_slices_stream.get() << "]" <<std::endl;



        return 0;
    }



    std::string run_slicer_program_from_strings(std::string command_line, std::string standard_input)
    {
        std::vector<std::string> args = lstk::split_on_get_strings(command_line, ' ');
        std::vector<const char*> c_args;

        for (const auto &arg : args)
            c_args.push_back(arg.c_str());

        std::istringstream input{standard_input};
        std::ostringstream output;
        std::ostringstream err;

        int exit_code;
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