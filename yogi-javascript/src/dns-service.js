(function () {
    class DnsService {
        constructor(session) {
            this._session = session;
        }

        lookup(hostOrIp) {
            return this._session._requestDnsLookup(hostOrIp);
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.DnsService = DnsService;
})();
