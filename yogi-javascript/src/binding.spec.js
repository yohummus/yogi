describe('Binding', () => {
    let session;
    let testIf;
    let terminal;
    let binding;

    beforeAll((done) => {
        session  = new yogi.Session();
        testIf   = new yogi.TestInterface(session);
        terminal = new yogi.DeafMuteTerminal(session, 'T', 0x12345678);
        binding  = new yogi.Binding(terminal, 'DmTestTerminal');

        Promise.all([
            testIf.run('reset'),
            binding.comeAlivePromise
        ]).then(() => {
            setTimeout(done, 50);
        });
    });

    afterAll(() => {
        session.close();
    });

    it('should have a getter for the terminal', () => {
        expect(binding.terminal).toBe(terminal);
    });

    it('should have a getter for targets', () => {
        expect(binding.targets).toBe('DmTestTerminal');
    });

    it('should have a getter for checking if the binding is alive', (done) => {
        let binding = new yogi.Binding(terminal, 'A');
        expect(binding.alive).toBe(false);
        binding.comeAlivePromise.then(() => {
            expect(binding.alive).toBe(true);
            binding.destroy().then(() => {
                expect(binding.alive).toBe(false);
                done();
            });
        });
    });

    it('should have a valid state when alive', () => {
        expect(binding.established === true || binding.established === false).toBe(true);
    });

    it('should support monitoring its state', (done) => {
        binding.onBindingStateChanged = (established) => {
            expect(established).toBe(true);
            expect(binding.established).toBe(true);
            done();
        };

        testIf.run('createTestTerminals');
    });

    it('should have a toString() function', () => {
        expect(binding.toString()).toMatch(/^Binding targeting \"DmTestTerminal\"$/);
    });

    it('should be destroyable', (done) => {
        let binding = new yogi.Binding(terminal, 'B');
        binding.comeAlivePromise.then(() => {
            binding.destroy().then(() => {
                expect(binding.alive).toBe(false);
                done();
            });
        });
    });
});
