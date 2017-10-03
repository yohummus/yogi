(function () {
    class SessionStorage {
        constructor(session) {
            this._session = session;
        }

        store(variable, data) {
            return this._session._storeData(false, variable, data);
        }

        read(variable) {
            return this._session._readData(false, variable);
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.SessionStorage = SessionStorage;
})();
