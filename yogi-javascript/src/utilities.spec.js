describe('Utilities', () => {
    it('should support gettting the HUB\'s protocol type', () => {
        expect(yogi.hubProtocol + ':').toMatch(window.location.protocol);
    });

    it('should support gettting the HUB\'s hostname or address', () => {
        expect(yogi.hubHost).toEqual(window.location.hostname);
    });

    it('should support gettting the HUB\'s port', () => {
        expect(yogi.hubPort).toEqual(parseInt(window.location.port !== '' ? window.location.port : '80'));
    });

    it('should support gettting the HUB\'s URI', () => {
        let loc = window.location;
        expect(yogi.hubUri).toMatch(loc.protocol + '//' + loc.host);
    });
});
