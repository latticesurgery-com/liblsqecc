type inputType = 'lli' | 'qasm';

type cnotCorrections = 'never' | 'always';

type layoutGenerator = 'compact' | 'edpc';

interface slicerResult {
    err: string;
    exit_code: number;
    output: string;
}

/**
 * Runs the slicer from string input. See for a more detailed description of the parameters
 *
 * @param input Circuit content as string
 * @param inputType 'qasm' for QASM input, 'lli' (default) for LLI input.
 * @param layoutGenerator Choose the type of layout generator: 'compact' (default), 'edpc'
 * @param cnotCorrections Add Xs and Zs to correct the the negative outcomes: 'never' (default), 'always'
 * @returns Slicer result
 *
 */
export function runSlicer(input: string, inputType?: inputType, layoutGenerator?: layoutGenerator, cnotCorrections?: cnotCorrections): slicerResult;
