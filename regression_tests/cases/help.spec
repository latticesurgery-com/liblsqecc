Usage: lsqecc_slicer [options...]
Options:
    -i, --input            File with input. If not provided will read LS Instructions from stdin
    -q, --qasm             File name of file with QASM. When not provided will read as LLI (not QASM)
    -l, --layout           File name of file with layout spec. Defaults to simple layout if none is provided
    -o, --output           File name of output. When not provided outputs to stdout
    -f, --output-format    Requires -o, STDOUT output format: progress, noprogress, machine
    -t, --timeout          Set a timeout in seconds after which stop producing slices
    -r, --router           Set a router: graph_search (default), graph_search_cached
    -g, --graph-search     Set a graph search provider: custom (default), boost (not always available)
    --graceful             If there is an error when slicing, print the error and terminate
    --printlli             Output LLI instead of JSONs
    --noslices             Do the slicing but don't write the slices out
    --cnotcorrections      Add Xs and Zs to correct the the negative outcomes: never (default), always
    --compactlayout        Uses Litinski's compact layout, incompatible with -l
    --edpclayout           Uses a layout specified in the EDPC paper by Beverland et. al., incompatible with -l
    --nostagger            Turns off staggered distillation block timing
    -h, --help             Shows this page        
