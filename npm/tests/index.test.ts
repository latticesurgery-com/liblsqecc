import Slicer from "../src";
import LsqeccModule from "../wasm/lsqecc_emscripten.js";

jest.mock("../wasm/lsqecc_emscripten.js");

describe("Slicer", () => {
    let mockSlicer: Slicer;
    let mockRunSlicerProgramFromStrings = jest.fn();

    beforeEach(() => {
        mockRunSlicerProgramFromStrings.mockReset();

        mockSlicer = new (Slicer as any)({
            run_slicer_program_from_strings: mockRunSlicerProgramFromStrings,
        });
    });

    it("return error when inputType is invalid", () => {
        const test = () => mockSlicer.run("some input", "invalid inputType" as any);
        expect(test).toThrow("Invalid argument inputType: invalid inputType");
    });

    it("return error when cnotCorrections is invalid", () => {
        const test = () =>
            mockSlicer.run("some input", "lli", "", "invalid cnotCorrections" as any);
        expect(test).toThrow("Invalid argument cnotCorrections: invalid cnotCorrections");
    });

    describe("valid arguments", () => {
        beforeEach(() => {
            mockRunSlicerProgramFromStrings.mockReturnValue(
                JSON.stringify({
                    err: "",
                    exit_code: 0,
                    output: '{ "result": "something" }',
                })
            );
        });

        it("inputType is qasm", () => {
            const result = mockSlicer.run("some input", "qasm");
            expect(mockRunSlicerProgramFromStrings).toHaveBeenCalledWith(
                " -q --cnotcorrections never",
                "some input"
            );

            expect(result).toEqual({
                result: "something",
            });
        });

        it("no layoutGenerator is not added to command line", () => {
            const result = mockSlicer.run("some input", "lli");
            expect(mockRunSlicerProgramFromStrings).toHaveBeenCalledWith(
                "  --cnotcorrections never",
                "some input"
            );

            expect(result).toEqual({
                result: "something",
            });
        });

        it("layoutGenerator is compact", () => {
            const result = mockSlicer.run("some input", "lli", "compact");
            expect(mockRunSlicerProgramFromStrings).toHaveBeenCalledWith(
                "-L compact  --cnotcorrections never",
                "some input"
            );

            expect(result).toEqual({
                result: "something",
            });
        });

        it("cnotCorrections is always", async () => {
            const result = mockSlicer.run("some input", "qasm", "", "always");
            expect(mockRunSlicerProgramFromStrings).toHaveBeenCalledWith(
                " -q --cnotcorrections always",
                "some input"
            );

            expect(result).toEqual({
                result: "something",
            });
        });
    });

    it("return error when exit code is not 0", () => {
        mockRunSlicerProgramFromStrings.mockReturnValue(
            JSON.stringify({
                err: "some error",
                exit_code: 100,
                output: "",
            })
        );

        const test = () => mockSlicer.run("some input");
        expect(test).toThrow("Exit code 100: some error");
    });
});
