describe("MessageFactory", () => {
    it('should support creating message classes with a timestamp and a value', () => {
        let cls = yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x00000806);

        let msg1 = new cls();
        let msg2 = new cls();
        expect(msg1 instanceof yogi.Message).toBe(true);

        msg1.timestamp = 123456;
        msg1.value = 55555;

        let buffer = msg1.serialize();
        expect(buffer.byteLength).toBeGreaterThan(0);

        msg2.deserialize(buffer);
        expect(msg2.timestamp.toString()).toEqual('123456');
        expect(msg2.value).toEqual(55555);
    });

    it('should cache already created message classes', () => {
        let cls1 = yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x000008c6);
        let cls2 = yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x000008c6);
        expect(cls1).toBe(cls2);
    });

    it('should throw an error if custom or reserved signatures are used', () => {
        expect(() => yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x00FFFFFF)).toThrow();
        expect(() => yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x01000000)).toThrow();
    });

    it('should support creating message classes without a timestamp but with a value', () => {
        let cls = yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x00000006);

        let msg = new cls();
        expect(msg instanceof yogi.Message).toBe(true);

        expect(msg.timeout).toBeUndefined();
        expect(msg.value).toBeDefined();
    });

    it('should support creating message classes without a timestamp and without a value', () => {
        let cls = yogi.MessageFactory.createMessageClass(yogi.MessageType.Publish, 0x00000000);

        let msg = new cls();
        expect(msg instanceof yogi.Message).toBe(true);

        expect(msg.timeout).toBeUndefined();
        expect(msg.value).toBeUndefined();
    });

    it('should support creating messages directly', () => {
        let msg = yogi.MessageFactory.createMessage(yogi.MessageType.Publish, 0x00000000);
        expect(msg instanceof yogi.Message).toBe(true);
    });

    it('should create messages that can be serialized and deserialized', () => {
        let msg1 = yogi.MessageFactory.createMessage(yogi.MessageType.Publish, 0x00000005);
        msg1.value = 123;
        let data = msg1.serialize();
        expect(data.byteLength).toBeGreaterThan(0);

        let msg2 = yogi.MessageFactory.createMessage(yogi.MessageType.Publish, 0x00000005);
        msg2.deserialize(data);
        expect(msg2.value).toBe(123);
    });
});
