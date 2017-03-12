(function () {
    class ConnectionsObserver {
        constructor(session) {
            this._session = session;
            session._registerConnectionsObserver(this);
        }

        get session() {
            return this._session;
        }

        getFactories() {
            return this._session._requestGetConnectionFactories();
        }

        getAll() {
            if (this._monitoring) {
                return this._session._requestGetConnections();
            }
            else {
                this._monitoring = true;
                return this._session._requestMonitorConnections();
            }
        }

        get onChanged() {
            return this._onChanged;
        }

        set onChanged(fn) {
            this._onChanged = fn;
            if (!this._monitoring) {
                this.getAll();
            }
        }

        _notifyChange(info) {
            if (this._onChanged) {
                this._onChanged(info);
            }
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.ConnectionsObserver = ConnectionsObserver;
})();
