describe('DnsService', () => {
    let session;
    let dns;

    beforeAll((done) => {
        session = new yogi.Session();
        dns     = new yogi.DnsService(session);

        session.comeAlivePromise.then(done);
    });

    afterAll((done) => {
        session.close();
        session.diePromise.then(done);
    });

    it('should support querying DNS entries', (done) => {
        dns.lookup('127.0.0.1').then((entry) => {
            expect(entry.hostname).toMatch(/.+/);
            expect(entry.addresses.indexOf('127.0.0.1') != -1).toBe(true);
            done();
        });
    });
});
