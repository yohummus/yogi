(function () {
    class KnownTerminalsObserver {
        constructor(session) {
            this._session = session;
            session._registerKnownTerminalsObserver(this);
        }

        get session() {
            return this._session;
        }

        getAll() {
            if (this._monitoring) {
                return this._session._requestKnownTerminals();
            }
            else {
                this._monitoring = true;
                return this._session._requestMonitorKnownTerminals();
            }
        }

        getSubtree(path) {
            return this._session._requestKnownTerminalsSubtree(path);
        }

        find(nameSubstr, caseSensitive=true) {
            return this._session._requestFindKnownTerminals(nameSubstr, caseSensitive);
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
    window.yogi.KnownTerminalsObserver = KnownTerminalsObserver;
})();
