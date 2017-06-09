describe('Session', () => {
    let session;

    beforeAll(() => {
        session = new yogi.Session();
    });

    afterAll(() => {
        session.close();
    });

    it('should become alive', (done) => {
        session.comeAlivePromise.then(() => {
            expect(session.alive).toBe(true);
            done();
        });
    });

    it('should be closable', (done) => {
        let session2 = new yogi.Session('Second Session');

        session2.diePromise.then(() => {
            expect(session2.alive).toBe(false);
            done();
        });

        session2.comeAlivePromise.then(() => {
            session2.close();
        });
    });

    it('should support multiple simultaneous instances', (done) => {
        let session2 = new yogi.Session('Second Session');
        expect(session2.name).not.toEqual(session.name);
        expect(session2.alive).toBe(false);
        session2.comeAlivePromise.then(() => {
            expect(session.alive).toBe(true);
            session2.close();
            done();
        });
    });

    it('should be constructable with a name', () => {
        let session2 = new yogi.Session('My Session');
        expect(session2.name).toMatch('My Session');
    });

    it('should throw when not constructed properly', () => {
        expect(() => new yogi.ConnectionsObserver({})).toThrow();
        expect(() => new yogi.ConnectionsObserver([])).toThrow();
        expect(() => new yogi.ConnectionsObserver(123)).toThrow();
    });

    it('should support getting the server\'s yogi version', (done) => {
        session.getVersion().then((version) => {
            expect(version).toMatch(/^\d+\.\d+\.\d+(-[a-zA-Z0-9_-]+)?$/);
            done();
        });
    });

    it('should support getting the server\'s current time', (done) => {
        session.getServerTime().then((date) => {
            expect(date instanceof Date).toBe(true);
            done();
        });
    });

    it('should support getting the client\'s IP address', (done) => {
        session.getClientAddress().then((addr) => {
            expect(addr).toMatch(/[0-9.:]+/);
            done();
        });
    });
});
