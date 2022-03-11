#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/fast_patch_computation.hpp>

#include <lstk/lstk.hpp>

#include <argparse/argparse.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <filesystem>
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
        Progress, NoProgress, Machine
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
                .description("File name of file with LS Instructions. If not provided will read from stdin")
                .required(false);
        parser.add_argument()
                .names({"-l", "--layout"})
                .description("File name of file with layout spec. Defaults to simple layout if none is provided")
                .required(false);
        parser.add_argument()
                .names({"-o", "--output"})
                .description("File name of output file to which write a latticesurgery.com JSON of the slices")
                .required(false);
        parser.add_argument()
                .names({"-t", "--timeout"})
                .description("Set a timeout in seconds after which stop producing slices")
                .required(false);
        parser.add_argument()
                .names({"-r", "--router"})
                .description("Set a router: naive_cached (default), naive")
                .required(false);
        parser.add_argument()
                .names({"-f", "--output-format"})
                .description("How to format output: progress (default), noprogres, machine")
                .required(false);
        parser.add_argument()
                .names({"-g", "--graph-search"})
                .description("Set a graph search provider: custom (default), boost (not allways available)")
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


        auto output_format_mode = OutputFormatMode::Progress;
        if(parser.exists("f"))
        {
            auto mode_arg = parser.get<std::string>("f");
            if( mode_arg== "progress")
                LSTK_NOOP;
            else if (mode_arg == "noprogres")
                output_format_mode = OutputFormatMode::NoProgress;
            else if (mode_arg == "machine")
                output_format_mode = OutputFormatMode::Machine;
            else
            {
                err_stream << "Unknown output format mode " << mode_arg << std::endl;
                return -1;
            }
        }



        std::optional<std::ifstream> instructions_file;
        if(parser.exists("i"))
        {
            instructions_file = std::ifstream(parser.get<std::string>("i"));
            if(instructions_file->fail()){
                err_stream << "Could not open instruction file: " << parser.get<std::string>("i") <<std::endl;
                return -1;
            }
        }

        LSInstructionStream instruction_stream{instructions_file?*instructions_file:in_stream};

        std::unique_ptr<Layout> layout;
        if(parser.exists("l"))
            layout = std::make_unique<LayoutFromSpec>(file_to_string(parser.get<std::string>("l")));
        else
            layout = std::make_unique<SimpleLayout>(instruction_stream.core_qubits().size());

        auto timeout = parser.exists("t") ?
                       std::make_optional(std::chrono::seconds{parser.get<uint32_t>("t")})
                                          : std::nullopt;


        std::unique_ptr<Router> router = std::make_unique<CachedNaiveDijkstraRouter>();
        if(parser.exists("r"))
        {
            auto router_name = parser.get<std::string>("r");
            if(router_name =="naive_cached")
                LSTK_NOOP;// Already set
            else if(router_name=="naive")
                router = std::make_unique<NaiveDijkstraRouter>();
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


        auto no_op_visitor = [](const Slice& s) -> void {LSTK_UNUSED(s);};

        // TODO replace this with a stream to file visitor
        PatchComputation::SliceVisitorFunction slice_appending_visitor(no_op_visitor);
        std::vector<Slice> slices;
        if(parser.exists("o"))
        {
            slice_appending_visitor = [&slices](const Slice& s){
                slices.push_back(s);
            };
        }

        size_t slice_counter = 0;
        PatchComputation::SliceVisitorFunction visitor_with_progress = slice_appending_visitor;
        auto gave_update_at = lstk::now();
        if(output_format_mode == OutputFormatMode::Progress)
        {
            visitor_with_progress = [&](const Slice& s)
            {
                slice_appending_visitor(s);
                slice_counter++;
                if(lstk::seconds_since(gave_update_at)>=1)
                {
                    out_stream << "Slice count: " << slice_counter << "\r" << std::flush;
                    gave_update_at = std::chrono::steady_clock::now();
                }
            };
        }

        auto start = lstk::now();

        try
        {
            PatchComputation patch_computation{
                    std::move(instruction_stream),
                    std::move(layout),
                    std::move(router),
                    timeout,
                    visitor_with_progress
            };

            if(output_format_mode == OutputFormatMode::Machine)
            {
                out_stream << patch_computation.ls_instructions_count() << ","
                           << patch_computation.slice_count() << ","
                           << lstk::seconds_since(start) << std::endl;
            }
            else
            {
                out_stream << "LS Instructions read  " << patch_computation.ls_instructions_count() << std::endl;
                out_stream << "Slices " << patch_computation.slice_count() << std::endl;
                out_stream << "Made patch computation. Took " << lstk::seconds_since(start) << "s." << std::endl;
            }

            if(slices.size()>0)
            {
                out_stream << "Writing slices to file" << std::endl;
                start = lstk::now();
                auto slices_json = slices_to_json(slices);
                std::ofstream(parser.get<std::string>("o")) << slices_json.dump(3) << std::endl;
                out_stream << "Written slices. Took "<< lstk::since(start).count() << "s." << std::endl;
            }

        }
        catch (const std::exception& e)
        {
            err_stream << "Compiler exception: " << e.what() << std::endl;
            return -1;
        }

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

        int exit_code = run_slicer_program(static_cast<int>(c_args.size()), c_args.data(), input, output, err);

        nlohmann::json json_res = {
                {"output", output.str()},
                {"err",err.str()},
                {"exit_code", exit_code}
        };

        return json_res.dump(3);
    }




}