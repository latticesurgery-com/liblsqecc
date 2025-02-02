#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/dag/domain_dags.hpp>
#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/gates/decompose_rotation_stream.hpp>
#include <lsqecc/ls_instructions/ls_instruction_stream.hpp>
#include <lsqecc/ls_instructions/boundary_rotation_injection_stream.hpp>
#include <lsqecc/ls_instructions/teleported_s_gate_injection_stream.hpp>
#include <lsqecc/ls_instructions/catalytic_s_gate_injection_stream.hpp>
#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <lsqecc/layout/router.hpp>
#include <lsqecc/layout/dynamic_layouts/compact_layout.hpp>
#include <lsqecc/layout/dynamic_layouts/edpc_layout.hpp>
#include <lsqecc/patches/slices_to_json.hpp>
#include <lsqecc/patches/slice.hpp>
#include <lsqecc/patches/slice_stats.hpp>
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

#include <string>


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
        Progress, NoProgress, Machine, Stats
    };

    enum class PrintDagMode {
        None, Input, ProcessedLli
     };

    enum class LLIPrintMode
    {
        None, BeforeSlicing, Sliced
    };

    enum class CompilationMode
    {
        Local, Nonlocal
    };

    enum class AutoLayoutMode // Layouts specified with -L
    {
        Compact, CompactNoClogging, Edpc
    };

    using LayoutMode = std::variant<AutoLayoutMode, std::unique_ptr<LayoutFromSpec>>; // LayoutFromSpec is for -l

    enum class SGateMode
    {
        Catalytic, Twists
    };

    DistillationOptions make_distillation_options(argparse::ArgumentParser& parser)
    {
        DistillationOptions distillation_options;
        if(parser.exists("disttime"))
            distillation_options.distillation_time = parser.get<size_t>("disttime");
        if(parser.exists("nostagger"))
            distillation_options.staggered = false;
        return distillation_options;

    }

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
                .description("File name of file with layout spec, otherwise the layout is auto-generated (configure with -L)")
                .required(false);
        parser.add_argument()
                .names({"-o", "--output"})
                .description("File name of output. When not provided outputs to stdout")
                .required(false);
        parser.add_argument()
                .names({"-f", "--output-format"})
                .description("Requires -o, STDOUT output format: progress, noprogress, machine, stats")
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
                .names({"-P", "--pipeline"})
                .description("pipeline mode: stream (default), wave, edpc, dag (deprecated)")
                .required(false);
        parser.add_argument()
                .names({"-g", "--graph-search"})
                .description("Set a graph search provider: djikstra (default), astar, boost (not always available)")
                .required(false);
        parser.add_argument()
                .names({"--graceful"})
                .description("If there is an error when slicing, print the error and terminate")
                .required(false);
        parser.add_argument()
                .names({"--printlli"})
                .description("Output LLI instead of JSONs. options: before (default), sliced (prints lli on the same slice separated by semicolons)")
                .required(false);
        parser.add_argument()
                .names({"--printdag"})
                .description("Prints a dependency dag of the circuit. Modes: input (default), processedlli")
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
                .names({"--op-ids"})
                .description("Generates certain opearion ids. For now, it only adds ids to multi-body-measurement")
                .required(false);
        parser.add_argument()
                .names({"--layoutgenerator","-L"})
                .description(
                    "Automatically generates a layout for the given number of qubits. Incompatible with -l. Options:" CONSOLE_HELP_NEWLINE_ALIGN
                    " - compact (default): Uses Litinski's Game of Surace Code compact layout (https://arxiv.org/abs/1808.02892)" CONSOLE_HELP_NEWLINE_ALIGN
                    " - compact_no_clogging: same as compact, but fewer cells for ancillas and magic state queues" CONSOLE_HELP_NEWLINE_ALIGN
                    " - edpc: Uses a family of layouts based upon the one specified in the EDPC paper by Beverland et. al. (https://arxiv.org/abs/2110.11493)"
                )
                .required(false);
        parser.add_argument()
                .names({"--numlanes"})
                .description("Only compatible with -L edpc. Configures number of free lanes for routing.")
                .required(false);
        parser.add_argument()
                    .names({"--condensed"})
                    .description("Only compatible with -L edpc. Packs logical qubits more compactly.")
                    .required(false);
        parser.add_argument()
                    .names({"--explicitfactories"})
                    .description("Only compatible with -L edpc. Explicitly specifies factories (otherwise, uses tiles reserved for magic state re-spawn).")
                    .required(false);
             
        #ifdef USE_GRIDSYNTH
        parser.add_argument()
                .names({"--rzprecision"})
                .description("Float to define the precision when Gridsynth is compiled. The precision is given by the " CONSOLE_HELP_NEWLINE_ALIGN
                             "negative power of ten of this value (I.e. precision=10^(-rzprecision)). Defaults to 10.")
                .required(false);
        #endif // USE_GRIDSYNTH
        parser.add_argument()
                .names({"--nostagger"})
                .description("Turns off staggered distillation block timing")
                .required(false);
        parser.add_argument()
                .names({"--disttime"})
                .description("Set the distillation time (default 10)")
                .required(false);
        parser.add_argument()
                .names({"--local"})
                .description("Compile gates into a pair-wise local lattice surgery instruction set")
                .required(false);
        parser.add_argument()
                .names({"--notwists"})
                .description("Compile S gates using the catalytic teleportation circuit from Fowler, 2012 instead of using the twist-based Y state initialization and teleportation from Gidney, 2024")
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

        DistillationOptions distillation_options = make_distillation_options(parser);

        LayoutMode layout_mode = AutoLayoutMode::Compact;
        if (parser.exists("layoutgenerator"))
        {
            if (parser.get<std::string>("layoutgenerator") == "compact")
            {
                layout_mode = AutoLayoutMode::Compact;
            } else if (parser.get<std::string>("layoutgenerator") == "compact_no_clogging")
            {
                layout_mode = AutoLayoutMode::CompactNoClogging;
            }
            else if (parser.get<std::string>("layoutgenerator") == "edpc") 
            {
                layout_mode = AutoLayoutMode::Edpc;
            }
            else
            {
                err_stream << "Unknown layout generator: " << parser.get<std::string>("layoutgenerator") << std::endl;
                return -1;
            }
        }
        else if(parser.exists("l"))
            layout_mode = std::make_unique<LayoutFromSpec>(file_to_string(parser.get<std::string>("l")), distillation_options);
        else {
            // Default to Litinsiki's compact layout
        }

        std::reference_wrapper<std::ostream> bulk_output_stream = std::ref(out_stream);
        std::unique_ptr<std::ostream> _ofstream_store;
        if(parser.exists("o"))
        {
            _ofstream_store = std::make_unique<std::ofstream>(parser.get<std::string>("o"));
            bulk_output_stream = std::ref(*_ofstream_store);
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
            else if (mode_arg=="stats")
                output_format_mode = OutputFormatMode::Stats;
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

        PrintDagMode print_dag_mode = PrintDagMode::None;
        if (parser.exists("printdag"))
        {
            auto mode_arg = parser.get<std::string>("printdag");
            if (mode_arg=="input" || mode_arg=="")
                print_dag_mode = PrintDagMode::Input;
            else if (mode_arg=="processedlli")
                print_dag_mode = PrintDagMode::ProcessedLli;
            else
            {
                err_stream << "Unknown print dag mode " << mode_arg << std::endl;
                return -1;
            }
        }
        

        PipelineMode pipeline_mode = PipelineMode::Stream;
        bool pipeline_mode_validity_checker = 1;
        if (parser.exists("pipeline"))
        {
            auto mode_arg = parser.get<std::string>("pipeline");
            if (mode_arg=="stream" || mode_arg=="")
                pipeline_mode = PipelineMode::Stream;
            else if (mode_arg=="dag")
                pipeline_mode = PipelineMode::Dag;
            else if (mode_arg=="wave")
                pipeline_mode = PipelineMode::Wave;
            else if (mode_arg=="edpc")
            {
                pipeline_mode = PipelineMode::EDPC;
                pipeline_mode_validity_checker = 0;
            }
            else
            {
                err_stream << "Unknown pipeline mode " << mode_arg << std::endl;
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
        
        CompilationMode compile_mode = CompilationMode::Nonlocal;
        if (parser.exists("local"))
            compile_mode = CompilationMode::Local;

        SGateMode sgate_mode = SGateMode::Twists;
        if (parser.exists("notwists"))
            sgate_mode = SGateMode::Catalytic;

        if(!parser.exists("q"))
        {
            instruction_stream = std::make_unique<LSInstructionStreamFromFile>(input_file_stream.get());
            id_generator.set_start(*std::max(instruction_stream->core_qubits().begin(),
                                             instruction_stream->core_qubits().end()));
            if( print_dag_mode == PrintDagMode::Input )
            {
                auto dag = dag::full_dependency_dag_from_instruction_stream(*instruction_stream);
                dag.to_graphviz(out_stream);
                return 0;
            }
        }
        if(parser.exists("q"))
        {
            gate_stream = std::make_unique<GateStreamFromFile>(input_file_stream.get());

            if (print_dag_mode == PrintDagMode::Input)
            {
                auto dag = dag::full_dependency_dag_from_gate_stream(*gate_stream);
                dag.to_graphviz(out_stream);
                return 0;
            }

            gate_stream = std::make_unique<DecomposeRotationStream>(
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
            instruction_stream = std::make_unique<LSInstructionStreamFromGateStream>(*gate_stream, cnot_correction_mode, id_generator, compile_mode == CompilationMode::Local);    
        }


        std::unique_ptr<Layout> layout;
        if (AutoLayoutMode* auto_layout_mode = std::get_if<AutoLayoutMode>(&layout_mode))
        {
            if (*auto_layout_mode == AutoLayoutMode::Compact)
            {
                if (sgate_mode == SGateMode::Catalytic) 
                {
                    err_stream << "Catalytic S Gates incompatible with layout: " << parser.get<std::string>("layoutgenerator") <<std::endl;
                    return -1;
                }
                layout = make_compact_layout(instruction_stream->core_qubits().size(), distillation_options);
                instruction_stream = std::make_unique<BoundaryRotationInjectionStream>(std::move(instruction_stream), *layout);
                instruction_stream = std::make_unique<TeleportedSGateInjectionStream>(std::move(instruction_stream), id_generator, false);
            } else if (*auto_layout_mode == AutoLayoutMode::CompactNoClogging)
            {
                if (sgate_mode == SGateMode::Catalytic) 
                {
                    err_stream << "Catalytic S Gates incompatible with layout: " << parser.get<std::string>("layoutgenerator") <<std::endl;
                    return -1;
                }
                layout = make_compact_layout(instruction_stream->core_qubits().size(), distillation_options, true);
                instruction_stream = std::make_unique<BoundaryRotationInjectionStream>(std::move(instruction_stream), *layout);
                instruction_stream = std::make_unique<TeleportedSGateInjectionStream>(std::move(instruction_stream), id_generator, false);
            }
            else if (*auto_layout_mode == AutoLayoutMode::Edpc) 
            {
                size_t num_lanes = 1;
                bool condensed = false;
                bool factories_explicit = false;
                
                if(parser.exists("numlanes"))
                    num_lanes = parser.get<size_t>("numlanes");
                
                if(parser.exists("condensed"))
                    condensed = parser.get<bool>("condensed");
                
                if ((pipeline_mode == PipelineMode::EDPC) &&
                    (compile_mode == CompilationMode::Local) &&
                    (num_lanes == 1))
                {
                    pipeline_mode_validity_checker = 1;
                }

                if (parser.exists("explicitfactories")) 
                    factories_explicit = true;

                if (sgate_mode == SGateMode::Catalytic) 
                {
                    layout = make_edpc_layout(instruction_stream->core_qubits().size(), num_lanes, condensed, factories_explicit, true, distillation_options);
                    instruction_stream = std::make_unique<CatalyticSGateInjectionStream>(std::move(instruction_stream), id_generator, compile_mode == CompilationMode::Local, true);
                }

                else if (sgate_mode == SGateMode::Twists) 
                {
                    layout = make_edpc_layout(instruction_stream->core_qubits().size(), num_lanes, condensed, factories_explicit, false, distillation_options);
                    instruction_stream = std::make_unique<TeleportedSGateInjectionStream>(std::move(instruction_stream), id_generator, true);
                }
            }
            else
            {
                LSTK_NOT_IMPLEMENTED;
            }
        }
        else if(std::unique_ptr<LayoutFromSpec>* custom_layout_path = std::get_if<std::unique_ptr<LayoutFromSpec>>(&layout_mode))
        {
            layout = std::move(*custom_layout_path);
            if ((sgate_mode == SGateMode::Catalytic) && (layout->predistilled_y_states().size() == 0))
            {
                err_stream << "Catalytic S Gates require pre-distilled Y states to be specified by 'Y' in layout ASCII." << std::endl;
                return -1;
            }
        }

        // Override the choice of ancilla placements when no location is provided
        if(layout->ancilla_location().empty())
        {
            gates::ControlledGate::default_ancilla_placement = gates::CNOTAncillaPlacement::ANCILLA_NEXT_TO_TARGET;
        }

        if (!pipeline_mode_validity_checker)
            throw std::runtime_error("PipelineMode::EDPC only valid when paired with (default or 1-lane condensed) EDPC layout and CompilationMode::Local.");

        LLIPrintMode lli_print_mode = LLIPrintMode::None;
        if(parser.exists("printlli"))
        {
            auto mode_arg = parser.get<std::string>("printlli");
            if (mode_arg=="before" || mode_arg=="")
                lli_print_mode = LLIPrintMode::BeforeSlicing;
            else if (mode_arg=="sliced")
                lli_print_mode = LLIPrintMode::Sliced;
            else
            {
                err_stream << "Unknown lli print mode " << mode_arg << std::endl;
                return -1;
            }
        }

        if(lli_print_mode == LLIPrintMode::BeforeSlicing)
        {
            print_all_ls_instructions_to_string(out_stream, std::move(instruction_stream));
            return 0;
        } else if (print_dag_mode == PrintDagMode::ProcessedLli)
        {
            auto dag = dag::full_dependency_dag_from_instruction_stream(*instruction_stream);
            dag.to_graphviz(out_stream);
            return 0;
        }

        auto timeout = parser.exists("t") ?
                       std::make_optional(std::chrono::seconds{parser.get<uint32_t>("t")})
                                          : std::nullopt;


        std::unique_ptr<Router> router = std::make_unique<CustomDPRouter>();
        if(parser.exists("r"))
        {
            auto router_name = parser.get<std::string>("r");
            if(router_name =="graph_search") //TODO change to djikstra
                LSTK_NOOP;// Already set
            else if(router_name=="graph_search_cached")
                router = std::make_unique<CachedRouter>();
            else
            {
                err_stream <<"Unknown router: "<< router_name << std::endl;
                return -1;
            }
        }

        router->set_graph_search_provider(GraphSearchProvider::Djikstra);
        if(parser.exists("g"))
        {
            auto router_name = parser.get<std::string>("g");
            if(router_name =="astar")
                router->set_graph_search_provider(GraphSearchProvider::AStar);
            else if (router_name=="djikstra")
                router->set_graph_search_provider(GraphSearchProvider::Djikstra);
            else if(router_name=="boost")
                router->set_graph_search_provider(GraphSearchProvider::Boost);
            else
            {
                err_stream<<"Unknown router: "<< router_name <<std::endl;
                return -1;
            }
        }


        bool print_slices = !parser.exists("noslices") && lli_print_mode == LLIPrintMode::None;
        DenseSliceVisitor slice_visitor = [](const DenseSlice& s) -> void {LSTK_UNUSED(s);};
        bool is_first_slice = true;
        if(print_slices)
        {
            slice_visitor = [&bulk_output_stream, &is_first_slice](const DenseSlice & s){
                if(is_first_slice)
                {
                    bulk_output_stream.get() << "[\n" << slice_to_json(s).dump(3);
                    is_first_slice = false;
                }
                else
                    bulk_output_stream.get() << ",\n" << slice_to_json(s).dump(3) << std::flush;
            };
        }

        size_t slice_counter = 0;

        auto gave_update_at = lstk::now();
        if(output_format_mode == OutputFormatMode::Progress)
        {
            slice_visitor = [&, slice_visitor](const DenseSlice & s)
            {
                slice_visitor(s);
                slice_counter++;
                if(lstk::seconds_since(gave_update_at)>=1)
                {
                    out_stream << "Slice count: " << slice_counter << "\r" << std::flush;
                    gave_update_at = std::chrono::steady_clock::now();
                }
            };
        }


        SliceStats slice_stats;
        if (output_format_mode == OutputFormatMode::Stats)
        {
            slice_visitor = [&, slice_visitor](const DenseSlice & s)
            {
                slice_visitor(s);
                slice_stats.totals += compute_volume_counts(s);
            };
        }

        LSInstructionVisitor instruction_visitor{[&](const LSInstruction& i){}};
        if (lli_print_mode == LLIPrintMode::Sliced)
        {
            instruction_visitor = [&](const LSInstruction& i)
            {
                bulk_output_stream.get() << i << ";";
            }; 

            slice_visitor = [&,slice_visitor](const DenseSlice & s)
            {
                slice_visitor(s);
                bulk_output_stream.get() << std::endl;
            };
        }


        auto start = lstk::now();

        std::unique_ptr<PatchComputationResult> computation_result = 
            std::make_unique<DensePatchComputationResult>(run_through_dense_slices(
                    std::move(*instruction_stream),
                    pipeline_mode,
                    compile_mode == CompilationMode::Local,
                    sgate_mode == SGateMode::Twists,
                    *layout,
                    std::move(router),
                    timeout,
                    slice_visitor,
                    instruction_visitor,
                    parser.exists("graceful"),
                    parser.exists("op-ids")
        ));

        if(parser.exists("o") || parser.exists("noslices"))
        {
            if (output_format_mode == OutputFormatMode::Machine)
            {
                out_stream << computation_result->ls_instructions_count() << ","
                           << computation_result->slice_count() << ","
                           << lstk::seconds_since(start) << std::endl;
            } else if (output_format_mode == OutputFormatMode::Progress || output_format_mode == OutputFormatMode::Stats )
            {
                out_stream << "LS Instructions read  " << computation_result->ls_instructions_count() << std::endl;
                out_stream << "Slices " << computation_result->slice_count() << std::endl;
                out_stream << "Made patch computation. Took " << lstk::seconds_since(start) << "s." << std::endl;
            }
            
            if ( output_format_mode == OutputFormatMode::Stats)
                out_stream << slice_stats << std::endl; 
                
        }
        
        if(print_slices)
            bulk_output_stream.get() << "]" <<std::endl;
        if (lli_print_mode == LLIPrintMode::Sliced)
            bulk_output_stream.get() << std::endl;

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