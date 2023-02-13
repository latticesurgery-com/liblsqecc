const LsqeccModule = require('../wasm/lsqecc_emscripten.js');

/**
 * Runs the slicer from string input.
 * 
 * @param {string} input Circuit content as string
 * @param {string} inputType 'qasm' for QASM input, 'lli' (default) for LLI input.
 * @param {string} layoutGenerator Choose the type of layout generator: 'compact' (default), 'edpc'
 * @param {string} cnotCorrections Add Xs and Zs to correct the the negative outcomes: 'never' (default), 'always'
 * @returns {{err: string, exit_code: number, output: string}} Slicer result
 * 
 */
async function runSlicer(input, inputType='lli', layoutGenerator='', cnotCorrections='never') {
    const loadedModule = await LsqeccModule();

    if (inputType !== 'qasm' && inputType !== 'lli') {
        throw new Error(`Invalid argument inputType: ${inputType}`);
    }

    if (cnotCorrections !== 'never' && cnotCorrections !== 'always') {
        throw new Error(`Invalid argument cnotCorrections: ${cnotCorrections}`);
    }

    const commandLineArgs = [
        layoutGenerator ? `-L {layoutGenerator}` : "",
        inputType === 'qasm' ? '-q' : '',
        '--cnotcorrections',
        cnotCorrections === 'always' ? 'always' : 'never',
    ];

    const result = loadedModule.run_slicer_program_from_strings(commandLineArgs.join(' '), input);

    return result;
}

module.exports = {
    runSlicer,
};
