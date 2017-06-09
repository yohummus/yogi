describe('Workarounds', () => {
    it('should put "Long" into the global namespace', () => {
        expect(window.Long).toBeDefined();
    });

    it('should put "ByteBuffer" into the global namespace', () => {
        expect(window.ByteBuffer).toBeDefined();
    });

    it('should put "ProtoBuf" into the global namespace', () => {
        expect(window.ProtoBuf).toBeDefined();
    });
});
