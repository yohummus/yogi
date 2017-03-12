describe('ConnectionsObserver', () => {
    let session;
    let testIf;
    let co;

    beforeAll(() => {
        session = new yogi.Session();
        testIf  = new yogi.TestInterface(session);
        co      = new yogi.ConnectionsObserver(session);
    });

    afterAll(() => {
        session.close();
    });

    beforeEach((done) => {
        testIf.run('reset').then(done);
    });

    it('should have a getter for the session', () => {
        expect(co.session).toBe(session);
    });

    it('should support getting a list of all connection factories', (done) => {
        testIf.run('createYogiTcpFactories');
        co.getFactories().then((factories) => {
            expect(factories.tcpClients.length).toBeGreaterThan(0);
            expect(factories.tcpClients[0].id).toBeGreaterThan(0);
            expect(typeof factories.tcpClients[0].host).toBe('string');
            expect(factories.tcpClients[0].port).toBeGreaterThan(0);

            expect(factories.tcpServers.length).toBeGreaterThan(0);
            expect(factories.tcpServers[0].id).toBeGreaterThan(0);
            expect(typeof factories.tcpServers[0].address).toBe('string');
            expect(factories.tcpServers[0].port).toBeGreaterThan(0);

            expect(factories.tcpClients[0].id).not.toEqual(factories.tcpServers[0].id);

            done();
        });
    });

    it('should support getting a list of all connections', (done) => {
        testIf.run('createYogiTcpFactories');
        co.getAll().then((connections) => {
            expect(connections.length).toBeGreaterThan(0);
            expect(connections[0].factoryId).toBeGreaterThan(0);
            expect(typeof connections[0].connected).toBe('boolean');
            expect(typeof connections[0].description).toBe('string');
            expect(typeof connections[0].remoteVersion).toBe('string');
            expect(connections[0].stateChangeTime instanceof Date).toBe(true);

            done();
        });
    });

    it('should support monitoring the state of connections', (done) => {
        let session = new yogi.Session();
        let testIf  = new yogi.TestInterface(session);
        let co      = new yogi.ConnectionsObserver(session);

        testIf.run('createYogiTcpFactories').then(() => {
            co.onChanged = (connection) => {
                co.onChanged = null;
                expect(connection.factoryId).toBeGreaterThan(-1);
                expect(typeof connection.connected).toBe('boolean');
                expect(typeof connection.description).toBe('string');
                expect(typeof connection.remoteVersion).toBe('string');
                expect(connection.stateChangeTime instanceof Date).toBe(true);

                session.close();
                done();
            };

            testIf.run('destroyYogiTcpClient');
        });
    });
});
