describe('KnownTerminalsObserver', () => {
    let session;
    let testIf;

    beforeAll((done) => {
        session = new yogi.Session();
        testIf  = new yogi.TestInterface(session);
        kto     = new yogi.KnownTerminalsObserver(session);

        session.comeAlivePromise.then(done);
    });

    afterAll(() => {
        session.close();
    });

    beforeEach((done) => {
        testIf.run('reset').then(() => {
            setTimeout(done, 50);
        });
    });

    it('should have a getter for the session', () => {
        expect(kto.session).toBe(session);
    });

    it('should support getting all terminals', (done) => {
        testIf.run('createTreeTestTerminals').then(() => {
            setTimeout(() => {
                kto.getAll().then((terminals) => {
                    expect(terminals.length).toBeGreaterThan(1);

                    let treeTestTerminal;
                    for (let terminal of terminals) {
                        if (terminal.signature.raw === 0x01000000) {
                            treeTestTerminal = terminal;
                        }
                    }

                    expect(treeTestTerminal).toBeDefined();
                    expect(treeTestTerminal.signature instanceof yogi.Signature).toBe(true);
                    expect(treeTestTerminal.name).toMatch(/.+/);
                    expect(treeTestTerminal.type.prototype instanceof yogi.Terminal).toBe(true);

                    done();
                });
            }, 50);
        });
    });

    it('should support monitoring the known terminals', (done) => {
        let tm;
        kto.onChanged = (info) => {
            if (info.terminal.name !== 'Hello') {
                return;
            }

            expect(info.added).toBe(true);
            expect(info.terminal.name).toMatch('Hello');
            expect(info.terminal.signature.raw).toBe(5);
            expect(info.terminal.type).toBe(yogi.DeafMuteTerminal);

            kto.onChanged = (info) => {
                if (info.terminal.name !== 'Hello') {
                    return;
                }

                kto.onChanged = null;
                expect(info.added).toBe(false);
                expect(info.terminal.name).toMatch('Hello');
                expect(info.terminal.signature.raw).toBe(5);
                expect(info.terminal.type).toBe(yogi.DeafMuteTerminal);
                done();
            };

            tm.destroy();
        };

        tm = new yogi.DeafMuteTerminal(session, 'Hello', 5);
    });

    describe('find() function', () => {
        beforeEach((done) => {
            testIf.run('createTreeTestTerminals').then(() => {
                setTimeout(() => {
                    done();
                }, 100);
            });
        });

        it('should support finding terminals via a case-sensitive substring', (done) => {
            kto.find('Power Modu').then((terminals) => {
                expect(terminals.length).toBeGreaterThan(0);
                expect(terminals[0].name).toMatch(/.*Power Module.*/);
                expect(terminals[0].signature.raw).toBeGreaterThan(0);
                expect(terminals[0].type.prototype instanceof yogi.Terminal).toBe(true);

                kto.find('Power modu').then((terminals) => {
                    expect(terminals.length).toBe(0);

                    done();
                });
            });
        });

        it('should support finding terminals via a case-insensitive substring', (done) => {
            kto.find('Power modu', false).then((terminals) => {
                expect(terminals.length).toBeGreaterThan(0);
                expect(terminals[0].name).toMatch(/.*Power Module.*/);
                expect(terminals[0].signature.raw).toBeGreaterThan(0);
                expect(terminals[0].type.prototype instanceof yogi.Terminal).toBe(true);

                done();
            });
        });
    });

    describe('getSubtree() function', () => {
        let tms;
        beforeAll((done) => {
            tms = [
                new yogi.DeafMuteTerminal(session, '/Power Module/Name', 4),
                new yogi.DeafMuteTerminal(session, '/Power Module/Motor A/Voltage', 8),
                new yogi.DeafMuteTerminal(session, '/Power Module/Motor A/Current', 9),
                new yogi.DeafMuteTerminal(session, '/Power Module/Motor A', 7),
                new yogi.DeafMuteTerminal(session, '/Power Module/Motor B/Voltage', 0),
                new yogi.DeafMuteTerminal(session, '/Remote/Speed Pot/Position', 0),
                new yogi.DeafMuteTerminal(session, 'Power Module/Motor A/Voltage', 2),
                new yogi.DeafMuteTerminal(session, 'Username', 1)
            ];

            let promises = tms.map((tm) => tm.alivePromise);
            Promise.all(promises).then(() => {
                setTimeout(done, 50);
            });
        });

        afterAll((done) => {
            let promises = tms.map((tm) => tm.destroy());
            Promise.all(promises).then(() => {
                setTimeout(done, 10); // need because next it() sometimes fails
            });
        });

        it('should report the tree node\'s name and all associated terminals with their type and signature', (done) => {
            kto.getSubtree('').then((terminals) => {
                expect(terminals.length).toBe(2);

                expect(terminals.find((tm) =>
                    tm.name == 'Power Module' &&
                    tm.terminals.length == 0
                )).toBeDefined();

                expect(terminals.find((tm) =>
                    tm.name == 'Username' &&
                    tm.terminals.length == 1 &&
                    tm.terminals[0].type == yogi.DeafMuteTerminal &&
                    tm.terminals[0].signature.raw == 1
                )).toBeDefined();

                done();
            });
        });

        it('should return the sub-paths under "/" if called with an empty path', (done) => {
            kto.getSubtree('/').then((terminals) => {
                expect(terminals.length).toBe(2);
                expect(terminals.find((tm) => tm.name == 'Power Module')).toBeDefined();
                expect(terminals.find((tm) => tm.name == 'Remote')).toBeDefined();
                done();
            });
        });

        it('should return the sub-paths under "/Power Module" if called with the path "/Power Module"', (done) => {
            kto.getSubtree('/Power Module').then((terminals) => {
                expect(terminals.length).toBe(3);
                expect(terminals.find((tm) => tm.name == 'Name')).toBeDefined();
                expect(terminals.find((tm) => tm.name == 'Motor A')).toBeDefined();
                expect(terminals.find((tm) => tm.name == 'Motor B')).toBeDefined();
                done();
            });
        });

        it('should return the sub-paths under "/Power Module/Motor A" if called with the path "/Power Module/Motor A"', (done) => {
            kto.getSubtree('/Power Module/Motor A').then((terminals) => {
                expect(terminals.length).toBe(2);
                expect(terminals.find((tm) => tm.name == 'Voltage')).toBeDefined();
                expect(terminals.find((tm) => tm.name == 'Current')).toBeDefined();
                done();
            });
        });

        it('should return an empty array if called with the path "/Power Module/Motor"', (done) => {
            kto.getSubtree('/Power Module/Motor').then((terminals) => {
                expect(terminals.length).toBe(0);
                done();
            });
        });

        it('should return the upper-most relative sub-paths if called with an empty path', (done) => {
            kto.getSubtree('').then((terminals) => {
                expect(terminals.length).toBe(2);
                expect(terminals.find((tm) => tm.name == 'Power Module')).toBeDefined();
                expect(terminals.find((tm) => tm.name == 'Username')).toBeDefined();
                done();
            });
        });

        it('should return the sub-paths under "Power Module/Motor A" if called with the path "Power Module/Motor A"', (done) => {
            kto.getSubtree('Power Module/Motor A').then((terminals) => {
                expect(terminals.length).toBe(1);
                expect(terminals.find((tm) => tm.name == 'Voltage')).toBeDefined();
                done();
            });
        });
    });
});
