const { runSlicer } = require('../src/index.js');
const LsqeccModule = require('../wasm/lsqecc_emscripten.js');

jest.mock('../wasm/lsqecc_emscripten.js');

describe('runSlicer', () => {
    const mockRunSlicerProgramFromStrings = jest.fn();

    beforeAll(() => {
        LsqeccModule.mockImplementation(() => ({
            run_slicer_program_from_strings: mockRunSlicerProgramFromStrings,
        }));
    });

    it('return error when inputType is invalid', () => {
        expect(runSlicer('some input', 'invalid inputType', false, 'never'))
        .rejects
        .toThrow('Invalid argument inputType: invalid inputType');
    });

    it('return error when cnotCorrections is invalid', () => {
        expect(runSlicer('some input', 'lli', false, 'invalid cnotCorrections'))
        .rejects
        .toThrow('Invalid argument cnotCorrections: invalid cnotCorrections');
    });
    
    describe('valid arguments', () => {
        beforeEach(() => {
            mockRunSlicerProgramFromStrings.mockReturnValue({
                err: '',
                exit_code: 0,
                output: 'some result',
            });
        });
        
        it('inputType is qasm', async () => {
            const result = await runSlicer('some input', 'qasm');
            expect(LsqeccModule().run_slicer_program_from_strings).toHaveBeenCalledWith(
                ' -q --cnotcorrections never',
                'some input'
            );
            
            expect(result).toEqual({
                err: '',
                exit_code: 0,
                output: 'some result',
            });
        });

        it('compactLayout is true', async () => {
            const result = await runSlicer('some input', 'lli', true);
            expect(LsqeccModule().run_slicer_program_from_strings).toHaveBeenCalledWith(
                '--compactlayout  --cnotcorrections never',
                'some input'
            );
            
            expect(result).toEqual({
                err: '',
                exit_code: 0,
                output: 'some result',
            });
        });

        it('cnotCorrections is always', async () => {
            const result = await runSlicer('some input', 'qasm', true, 'always');
            expect(LsqeccModule().run_slicer_program_from_strings).toHaveBeenCalledWith(
                '--compactlayout -q --cnotcorrections always',
                'some input'
            );
            
            expect(result).toEqual({
                err: '',
                exit_code: 0,
                output: 'some result',
            });
        });
    });    
});
