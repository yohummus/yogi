describe('TestInterface', () => {
    let session;
    let testIf;

    beforeAll(() => {
        session = new yogi.Session();
        testIf  = new yogi.TestInterface(session);
    });

    afterEach(() => {
        session.close();
    });

    it('should support sending test commands', (done) => {
        expect(testIf).toBeDefined();
        testIf.run('reset').then(done);
    });
});
