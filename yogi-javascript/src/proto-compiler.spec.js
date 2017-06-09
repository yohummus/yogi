describe('ProtoCompiler', () => {
    let compiler;
    let file;

    beforeAll(() => {
        compiler = new yogi.ProtoCompiler();
        file     = new yogi.ProtoFile(0x00000c07);
    });

    it('should support compiling to Python', (done) => {
        compiler.compileToPython(file).then((response) => {
            expect(response[`${file.filenameBase}_pb2.py`].indexOf(file.signature.hex)).toBeGreaterThan(-1);
            done();
        });
    });

    it('should support compiling to C++', (done) => {
        compiler.compileToCpp(file).then((response) => {
            expect(response[`${file.filenameBase}.cc`].indexOf(file.signature.hex)).toBeGreaterThan(-1);
            expect(response[`${file.filenameBase}.h`].indexOf(file.signature.hex)).toBeGreaterThan(-1);
            done();
        });
    });

    it('should support compiling to C#', (done) => {
        compiler.compileToCSharp(file).then((response) => {
            expect(response[`${file.filenameBase}.cs`].indexOf(file.signature.hex)).toBeGreaterThan(-1);
            done();
        });
    });

    xit('should support downloading the generated files', (done) => {
        compiler.compileToCppAndDownload(file).then((response) => {
            expect(response[`${file.filenameBase}.cc`].indexOf(file.signature.hex)).toBeGreaterThan(-1);
            expect(response[`${file.filenameBase}.h`].indexOf(file.signature.hex)).toBeGreaterThan(-1);
            done();
        });
    });
});
