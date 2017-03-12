describe('CustomCommand', () => {
    let session;

    beforeAll((done) => {
        session = new yogi.Session();
        session.comeAlivePromise.then(done);
    });

    afterAll((done) => {
        session.close();
        session.diePromise.then(done);
    });

    it('should support running commands with parameters', (done) => {
        let cmd = new yogi.CustomCommand(session, 'talk-to-me', ['one', 'two', 'three']);

        let error = null;
        cmd.onError = (err) => {
            error = err;
        };

        let stringOut = '';
        cmd.onStdoutRead = (str) => {
            stringOut += str;
        };

        cmd.startedPromise.then(() => {
            expect(cmd.running).toBe(true);
            cmd.terminatedPromise.then((exitCode) => {
                terminated = true;
                expect(cmd.running).toBe(false);
                expect(exitCode).toBe(0);
                expect(cmd.exitCode).toBe(0);
                expect(error).toBe(null);
                expect(stringOut).toEqual('hi three two three one two one two three\n');
                done();
            });
        });
    });

    it('should support writing to a command\'s stdin and killing it', (done) => {
        let cmd = new yogi.CustomCommand(session, 'cat');

        let written = false;
        cmd.writeStdin('dog\n').then(() => {
            written = true;
        });

        let stringOut = '';
        cmd.onStdoutRead = (str) => {
            stringOut += str;

            cmd.kill().then(() => {
                cmd.terminatedPromise.then((exitCode) => {
                    expect(written).toBe(true);
                    expect(stringOut).toEqual('dog\n');
                    done();
                });
            });
        };
    });
});
