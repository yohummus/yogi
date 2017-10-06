(function () {
    class CustomCommand {
        constructor(session, command, args = []) {
            this._session = session;
            this._cmd = command;
            this._args = args;
            this._id = null;
            this._exitCode = null;

            this._startedPromise = new Promise((resolve, reject) => {
                this._session._requestStartCustomCommand(this)
                .then((cmdId) => {
                    this._id = cmdId;
                    resolve();
                })
                .catch((err) => {
                    reject(err);
                });
            });

            this._terminatedPromise = new Promise((resolve) => {
                this._terminatedPromiseResolve = resolve;
            });
        }

        get session() {
            return this._session;
        }

        get command() {
            return this._cmd;
        }

        get args() {
            return this._args;
        }

        get running() {
            return this._id !== null;
        }

        get exitCode() {
            return this._exitCode;
        }

        writeStdin(str) {
            return new Promise((resolve, reject) => {
                this._startedPromise
                .then(() => {
                    if (this._id === null) {
                        reject('The command failed to start or has already terminated');
                    }
                    this._session._requestWriteCustomCommandStdin(this._id, str)
                    .then(resolve)
                    .catch(reject);
                })
                .catch(reject);
            });
        }

        kill() {
            let promise = new Promise((resolve, reject) => {
                this._session._requestTerminateCustomCommand(this._id)
                .then(() => {
                    this._id = null;
                    resolve();
                })
                .catch(reject);
            });

            return promise;
        }

        get startedPromise() {
            return this._startedPromise;
        }

        get terminatedPromise() {
            return this._terminatedPromise;
        }

        set onStdoutRead(fn) {
            this._onStdoutRead = fn;
        }

        set onStderrRead(fn) {
            this._onStderrRead = fn;
        }

        set onError(fn) {
            this._onError = fn;
        }

        _updateState(running, exitCode, out, err, error) {
            if (out.length > 0) {
                if (this._onStdoutRead) {
                    this._onStdoutRead(out);
                }
            }

            if (err.length > 0) {
                if (this._onStderrRead) {
                    this._onStderrRead(err);
                }
            }

            if (error && this._onError) {
                this._onError(error);
            }

            if (running === false) {
                this._id = null;
                this._exitCode = exitCode;
                this._terminatedPromiseResolve(exitCode);
            }
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.CustomCommand = CustomCommand;
})();
