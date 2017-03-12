(function () {
    class Binding {
        constructor(terminal, targets) {
            this._terminal    = terminal;
            this._targets     = targets;
            this._established = false;

            this._create();
        }

        get terminal() {
            return this._terminal;
        }

        get targets() {
            return this._targets;
        }

        get alive() {
            return this._terminal.alive && !!this._id;
        }

        get comeAlivePromise() {
            return this._comeAlivePromise;
        }

        get established() {
            return this._established;
        }

        set onBindingStateChanged(fn) {
            this._onBindingStateChanged = fn;
        }

        toString() {
            return `Binding targeting "${this._targets}"`;
        }

        destroy() {
            return new Promise((resolve, reject) => {
                this._comeAlivePromise.then(() => {
                    this._terminal.session._requestDestroyBinding(this)
                    .then(() => {
                        this._id = null;
                        resolve();
                    })
                    .catch((err) => {
                        reject(`Could not destroy ${this}: ${err}`);
                    });
                });
            });
        }

        _create() {
            this._comeAlivePromise = new Promise((resolve, reject) => {
                this._terminal.comeAlivePromise.then(() => {
                    this._terminal.session._requestCreateBinding(this)
                    .then((id) => {
                        this._terminal.session._requestMonitorBindingState(id)
                        .then(() => {
                            this._id = id;
                            resolve();
                        })
                        .catch((err) => {
                            reject(`Could not start monitoring binding state for ${this}: ${err}`);
                        });
                    })
                    .catch((err) => {
                        reject(`Could not create ${this}: ${err}`);
                    });
                });
            });
        }

        _updateBindingState(established) {
            if (this._established !== established) {
                this._established = established;
                if (this._onBindingStateChanged) {
                    this._onBindingStateChanged(established);
                }
            }
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.Binding = Binding;
})();
