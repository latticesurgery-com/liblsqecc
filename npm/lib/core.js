const LsqeccModule = require('../wasm/lsqecc_emscripten.js');

/**
 * Runs the slicer from QASM. Only supports a small subset of OpenQASM 2.0. See the README for details.
 * 
 * @param {string} input QASM circuit content as string
 * @param {boolean} compactLayout Use Litinski's compact layout. Default is 'false'.
 * @param {string} cnotCorrections Add Xs and Zs to correct the the negative outcomes: 'never' (default), 'always'
 * @returns {{err: string, exit_code: number, output: string}} Slicer result
 * 
 */
const slicerFromQasm = async (input, compactLayout=false, cnotCorrections='never') => {
    const loadedModule = await LsqeccModule();

    const commandLineArgs = [
        compactLayout ? '--compactlayout' : '',
        '-q',
        '--cnotcorrections',
        cnotCorrections === 'always' ? 'always' : 'never',
    ];

    const result = loadedModule.run_slicer_program_from_strings(commandLineArgs.join(' '), input);

    return result;
}

/**
 * Runs the slicer from LLI instruction.
 * 
 * @param {string} input QASM circuit content as string
 * @param {boolean} compactLayout Use Litinski's compact layout. Default is 'false'.
 * @param {string} cnotCorrections Add Xs and Zs to correct the the negative outcomes: 'never' (default), 'always'
 * @returns {{err: string, exit_code: number, output: string}} Slicer result
 */
const slicerFromLLI = async (input, compactLayout=false, cnotCorrections='never') => {
    const loadedModule = await LsqeccModule();

    const commandLineArgs = [
        compactLayout ? '--compactlayout' : '',
        '--cnotcorrections',
        cnotCorrections === 'always' ? 'always' : 'never',
    ];

    const result = loadedModule.run_slicer_program_from_strings(commandLineArgs.join(' '), input);

    return result;
}

module.exports = {
    slicerFromQasm,
    slicerFromLLI,
};
