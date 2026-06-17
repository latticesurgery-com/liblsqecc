#include <lsqecc/pipelines/slicer.hpp>

#include <lsqecc/dag/domain_dags.hpp>
#include <lsqecc/gates/parse_gates.hpp>
#include <lsqecc/gates/decompose_rotation_stream.hpp>
#include <lsqecc/gates/postgres_gate_stream.hpp>
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
#include <lsqecc/patches/minetest_export.hpp>

#include <lstk/lstk.hpp>

#include <argparse/argparse.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cmath>

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

    enum class InputFormat
    {
        LLI, QASM, Pandora
    };

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
                .names({"-I", "--input-format"})
                .description("Format of input. Modes: qasm|q, pandora|p, lli|l (default)")
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
                .names({"--port"})
                .description("Only compatible with -I pandora. Sets an (optional) port number for Pandora server (default 5432)")
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
                .description("Generates certain operation ids. For now, it only adds ids to multi-body-measurement")
                .required(false);
        parser.add_argument()
                .names({"--layoutgenerator","-L"})
                .description(
                    "Automatically generates a layout for the given number of qubits. Incompatible with -l. Options:" CONSOLE_HELP_NEWLINE_ALIGN
                    " - compact (default): Uses Litinski's Game of Surface Code compact layout (https://arxiv.org/abs/1808.02892)" CONSOLE_HELP_NEWLINE_ALIGN
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
                    .description("Only compatible with -L edpc. Explicitly specifies factories but clogs easily (otherwise, uses tiles reserved for magic state re-spawn).")
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
        parser.add_argument()
                .names({"--minetest"})
                .description("Generate a map.sqlite file for Minetest")
                .required(false);
        parser.add_argument()
                .names({"--stripeheight"})
                .description("Set the stripe height for minetest export (default 4)")
                .required(false);
        parser.add_argument()
                .names({"--slicetiming"})
                .description("Times the per-slice output work (the minetest export when --minetest is active, else" CONSOLE_HELP_NEWLINE_ALIGN
                             "the JSON serialization), excluding routing/production, and appends one CSV row to the" CONSOLE_HELP_NEWLINE_ALIGN
                             "given file (default slice_timings.csv): rows,cols,cells,num_slices,mean_ms,stddev_ms," CONSOLE_HELP_NEWLINE_ALIGN
                             "total_s, keyed by layout size so runs across layout sizes plot as avg/error bars.")
                .required(false);
        parser.add_argument()
                .names({"--maxwait"})
                .description("Max consecutive slices without routing progress before the stream pipeline" CONSOLE_HELP_NEWLINE_ALIGN
                             "aborts as deadlocked (default 1000). Raise for circuits with long magic-state waits.")
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

        InputFormat input_format = InputFormat::LLI;
        if (parser.exists("input-format"))
        {
            auto input_format_arg = parser.get<std::string>("input-format");
            if (input_format_arg == "lli" || input_format_arg == "l")
            {
                input_format = InputFormat::LLI;
            }
            else if (input_format_arg == "qasm" || input_format_arg == "q")
            {
                input_format = InputFormat::QASM;
            }
            else if (input_format_arg == "pandora" || input_format_arg == "p")
            {
                input_format = InputFormat::Pandora;
            }
            else
            {
                err_stream << "Unknown input format: " << input_format_arg << std::endl;
                return -1;
            }
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
        
        CompilationMode compile_mode = CompilationMode::Nonlocal;
        if (parser.exists("local"))
            compile_mode = CompilationMode::Local;

        SGateMode sgate_mode = SGateMode::Twists;
        if (parser.exists("notwists"))
            sgate_mode = SGateMode::Catalytic;

        IdGenerator id_generator;
        std::unique_ptr<LSInstructionStream> instruction_stream;
        std::unique_ptr<GateStream> gate_stream;

        if (input_format == InputFormat::LLI)
        {
            try
            {
                instruction_stream = std::make_unique<LSInstructionStreamFromFile>(input_file_stream.get());
            }
            catch (const std::exception& e)
            {
                err_stream << "Could not read LS instructions: " << e.what() << std::endl;
                return -1;
            }
            const auto& core_qubits = instruction_stream->core_qubits();
            if (core_qubits.empty())
            {
                err_stream << "No logical qubits declared; nothing to slice." << std::endl;
                return -1;
            }
            // Start generating ids past the highest declared core-qubit id so new patches don't
            // collide with them (matches the QASM path's set_start(qreg.size)). core_qubits() is
            // insertion-ordered, not sorted, so scan for the max rather than taking the last.
            id_generator.set_start(*std::max_element(core_qubits.begin(), core_qubits.end()) + 1);
            if( print_dag_mode == PrintDagMode::Input )
            {
                auto dag = dag::full_dependency_dag_from_instruction_stream(*instruction_stream);
                dag.to_graphviz(out_stream);
                return 0;
            }
        }
        else if (input_format == InputFormat::QASM)
        {
            gate_stream = std::make_unique<GateStreamFromFile>(input_file_stream.get());
        }
        else if (input_format == InputFormat::Pandora)
        {
            auto port = !parser.get<std::string>("port").empty() ? parser.get<std::string>("port") : "5432";
            gate_stream = std::make_unique<PandoraPostgresGateStream>("127.0.0.1", port, "postgres");
        }

        if (gate_stream) {
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

            if (gate_stream->get_qreg().size == 0)
            {
                err_stream << "No qubits in the circuit; nothing to slice." << std::endl;
                return -1;
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

        int stripe_height = parser.exists("stripeheight") ? parser.get<int>("stripeheight") : 4;
        // Builder opens the DB and holds the transaction for the duration of slicing.
        // Constructed only when --minetest is active so that no DB file is created otherwise.
        std::optional<lsqecc::MinetestMapBuilder> minetest_builder;
        if (parser.exists("minetest") && print_slices)
            minetest_builder.emplace("map.sqlite");

        const bool minetest_output = parser.exists("minetest");

        size_t slice_counter = 0;

        // The minetest path runs with no -o/--noslices, so it would otherwise be silent.
        // Show the live slice count so routing progress is visible and stalls are obvious.
        bool show_progress = output_format_mode == OutputFormatMode::Progress || minetest_output;
        auto gave_update_at = lstk::now();

        SliceStats slice_stats;

        LSInstructionVisitor instruction_visitor{[&](const LSInstruction& i){LSTK_UNUSED(i);}};
        if (lli_print_mode == LLIPrintMode::Sliced)
        {
            instruction_visitor = [&](const LSInstruction& i)
            {
                bulk_output_stream.get() << i << ";";
            };
        }

        // Per-slice consumer state, previously hidden inside the nested visitor lambdas.
        bool is_first_slice = true;
        size_t time_stamp = 0;

        // Per-slice timing, for benchmarking output cost vs layout size. Times only this
        // iteration's output work (the minetest export when active, else the JSON serialization),
        // not the routing/production that precedes it. Welford's online algorithm keeps the
        // mean/variance in O(1) memory, matching the streaming design (no per-slice vector
        // retained). Gated on --slice-timing so the common path pays nothing.
        const bool profile_slices = parser.exists("slicetiming");
        size_t timing_n        = 0;
        double timing_mean_ms  = 0.0; // running mean of per-slice processing time
        double timing_m2       = 0.0; // running sum of squared deviations from the mean
        double timing_total_ms = 0.0;

        auto start = lstk::now();

        size_t max_no_progress = parser.exists("maxwait")
            ? parser.get<size_t>("maxwait") : MAX_NO_PROGRESS_SLICES_STREAM_DEFAULT;

        // Pull-based: the engine produces one slice at a time and we drive the loop, applying
        // each output concern as a flat, ordered step. The slice reference is only valid for the
        // current iteration (the engine mutates it on the next advance), so we never retain it.
        std::unique_ptr<DenseSliceStream> slices = run_through_dense_slices(
                std::move(instruction_stream),
                *layout,
                std::move(router),
                instruction_visitor,
                {.pipeline_mode          = pipeline_mode,
                 .local_instructions     = compile_mode == CompilationMode::Local,
                 .allow_twists           = sgate_mode == SGateMode::Twists,
                 .gen_op_ids             = parser.exists("op-ids") || input_format == InputFormat::Pandora, // op-ids required by Pandora
                 .timeout                = timeout,
                 .max_no_progress_slices = max_no_progress});

        auto consume_slices = [&]()
        {
            for (const DenseSlice& slice : *slices)
            {
                // Time only the output work below, excluding the routing/production that the
                // iterator advance already performed before handing us this slice.
                auto output_start = lstk::now();

                // Base output: write the slice to its destination.
                if (print_slices)
                {
                    if (minetest_output)
                    {
                        minetest_builder->add_slice(slice, time_stamp, stripe_height);
                        ++time_stamp;
                    }
                    else if (is_first_slice)
                    {
                        bulk_output_stream.get() << "[\n" << slice_to_json(slice).dump(3);
                        is_first_slice = false;
                    }
                    else
                        bulk_output_stream.get() << ",\n" << slice_to_json(slice).dump(3) << std::flush;
                }

                // Live progress counter.
                if (show_progress)
                {
                    slice_counter++;
                    if(lstk::seconds_since(gave_update_at)>=1)
                    {
                        out_stream << "Routing slices, count: " << slice_counter << "\r" << std::flush;
                        gave_update_at = std::chrono::steady_clock::now();
                    }
                }

                // Stats accumulation.
                if (output_format_mode == OutputFormatMode::Stats)
                    slice_stats.totals += compute_volume_counts(slice);

                // Sliced-LLI: terminate this slice's instruction line.
                if (lli_print_mode == LLIPrintMode::Sliced)
                    bulk_output_stream.get() << std::endl;

                // Accumulate this slice's output time (Welford online mean/variance).
                if (profile_slices)
                {
                    double dt_ms = std::chrono::duration_cast<
                        std::chrono::duration<double, std::milli>>(
                            lstk::now() - output_start).count();
                    ++timing_n;
                    double delta = dt_ms - timing_mean_ms;
                    timing_mean_ms += delta / static_cast<double>(timing_n);
                    timing_m2 += delta * (dt_ms - timing_mean_ms);
                    timing_total_ms += dt_ms;
                }
            }
        };

        // Exceptions (deadlock, routing failure, timeout) surface here, in the consumer loop.
        if (parser.exists("graceful"))
        {
            try
            {
                consume_slices();
            }
            catch (const std::exception& e)
            {
                std::cout << "Encountered exception: " << e.what() << std::endl;
                std::cout << "Halting slicing" << std::endl;
            }
        }
        else
            consume_slices();

        // Slice-timing report: one CSV row per run, keyed by layout size, so a sweep over layout
        // sizes can be plotted as avg/error bars (error bar = stddev_ms, or stddev_ms/sqrt(n) for
        // the standard error of the mean). Placed before the minetest early-return so it runs for
        // every output mode.
        if (profile_slices)
        {
            auto [max_row, max_col] = layout->furthest_cell();
            size_t rows  = static_cast<size_t>(max_row) + 1;
            size_t cols  = static_cast<size_t>(max_col) + 1;
            size_t cells = rows * cols;
            double stddev_ms = timing_n > 1
                ? std::sqrt(timing_m2 / static_cast<double>(timing_n - 1)) : 0.0;

            std::string timing_path = parser.get<std::string>("slicetiming");
            if (timing_path.empty())
                timing_path = "slice_timings.csv";

            bool needs_header = !std::filesystem::exists(timing_path);
            std::ofstream timing_file(timing_path, std::ios::app);
            if (!timing_file)
                err_stream << "Warning: could not open slice-timing file: " << timing_path << std::endl;
            else
            {
                if (needs_header)
                    timing_file << "rows,cols,cells,num_slices,mean_ms,stddev_ms,total_s\n";
                timing_file << rows << "," << cols << "," << cells << ","
                            << timing_n << "," << timing_mean_ms << "," << stddev_ms << ","
                            << (timing_total_ms / 1000.0) << "\n";
            }

            out_stream << "Slice-timing: " << timing_n << " slices, mean " << timing_mean_ms
                       << " ms, stddev " << stddev_ms << " ms (layout " << rows << "x" << cols
                       << ", " << cells << " cells) -> " << timing_path << std::endl;
        }

        if (parser.exists("minetest") && print_slices)
        {
            out_stream << "\nRouted " << slice_counter << " slices. Finalizing map.sqlite..."
                       << std::endl;
            if (!minetest_builder->finish())
            {
                err_stream << "Failed to generate map.sqlite." << std::endl;
                return -1;
            }
            out_stream << "Generated map.sqlite (" << minetest_builder->block_count()
                       << " mapblocks)" << std::endl;
            return 0;
        }

        if(parser.exists("o") || parser.exists("noslices"))
        {
            if (output_format_mode == OutputFormatMode::Machine)
            {
                out_stream << slices->result().ls_instructions_count() << ","
                           << slices->result().slice_count() << ","
                           << lstk::seconds_since(start) << std::endl;
            } else if (output_format_mode == OutputFormatMode::Progress || output_format_mode == OutputFormatMode::Stats )
            {
                out_stream << "LS Instructions read  " << slices->result().ls_instructions_count() << std::endl;
                out_stream << "Slices " << slices->result().slice_count() << std::endl;
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