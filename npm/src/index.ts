// @ts-ignore
import LsqeccModule from "../wasm/lsqecc_emscripten.js";

type inputType = "lli" | "qasm";

type cnotCorrections = "never" | "always";

type layoutGenerator = "compact" | "edpc" | "";

interface slicerResult {
    err: string;
    exit_code: number;
    output: string;
}

class Slicer {
    private constructor(protected _module: any) {}

    /**
     * Loads and initializes the WASM module.
     *
     * @returns Promise that resolves to a Slicer instance
     */
    static load(): Promise<Slicer> {
        return LsqeccModule().then((module: any) => new Slicer(module));
    }

    /**
     * Runs the slicer from string input.
     *
     * @param input Circuit content as string
     * @param inputType 'qasm' for QASM input, 'lli' (default) for LLI input.
     * @param layoutGenerator Choose the type of layout generator: 'compact' (default), 'edpc'
     * @param cnotCorrections Add Xs and Zs to correct the the negative outcomes: 'never' (default), 'always'
     * @returns Slicer result
     *
     */
    run(
        input: string,
        inputType: inputType = "lli",
        layoutGenerator: layoutGenerator = "",
        cnotCorrections: cnotCorrections = "never"
    ): slicerResult {
        if (inputType !== "qasm" && inputType !== "lli") {
            throw new Error(`Invalid argument inputType: ${inputType}`);
        }

        if (cnotCorrections !== "never" && cnotCorrections !== "always") {
            throw new Error(`Invalid argument cnotCorrections: ${cnotCorrections}`);
        }

        const commandLineArgs = [
            layoutGenerator ? `-L ${layoutGenerator}` : "",
            inputType === "qasm" ? "-q" : "",
            "--cnotcorrections",
            cnotCorrections === "always" ? "always" : "never",
        ];

        const result = this._module.run_slicer_program_from_strings(
            commandLineArgs.join(" "),
            input
        );

        return JSON.parse(result);
    }
}

export default Slicer;
