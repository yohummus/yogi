describe("OfficialSignatureHalf", () => {
    it('should have a getter for checking if the message contains a list', () => {
        expect(new yogi.OfficialSignatureHalf(0 << 10).isList).toBe(false);
        expect(new yogi.OfficialSignatureHalf(1 << 10).isList).toBe(true);
    });

    it('should have a getter for checking if the message contains pairs', () => {
        expect(new yogi.OfficialSignatureHalf(0 << 5).isPair).toBe(false);
        expect(new yogi.OfficialSignatureHalf(1 << 5).isPair).toBe(true);
    });

    it('should have a getter for checking if the message contains a timestamp', () => {
        expect(new yogi.OfficialSignatureHalf(0 << 11).hasTimestamp).toBe(false);
        expect(new yogi.OfficialSignatureHalf(1 << 11).hasTimestamp).toBe(true);
    });

    it('should have a getter for checking if the message contains data fields', () => {
        expect(new yogi.OfficialSignatureHalf(0).hasData).toBe(false);
        expect(new yogi.OfficialSignatureHalf(1).hasData).toBe(true);
    });

    it('should have a getter for the primitive types', () => {
        expect(new yogi.OfficialSignatureHalf(12 << 5 | 6).primitiveTypes).toEqual({
            first  : yogi.PrimitiveType.UINT16,
            second : yogi.PrimitiveType.DOUBLE
        });
    });

    it('should have a getter for the primitive type names', () => {
        expect(new yogi.OfficialSignatureHalf(yogi.PrimitiveType.DOUBLE << 5 | yogi.PrimitiveType.UINT16).primitiveTypeNames).toEqual({
            first  : 'uint16',
            second : 'double'
        });
    });

    it('should have a getter for the primitive proto type names', () => {
        expect(new yogi.OfficialSignatureHalf(yogi.PrimitiveType.DOUBLE << 5 | yogi.PrimitiveType.UINT16).primitiveProtoTypeNames).toEqual({
            first  : 'uint32',
            second : 'double'
        });
    });

    it('should have a getter for checking if the signature half is reserved', () => {
        expect(new yogi.OfficialSignatureHalf(10).isReserved).toBe(false);
        expect(new yogi.OfficialSignatureHalf(28).isReserved).toBe(true);
    });

    it('should have a toString() function', () => {
        let half = new yogi.OfficialSignatureHalf(yogi.PrimitiveType.DOUBLE << 5 | yogi.PrimitiveType.UINT16 | 1 << 10 | 1 << 11);
        expect(half.toString()).toMatch(/^timestamped list of pair\(double, uint16\)$/);
    });
});

describe("Signature", () => {
    it('should have a getter for the raw number', () => {
        expect(new yogi.Signature(123456).raw).toBe(123456);
    });

    it('should have a getter for checking if the signature is custom or official', () => {
        expect(new yogi.Signature(0x00FFFFFF).isCustom).toBe(false);
        expect(new yogi.Signature(0x01000000).isCustom).toBe(true);
    });

    it('should have a getter for checking if the signature is reserved', () => {
        expect(new yogi.Signature(0x00000000).isReserved).toBe(false);
        expect(new yogi.Signature(0x00FFFFFF).isReserved).toBe(true);
        expect(new yogi.Signature(0x01FFFFFF).isReserved).toBe(false);
    });

    it('should have a getter for checking if the signature represents a Protocol Buffers message', () => {
        expect(new yogi.Signature(0x00000000).representsProtoMessage).toBe(true);
        expect(new yogi.Signature(0x00FFFFFF).representsProtoMessage).toBe(false);
        expect(new yogi.Signature(0x01FFFFFF).representsProtoMessage).toBe(false);
    });

    it('should have a getter for individual bytes', () => {
        expect(new yogi.Signature(0x12345678).bytes).toEqual([0x78, 0x56, 0x34, 0x12]);
    });

    it('should have a getter for individual bits', () => {
        expect(new yogi.Signature(0x12345678).bits).toEqual([0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0]);
    });

    it('should have a getter for the hexadecimal representation', () => {
        expect(new yogi.Signature(0x12345678).hex).toEqual("12345678");
    });

    it('should have a toString() function', () => {
        expect(new yogi.Signature(0x12345678).toString()).toMatch(/^Signature [0-9a-f]{8} \(custom\)$/);
    });

    it('should have getters for the halfs of an official signature', () => {
        let sig = new yogi.Signature(0x00000001);
        expect(sig.lowerHalf instanceof yogi.OfficialSignatureHalf).toBe(true);
        expect(sig.upperHalf instanceof yogi.OfficialSignatureHalf).toBe(true);
        expect(sig.lowerHalf).not.toEqual(sig.upperHalf);
    });
});

describe("ProtoFile", () => {
    let signature;
    let file;
    beforeEach(() => {
        signature = new yogi.Signature(0x00000c07);
        file      = new yogi.ProtoFile(signature);
    });

    it('should be constructible with raw signatures', () => {
        expect((new yogi.ProtoFile(0x00000c07)).signature).toEqual(signature);
    });

    it('should not be constructible with reserved signatures', () => {
        expect(() => new yogi.ProtoFile(0x00FFFFFF)).toThrow();
    });

    it('should not be constructible with custom signatures', () => {
        expect(() => new yogi.ProtoFile(0x01000000)).toThrow();
    });

    it('should have a getter for the signature', () => {
        expect(file.signature).toBe(signature);
    });

    it('should have a getter for the package string', () => {
        expect(file.package).toMatch(`yogi_${signature.hex}`);
    });

    it('should have a getter for the filename base', () => {
        expect(file.filenameBase).toMatch(`yogi_${signature.hex}`);
    });

    it('should have a getter for the filename', () => {
        expect(file.filename).toMatch(`yogi_${signature.hex}.proto`);
    });

    it('should have a getter for the content', () => {
        expect(file.content.indexOf(file.package))       .toBeGreaterThan(-1);
        expect(file.content.indexOf('syntax = "proto3"')).toBeGreaterThan(-1);
        expect(file.content.indexOf('ScatterMessage'))   .toBeGreaterThan(-1);
        expect(file.content.indexOf('GatherMessage'))    .toBeGreaterThan(-1);
        expect(file.content.indexOf('PublishMessage'))   .toBeGreaterThan(-1);
        expect(file.content.indexOf('MasterMessage'))    .toBeGreaterThan(-1);
        expect(file.content.indexOf('SlaveMessage'))     .toBeGreaterThan(-1);
    });
});