describe('Utilities', () => {
    it('should support gettting the HUB\'s protocol type', () => {
        expect(yogi.hubProtocol + ':').toMatch(window.location.protocol);
    });

    it('should support gettting the HUB\'s hostname or address', () => {
        expect(yogi.hubHost).toEqual('127.0.0.1');
    });

    it('should support gettting the HUB\'s port', () => {
        expect(yogi.hubPort).toEqual(1234);
    });

    it('should support gettting the HUB\'s URI', () => {
        let loc = window.location;
        expect(yogi.hubUri).toMatch('http://127.0.0.1:1234');
    });
});
