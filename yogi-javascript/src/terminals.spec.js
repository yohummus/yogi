describe('Terminals', () => {
    let session;
    let testIf;
    let terminals;

    beforeAll((done) => {
        session = new yogi.Session();
        testIf  = new yogi.TestInterface(session);

        terminals = {
            'DeafMute'               : new yogi.DeafMuteTerminal(              session, 'DmTestTerminal',  0x12345678),
            'PublishSubscribe'       : new yogi.PublishSubscribeTerminal(      session, 'PsTestTerminal',  0x12345678),
            'CachedPublishSubscribe' : new yogi.CachedPublishSubscribeTerminal(session, 'CpsTestTerminal', 0x12345678),
            'ScatterGather'          : new yogi.ScatterGatherTerminal(         session, 'SgTestTerminal',  0x12345678),
            'Producer'               : new yogi.ProducerTerminal(              session, 'PcTestTerminal',  0x12345678),
            'Consumer'               : new yogi.ConsumerTerminal(              session, 'PcTestTerminal',  0x12345678),
            'CachedProducer'         : new yogi.CachedProducerTerminal(        session, 'CpcTestTerminal', 0x12345678),
            'CachedConsumer'         : new yogi.CachedConsumerTerminal(        session, 'CpcTestTerminal', 0x12345678),
            'Master'                 : new yogi.MasterTerminal(                session, 'MsTestTerminal',  0x12345678),
            'Slave'                  : new yogi.SlaveTerminal(                 session, 'MsTestTerminal',  0x12345678),
            'CachedMaster'           : new yogi.CachedMasterTerminal(          session, 'CmsTestTerminal', 0x12345678),
            'CachedSlave'            : new yogi.CachedSlaveTerminal(           session, 'CmsTestTerminal', 0x12345678),
            'Service'                : new yogi.ServiceTerminal(               session, 'ScTestTerminal',  0x12345678),
            'Client'                 : new yogi.ClientTerminal(                session, 'ScTestTerminal',  0x12345678)
        };

        bindings = {
            'DeafMute'               : new yogi.Binding(terminals['DeafMute'],               'DmTestTerminal'),
            'PublishSubscribe'       : new yogi.Binding(terminals['PublishSubscribe'],       'PsTestTerminal'),
            'CachedPublishSubscribe' : new yogi.Binding(terminals['CachedPublishSubscribe'], 'CpsTestTerminal'),
            'ScatterGather'          : new yogi.Binding(terminals['ScatterGather'],          'SgTestTerminal')
        };

        let steadyStatePromises = [];
        for (let name in terminals) {
            let terminal = terminals[name];

            if (terminal._isSubscribable) {
                steadyStatePromises.push(new Promise((resolve) => {
                    terminal.onSubscriptionStateChanged = resolve;
                }));
            }

            if (terminal._isBinder) {
                steadyStatePromises.push(new Promise((resolve) => {
                    terminal.onBindingStateChanged = resolve;
                }));
            }
        }

        for (let name in bindings) {
            let binding = bindings[name];
            steadyStatePromises.push(new Promise((resolve) => {
                binding.onBindingStateChanged = resolve;
            }));
        }

        expect(steadyStatePromises.length).toBe(21);

        Promise.all([
            testIf.run('reset'),
            testIf.run('createTestTerminals'),
            testIf.run('createSignatureTestTerminals')
        ].concat(
            steadyStatePromises
        )).then(done);
    });

    afterAll(() => {
        session.close();
    });

    describe('Terminal', () => {
        let terminal;
        beforeAll((done) => {
            terminal = new yogi.DeafMuteTerminal(session, 'T', new yogi.Signature(0x12345678));
            terminal.comeAlivePromise.then(done);
        });

        afterAll((done) => {
            terminal.destroy().then(done);
        });

        it('should create a Signature object if constructed with a raw number as signature', (done) => {
            let terminal = new yogi.DeafMuteTerminal(session, 'A', 0x12345678);
            expect(terminal.signature).toEqual(new yogi.Signature(0x12345678));
            terminal.comeAlivePromise.then(() => {
                terminal.destroy().then(done);
            });
        });

        it('should have a getter for the session', () => {
            expect(terminal.session).toBe(session);
        });

        it('should have a getter for the name', () => {
            expect(terminal.name).toBe('T');
        });

        it('should have a getter for the signature', () => {
            expect(terminal.signature).toEqual(new yogi.Signature(0x12345678));
        });

        it('should have a getter for checking if the terminal is alive', (done) => {
            let terminal = new yogi.DeafMuteTerminal(session, 'A', 0x12345678);
            expect(terminal.alive).toBe(false);
            terminal.comeAlivePromise.then(() => {
                expect(terminal.alive).toBe(true);
                terminal.destroy().then(() => {
                    expect(terminal.alive).toBe(false);
                    done();
                });
            });
        });

        it('should have a toString() function', () => {
            expect(terminal.toString()).toMatch(/^DeafMuteTerminal \"T\" \[.*\]$/);
        });

        it('should be destroyable', (done) => {
            let terminal = new yogi.DeafMuteTerminal(session, 'A', 0x12345678);
            terminal.comeAlivePromise.then(() => {
                terminal.destroy().then(() => {
                    expect(terminal.alive).toBe(false);
                    done();
                });
            });
        });
    });

    describe('Proto Messages', () => {
        it('can be used with Publish-Subscribed based terminals', (done) => {
            let terminal = new yogi.PublishSubscribeTerminal(session, '/PS int32', 0x00000007);
            let binding  = new yogi.Binding(terminal, terminal.name);

            binding.onBindingStateChanged = () => {
                binding.onBindingStateChanged = null;
                expect(binding.established).toBe(true);
                let msg = terminal.makeMessage();
                msg.value = 234;
                terminal.publish(msg);
            };

            terminal.onMessageReceived = (msg) => {
                terminal.onMessageReceived = null;
                expect(typeof msg.value).toEqual('number');

                binding.comeAlivePromise.then(() => {
                    binding.destroy();
                    terminal.destroy().then(done);
                });
            };
        });

        it('can be used with Scatter-Gather based terminals', (done) => {
            let terminal = new yogi.ScatterGatherTerminal(session, '/SG int32', 0x0000d007);
            let binding  = new yogi.Binding(terminal, terminal.name);

            terminal.onSubscriptionStateChanged = (established) => {
                terminal.onSubscriptionStateChanged = null;
                let scatMsg = terminal.makeScatterMessage();
                scatMsg.value = 'Hello';
                terminal.scatterGather(scatMsg, (gathMsg) => {
                    expect(gathMsg.message.value).toBe(scatMsg.value.length);
                });
            };

            terminal.onScatterMessageReceived = (request) => {
                terminal.onScatterMessageReceived = null;
                expect(request.message.value).toEqual('Hello');
                let msg = terminal.makeGatherMessage();
                msg.value = 555;
                request.respond(msg);

                binding.comeAlivePromise.then(() => {
                    binding.destroy();
                    terminal.destroy().then(done);
                });
            };
        });
    });

    let checkSupportsPublishingMessages = function (terminal, testIfCheckCmd, done) {
        expect(true).toBe(true);
        terminal.publish([12, 34]).then(() => {
            testIf.runUntilSuccess(testIfCheckCmd).then(done);
        });
    };

    let checkSupportsMonitoringReceivedPublishMessages = function (terminal, testIfPublishCmd, done) {
        terminal.onMessageReceived = (msg) => {
            if (msg.toString() == [12, 34].toString()) {
                return;
            }

            expect(msg.toString()).toEqual([56, 78].toString());
            done();
        };

        testIf.runUntilSuccess(testIfPublishCmd);
    };

    let checkSupportsMonitoringReceivedCachedPublishMessages = function (terminal, testIfPublishCmd, done) {
        terminal.onMessageReceived = (msg, cached) => {
            if (msg.toString() == [12, 34].toString()) {
                return;
            }

            expect(msg.toString()).toEqual([56, 78].toString());
            expect(cached).toBe(false);
            done();
        };

        testIf.runUntilSuccess(testIfPublishCmd);
    };

    describe('Primitive Terminals', () => {
        describe('PublishSubscribeTerminal', () => {
            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['PublishSubscribe'], 'psCheckPublished', done);
            });

            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedPublishMessages(terminals['PublishSubscribe'], 'psPublish', done);
            });
        });

        describe('CachedPublishSubscribeTerminal', () => {
            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['CachedPublishSubscribe'], 'cpsCheckPublished', done);
            });

            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedCachedPublishMessages(terminals['CachedPublishSubscribe'], 'cpsPublish', done);
            });
        });

        describe('ScatterGatherTerminal', () => {
            let terminal;
            beforeAll(() => {
                terminal = terminals['ScatterGather'];
            });

            describe('Scattered Messages', () => {
                it('should be receivable', (done) => {
                    terminal.onScatterMessageReceived = (msg) => {
                        expect(msg.message.toString()).toEqual([56, 78].toString());
                        done();
                    };

                    testIf.run('sgScatter');
                });

                it('should have a getter for the terminal', (done) => {
                    terminal.onScatterMessageReceived = (msg) => {
                        expect(msg.terminal).toBe(terminal);
                        done();
                    };

                    testIf.run('sgScatter');
                });

                it('should have a toString() function', (done) => {
                    terminal.onScatterMessageReceived = (msg) => {
                        expect(msg.toString()).toMatch(/ScatterMessage \[active\]/);
                        done();
                    };

                    testIf.run('sgScatter');
                });

                it('should be ignored if they are not being monitored', (done) => {
                    terminal.onScatterMessageReceived = null;
                    testIf.run('sgScatter').then(() => {
                        testIf.runUntilSuccess('sgCheckGatheredIgnore').then(done);
                    });
                });

                it('should support ignoring the message explicitly', (done) => {
                    terminal.onScatterMessageReceived = (msg) => {
                        msg.ignore();
                    };

                    testIf.run('sgScatter').then(() => {
                        testIf.runUntilSuccess('sgCheckGatheredIgnore').then(() => {
                            expect(true).toBe(true);
                            done();
                        });
                    });
                });

                it('should support responding to the message', (done) => {
                    terminal.onScatterMessageReceived = (msg) => {
                        msg.respond([12, 34]);
                    };

                    testIf.run('sgScatter').then(() => {
                        testIf.runUntilSuccess('sgCheckGatheredRespond').then(done);
                    });
                });
            });

            describe('Scatter Gather Operations', () => {
                it('should succeed with the correct flags and message', (done) => {
                    terminal.scatterGather([12, 34], (msg) => {
                        expect(msg.flags).toBe(yogi.GatherFlags.Finished);
                        expect(msg.message.toString()).toEqual([56, 78].toString());
                        done();
                    });
                });

                it('should report the correct Operation object', (done) => {
                    let operation;
                    terminal.scatterGather([12, 34], (msg) => {
                        expect(msg.operation).toBe(operation);
                        done();
                    }).then((op) => {
                        operation = op;
                    });
                });
            });
        });
    });

    describe('Convenience Terminals', () => {
        describe('ProducerTerminal', () => {
            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['Producer'], 'consumerCheckPublished', done);
            });
        });

        describe('ConsumerTerminal', () => {
            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedPublishMessages(terminals['Consumer'], 'producerPublish', done);
            });
        });

        describe('CachedProducerTerminal', () => {
            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['CachedProducer'], 'cachedConsumerCheckPublished', done);
            });
        });

        describe('CachedConsumerTerminal', () => {
            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedCachedPublishMessages(terminals['CachedConsumer'], 'cachedProducerPublish', done);
            });
        });

        describe('MasterTerminal', () => {
            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedPublishMessages(terminals['Master'], 'slavePublish', done);
            });

            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['Master'], 'slaveCheckPublished', done);
            });
        });

        describe('SlaveTerminal', () => {
            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedPublishMessages(terminals['Slave'], 'masterPublish', done);
            });

            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['Slave'], 'masterCheckPublished', done);
            });
        });

        describe('CachedMasterTerminal', () => {
            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedCachedPublishMessages(terminals['CachedMaster'], 'cachedSlavePublish', done);
            });

            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['CachedMaster'], 'cachedSlaveCheckPublished', done);
            });
        });

        describe('CachedSlaveTerminal', () => {
            it('should support monitoring received messages', (done) => {
                checkSupportsMonitoringReceivedCachedPublishMessages(terminals['CachedSlave'], 'cachedMasterPublish', done);
            });

            it('should support publishing messages', (done) => {
                checkSupportsPublishingMessages(terminals['CachedSlave'], 'cachedMasterCheckPublished', done);
            });
        });

        describe('ServiceTerminal', () => {
            let terminal;
            beforeAll(() => {
                terminal = terminals['Service'];
            });

            describe('Requests', () => {
                it('should be receivable', (done) => {
                    terminal.onRequestReceived = (msg) => {
                        expect(msg.message.toString()).toEqual([56, 78].toString());
                        done();
                    };

                    testIf.run('scRequest');
                });

                it('should have a getter for the terminal', (done) => {
                    terminal.onRequestReceived = (msg) => {
                        expect(msg.terminal).toBe(terminal);
                        done();
                    };

                    testIf.run('scRequest');
                });

                it('should have a toString() function', (done) => {
                    terminal.onRequestReceived = (msg) => {
                        expect(msg.toString()).toMatch(/Request \[active\]/);
                        done();
                    };

                    testIf.run('scRequest');
                });

                it('should be ignored if they are not being monitored', (done) => {
                    terminal.onRequestReceived = null;
                    testIf.run('scRequest').then(() => {
                        testIf.runUntilSuccess('scCheckIgnored').then(done);
                    });
                });

                it('should support ignoring the message explicitly', (done) => {
                    terminal.onRequestReceived = (msg) => {
                        msg.ignore();
                    };

                    testIf.run('scRequest').then(() => {
                        testIf.runUntilSuccess('scCheckIgnored').then(() => {
                            expect(true).toBe(true);
                            done();
                        });
                    });
                });

                it('should support responding to the message', (done) => {
                    terminal.onRequestReceived = (msg) => {
                        msg.respond([12, 34]);
                    };

                    testIf.run('scRequest').then(() => {
                        testIf.runUntilSuccess('scCheckResponded').then(done);
                    });
                });
            });
        });

        describe('ClientTerminal', () => {
            let terminal;
            beforeAll(() => {
                terminal = terminals['Client'];
            });

            describe('Request Operations', () => {
                it('should succeed with the correct flags and message', (done) => {
                    terminal.request([12, 34], (msg) => {
                        expect(msg.flags).toBe(yogi.GatherFlags.Finished);
                        expect(msg.message.toString()).toEqual([56, 78].toString());
                        done();
                    });
                });

                it('should report the correct Operation object', (done) => {
                    let operation;
                    terminal.request([12, 34], (msg) => {
                        expect(msg.operation).toBe(operation);
                        done();
                    }).then((op) => {
                        operation = op;
                    });
                });
            });
        });
    });
});
