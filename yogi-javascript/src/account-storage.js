(function () {
    class AccountStorage {
        constructor(session) {
            this._session = session;
        }

        store(variable, data) {
            return this._session._storeData(true, variable, data);
        }

        read(variable) {
            return this._session._readData(true, variable);
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.AccountStorage = AccountStorage;
  })();
