(function () {
    let msgClassCache = new Map();

    const MessageType = {
        Publish : 0,
        Master  : 1,
        Slave   : 2,
        Scatter : 3,
        Gather  : 4
    };

    class Message {
        constructor() {
            this._protoMsg = new this.constructor._ProtoMessage();
        }

        serialize() {
            return this._protoMsg.toArrayBuffer();
        }

        deserialize(buffer) {
            this._protoMsg = this.constructor._ProtoMessage.decode(buffer);
        }
    }

    let TimestampMixin = (superclass) => class extends superclass {
        get timestamp() {
            return this._protoMsg.timestamp;
        }

        set timestamp(val) {
            this._protoMsg.timestamp = val;
        }
    };

    let DataMixin = (superclass) => class extends superclass {
        get value() {
            return this._protoMsg.value;
        }

        set value(val) {
            this._protoMsg.value = val;
        }
    };

    class MessageFactory {
        static createMessageClass(type, signature) {
            let sig = signature instanceof window.yogi.Signature ? signature : new window.yogi.Signature(signature);

            let signatureHalf = this._makeSignatureHalf(type, sig);

            let cls = msgClassCache.get(signatureHalf.raw);
            if (!cls) {
                cls = this._makeMsgClass(type, signature, signatureHalf);
                msgClassCache.set(signatureHalf.raw, cls);
            }

            return cls;
        }

        static _makeSignatureHalf(type, signature) {
            return (type === MessageType.Scatter || type === MessageType.Master) ? signature.upperHalf : signature.lowerHalf;
        }

        static _makeMsgClass(type, signature, signatureHalf) {
            let protoFile = new window.yogi.ProtoFile(signature);
            const msgName = ['PublishMessage', 'MasterMessage', 'SlaveMessage', 'ScatterMessage', 'GatherMessage'][type];
            let protoMsgClass = dcodeIO.ProtoBuf.loadProto(protoFile.content).build(`${protoFile.package}.${msgName}`);
            return this._makeSignatureSpecificClass(signatureHalf, protoMsgClass);
        }

        static _makeSignatureSpecificClass(signatureHalf, protoMsgClass) {
            let base = Message;
            if (signatureHalf.hasTimestamp) {
                base = TimestampMixin(base);
            }
            if (signatureHalf.hasData) {
                base = DataMixin(base);
            }

            let immediateBase = class extends base {
                static get signatureHalf() {
                    return signatureHalf;
                }

                static get _ProtoMessage() {
                    return protoMsgClass;
                }
            };

            if (signatureHalf.isPair && !signatureHalf.isList) {
                return class extends immediateBase {
                    constructor() {
                        super(arguments);
                        this.value = new protoMsgClass.Pair();
                    }
                }
            }
            else {
                return immediateBase;
            }
        }

        static createMessage(type, signature) {
            let msg = new (this.createMessageClass(type, signature))();
            let sigHalf = msg.constructor.signatureHalf;
            if (!sigHalf.isList) {
                let types = sigHalf.primitiveTypes;
                let jsonType = window.yogi.PrimitiveType.JSON;
                if (sigHalf.isPair) {
                    if (types.first === jsonType) {
                        msg.value.first = '{}';
                    }
                    if (types.second === jsonType) {
                        msg.value.second = '{}';
                    }
                }
                else {
                    if (types.first === jsonType) {
                        msg.value = '{}';
                    }
                }
            }

            return msg;
        }
    }

    window.yogi = window.yogi || {};
    window.yogi.MessageType    = MessageType;
    window.yogi.Message        = Message;
    window.yogi.MessageFactory = MessageFactory;
})();
