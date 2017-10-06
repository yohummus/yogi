(function () {
    class TestInterface {
        constructor(session) {
            this._session = session;
        }

        run(command) {
            return this._session._requestTestCommand(command);
        }

        runUntilSuccess(command, retries=1000) {
            return new Promise((resolve, reject) => {
                let i = 0;
                let fn;
                fn = (err) => {
                    if (err) {
                        ++i;
                        if (i < retries) {
                            this.run(command).then(fn, fn);
                        }
                        else {
                            reject(`Tried to send the test command "${command}" ${retries} times without success`);
                        }
                    }
                    else {
                        resolve();
                    }
                };

                this.run(command).then(fn, fn);
            });
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.TestInterface = TestInterface;
})();
