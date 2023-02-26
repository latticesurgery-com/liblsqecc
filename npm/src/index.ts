// @ts-ignore
import LsqeccModule from "../wasm/lsqecc_emscripten.js";

import type { Slices, Slice, VisualArrayCell } from "./slices";

type inputType = "lli" | "qasm";

type cnotCorrections = "never" | "always";

type layoutGenerator = "compact" | "edpc" | "";

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
    ): Slices {
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

        const resultAsJson = JSON.parse(result);
        if (resultAsJson.exit_code !== 0) {
            throw new Error(`Exit code ${resultAsJson.exit_code}: ${resultAsJson.err}`);
        }

        return JSON.parse(resultAsJson.output) as Slices;
    }
}

export default Slicer;
export type { Slices, Slice, VisualArrayCell };
