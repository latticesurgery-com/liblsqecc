type inputType = 'lli' | 'qasm';

type cnotCorrections = 'never' | 'always';

interface slicerResult {
    err: string;
    exit_code: number;
    output: string;
}

/**
 * Runs the slicer from string input.
 *
 * @param input Circuit content as string
 * @param inputType 'qasm' for QASM input, 'lli' (default) for LLI input.
 * @param compactLayout Use Litinski's compact layout. Default is 'false'.
 * @param cnotCorrections Add Xs and Zs to correct the the negative outcomes: 'never' (default), 'always'
 * @returns Slicer result
 *
 */
export function runSlicer(input: string, inputType?: inputType, compactLayout?: boolean, cnotCorrections?: cnotCorrections): slicerResult;
