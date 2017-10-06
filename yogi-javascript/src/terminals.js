(function () {
    class Terminal {
        constructor(session, type, name, signature, sendMsgType, recvMsgType) {
            this._session             = session;
            this._name                = name;
            this._signature           = signature instanceof window.yogi.Signature ? signature : new window.yogi.Signature(signature);
            this._sendOrScatterMsgCls = this._createMsgCls(sendMsgType);
            this._recvOrGatherMsgCls  = this._createMsgCls(recvMsgType);

            this._create(type);
        }

        get session() {
            return this._session;
        }

        get name() {
            return this._name;
        }

        get signature() {
            return this._signature;
        }

        get alive() {
            return this._session.alive && !!this._id;
        }

        get comeAlivePromise() {
            return this._comeAlivePromise;
        }

        toString() {
            return `${this.constructor.name} "${this._name}" [${this._signature}]`;
        }

        destroy() {
            return new Promise((resolve, reject) => {
                this._comeAlivePromise.then(() => {
                    this._session._requestDestroyTerminal(this)
                    .then(() => {
                        this._id = null;
                        resolve();
                    })
                    .catch((err) => {
                        reject(`Could not destroy ${this}: ${err}`);
                    });
                });
            });
        }

        _createMsgCls(type) {
            if (type !== undefined && this._signature.representsProtoMessage) {
                return window.yogi.MessageFactory.createMessageClass(type, this._signature);
            }

            return undefined;
        }

        _create(type) {
            this._comeAlivePromise = new Promise((resolve, reject) => {
                window.yogi.nextTick(() => { // next tick to send request after destroy requests have been sent
                    this._session._requestCreateTerminal(this, type)
                    .then((id) => {
                        let promises = [];
                        let cachedMessageData = null;

                        if (this._isSubscribable) {
                            promises.push(this._session._requestMonitorSubscriptionState(id));
                        }

                        if (this._isBinder) {
                            promises.push(this._session._requestMonitorBuiltinBindingState(id));
                        }

                        if (this._isPublishMessageReceiver) {
                            let promise = this._session._requestMonitorReceivedPublishMessages(id);
                            promise.then((data) => cachedMessageData = data);
                            promises.push(promise);
                        }

                        if (this._isScatterMessageReceiver) {
                            promises.push(this._session._requestMonitorReceivedScatterMessages(id));
                        }

                        Promise.all(promises)
                        .then(() => {
                            this._id = id;
                            resolve();

                            if (cachedMessageData !== null) {
                                this._notifyReceivedMessage(cachedMessageData, true);
                            }
                        })
                        .catch((err) => {
                            reject(`Could not start monitoring subscription state or messages for ${this}: ${err}`);
                        });
                    })
                    .catch((err) => {
                        reject(`Could not create ${this}: ${err}`);
                    });
                });
            });
        }

        static _terminalTypeToTerminalClass(typeNumber) {
            switch (typeNumber) {
                case TM_DEAF_MUTE:
                    return DeafMuteTerminal;

                case TM_PUBLISH_SUBSCRIBE:
                    return PublishSubscribeTerminal;

                case TM_SCATTER_GATHER:
                    return ScatterGatherTerminal;

                case TM_CACHED_PUBLISH_SUBSCRIBE:
                    return CachedPublishSubscribeTerminal;

                case TM_PRODUCER:
                    return ProducerTerminal;

                case TM_CONSUMER:
                    return ConsumerTerminal;

                case TM_CACHED_PRODUCER:
                    return CachedProducerTerminal;

                case TM_CACHED_CONSUMER:
                    return CachedConsumerTerminal;

                case TM_MASTER:
                    return MasterTerminal;

                case TM_SLAVE:
                    return SlaveTerminal;

                case TM_CACHED_MASTER:
                    return CachedMasterTerminal;

                case TM_CACHED_SLAVE:
                    return CachedSlaveTerminal;

                case TM_SERVICE:
                    return ServiceTerminal;

                case TM_CLIENT:
                    return ClientTerminal;

                default:
                    throw new Error('Invalid Terminal type number: ' + typeNumber);
            }
        }
    }

    class PrimitiveTerminal extends Terminal {
    }

    class ConvenienceTerminal extends Terminal {
    }

    let BinderMixin = (superclass) => class extends superclass {
        constructor() {
            super(...arguments);
            this._isBinder = true;
            this._established = false;
        }

        get established() {
            return this._established;
        }

        get onBindingStateChanged() {
            return this._onBindingStateChanged;
        }

        set onBindingStateChanged(fn) {
            this._onBindingStateChanged = fn;
        }

        _updateBindingState(established) {
            if (this._established !== established) {
                this._established = established;
                if (this._onBindingStateChanged) {
                    this._onBindingStateChanged(established);
                }
            }
        }
    };

    let SubscribableMixin = (superclass) => class extends superclass {
        constructor() {
            super(...arguments);
            this._isSubscribable = true;
            this._subscribed = false;
        }

        get subscribed() {
            return this._subscribed;
        }

        get onSubscriptionStateChanged() {
            return this._onSubscriptionStateChanged;
        }

        set onSubscriptionStateChanged(fn) {
            this._onSubscriptionStateChanged = fn;
        }

        _updateSubscriptionState(subscribed) {
            if (this._subscribed !== subscribed) {
                this._subscribed = subscribed;
                if (this._onSubscriptionStateChanged) {
                    this._onSubscriptionStateChanged(subscribed);
                }
            }
        }
    };

    let PublisherMixin = (superclass) => class extends superclass {
        constructor() {
            super(...arguments);
        }

        makeMessage() {
            if (!this._signature.representsProtoMessage) {
                throw new Error('Cannot create message for Terminal whose signature does not represent a Protocol Buffers message');
            }
            else {
                return new this._sendOrScatterMsgCls();
            }
        }

        publish(msg) {
            let data = msg;
            if (this._signature.representsProtoMessage) {
                if (!(msg instanceof this._sendOrScatterMsgCls)) {
                    throw new Error('Incompatible message type');
                }

                data = msg.serialize();
            }

            return this._session._requestPublish(this._id, data);
        }
    };

    let PublishMessageReceiverMixin = (superclass) => class extends superclass {
        constructor() {
            super(...arguments);
            this._isPublishMessageReceiver = true;
        }

        get onMessageReceived() {
            return this._onMessageReceived;
        }

        set onMessageReceived(fn) {
            this._onMessageReceived = fn;
        }

        _notifyReceivedMessage(data, cached) {
            if (!this._onMessageReceived) {
                return;
            }

            let msg = data;
            if (this._signature.representsProtoMessage) {
                msg = new this._recvOrGatherMsgCls();
                msg.deserialize(data);
            }

            this._onMessageReceived(msg, cached);
        }
    };

    let ScattererMixin = (superclass) => class extends superclass {
        constructor() {
            super(...arguments);
            this._operations = new Map();
        }

        makeScatterMessage() {
            if (!this._signature.representsProtoMessage) {
                throw new Error('Cannot create message for Terminal whose signature does not represent a Protocol Buffers message');
            }
            else {
                return new this._sendOrScatterMsgCls();
            }
        }

        _scatterGather(msg, gatherFn) {
            let data = msg;
            if (this._signature.representsProtoMessage) {
                if (!(msg instanceof this._sendOrScatterMsgCls)) {
                    throw new Error('Incompatible message type');
                }

                data = msg.serialize();
            }

            let promise = this._session._requestScatterGather(this, data);
            promise.then((operation) => {
                this._operations.set(operation._id, (msg, flags) => {
                    gatherFn(new GatherMessage(operation, flags, msg));
                });
            });
            return promise;
        }

        _notifyReceivedGatherMessage(opId, data, flags) {
            let operation = this._operations.get(opId);
            if (operation) {
                let msg = data;
                if (this._signature.representsProtoMessage) {
                    msg = new this._recvOrGatherMsgCls();
                    msg.deserialize(data);
                }

                operation(msg, flags);

                if (flags & GatherFlags.Finished) {
                    this._operations.delete(opId);
                    operation._id = null;
                }
            }
        }
    };

    let ScatterMessageReceiverMixin = (superclass) => class extends superclass {
        constructor() {
            super(...arguments);
            this._isScatterMessageReceiver = true;
        }

        makeGatherMessage() {
            if (!this._signature.representsProtoMessage) {
                throw new Error('Cannot create message for Terminal whose signature does not represent a Protocol Buffers message');
            }
            else {
                return new this._recvOrGatherMsgCls();
            }
        }

        _notifyReceivedScatterMessage(opId, data) {
            let msg = data;
            if (this._signature.representsProtoMessage) {
                msg = new this._sendOrScatterMsgCls();
                msg.deserialize(data);
            }

            let msgObj = new ScatterMessage(this, opId, msg);
            if (this._onScatterMessageReceived) {
                this._onScatterMessageReceived(msgObj);
            }
            else {
                msgObj.ignore();
            }
        }
    };

    class Operation {
        constructor(terminal, id) {
            this._terminal = terminal;
            this._id = id;
        }

        get terminal() {
            return this._terminal;
        }

        get finished() {
            return this._id === null;
        }

        get id() {
            return this._id;
        }

        toString() {
            return `${this.constructor.name} [${this._id}]`;
        }
    }

    class ScatterMessage {
        constructor(terminal, opId, msg) {
            this._terminal = terminal;
            this._opId     = opId;
            this._msg      = msg;
        }

        get terminal() {
            return this._terminal;
        }

        get message() {
            return this._msg;
        }

        respond(msg) {
            let data = msg;
            if (this._terminal.signature.representsProtoMessage) {
                if (!(msg instanceof this._terminal._recvOrGatherMsgCls)) {
                    throw new Error('Incompatible message type');
                }

                data = msg.serialize();
            }

            let promise = this._terminal._session._requestReactToScatteredMessage(this._terminal._id, this._opId, data, false);
            this._opId = null;
            return promise;
        }

        ignore() {
            let promise = this._terminal._session._requestReactToScatteredMessage(this._terminal._id, this._opId, new ArrayBuffer(0), true);
            this._opId = null;
            return promise;
        }

        toString() {
            let status;
            if (this._opId === null) {
                status = 'handled';
            }
            else if (this._opId > 0) {
                status = 'active';
            }
            else {
                status = 'INVALID';
            }

            let name = this._terminal instanceof PrimitiveTerminal ? 'ScatterMessage' : 'Request';
            return `${name} [${status}]`;
        }
    }

    class GatherMessage {
        constructor(operation, flags, msg) {
            this._operation = operation;
            this._flags     = flags;
            this._msg       = msg;
        }

        get terminal() {
            return this._operation.terminal;
        }

        get operation() {
            return this._operation;
        }

        get flags() {
            return this._flags;
        }

        get message() {
            return this._msg;
        }

        toString() {
            let name = this._terminal instanceof PrimitiveTerminal ? 'GatherMessage' : 'Response';
            return `${name}`;
        }
    }

    class DeafMuteTerminal extends PrimitiveTerminal {
        constructor(session, name, signature) {
            super(session, TM_DEAF_MUTE, name, signature, undefined, undefined);
        }

        static get CompatibleTerminalType() {
            return DeafMuteTerminal;
        }
    }

    class PublishSubscribeTerminal extends SubscribableMixin(PublisherMixin(PublishMessageReceiverMixin(PrimitiveTerminal))) {
        constructor(session, name, signature) {
            super(session, TM_PUBLISH_SUBSCRIBE, name, signature, yogi.MessageType.Publish, yogi.MessageType.Publish);
        }

        static get CompatibleTerminalType() {
            return PublishSubscribeTerminal;
        }
    }

    class CachedPublishSubscribeTerminal extends SubscribableMixin(PublisherMixin(PublishMessageReceiverMixin(PrimitiveTerminal))) {
        constructor(session, name, signature) {
            super(session, TM_CACHED_PUBLISH_SUBSCRIBE, name, signature, yogi.MessageType.Publish, yogi.MessageType.Publish);
        }

        static get CompatibleTerminalType() {
            return CachedPublishSubscribeTerminal;
        }
    }

    class ScatterGatherTerminal extends SubscribableMixin(ScattererMixin(ScatterMessageReceiverMixin(PrimitiveTerminal))) {
        constructor(session, name, signature) {
            super(session, TM_SCATTER_GATHER, name, signature, yogi.MessageType.Scatter, yogi.MessageType.Gather);
        }

        static get CompatibleTerminalType() {
            return ScatterGatherTerminal;
        }

        get Operation() {
            return Operation;
        }

        get ScatterMessage() {
            return ScatterMessage;
        }

        get GatherMessage() {
            return GatherMessage;
        }

        get onScatterMessageReceived() {
            return this._onScatterMessageReceived;
        }

        set onScatterMessageReceived(fn) {
            this._onScatterMessageReceived = fn;
        }

        scatterGather(msg, gatherFn) {
            return this._scatterGather(msg, gatherFn);
        }
    }

    class ProducerTerminal extends SubscribableMixin(PublisherMixin(ConvenienceTerminal)) {
        constructor(session, name, signature) {
            super(session, TM_PRODUCER, name, signature, yogi.MessageType.Publish, undefined);
        }

        static get CompatibleTerminalType() {
            return ConsumerTerminal;
        }
    }

    class ConsumerTerminal extends BinderMixin(PublishMessageReceiverMixin(ConvenienceTerminal)) {
        constructor(session, name, signature) {
            super(session, TM_CONSUMER, name, signature, undefined, yogi.MessageType.Publish);
        }

        static get CompatibleTerminalType() {
            return ProducerTerminal;
        }
    }

    class CachedProducerTerminal extends SubscribableMixin(PublisherMixin(ConvenienceTerminal)) {
        constructor(session, name, signature) {
            super(session, TM_CACHED_PRODUCER, name, signature, yogi.MessageType.Publish, undefined);
        }

        static get CompatibleTerminalType() {
            return CachedConsumerTerminal;
        }
    }

    class CachedConsumerTerminal extends BinderMixin(PublishMessageReceiverMixin(ConvenienceTerminal)) {
        constructor(session, name, signature) {
            super(session, TM_CACHED_CONSUMER, name, signature, undefined, yogi.MessageType.Publish);
        }

        static get CompatibleTerminalType() {
            return CachedProducerTerminal;
        }
    }

    class MasterTerminal extends SubscribableMixin(BinderMixin(PublisherMixin(PublishMessageReceiverMixin(ConvenienceTerminal)))) {
        constructor(session, name, signature) {
            super(session, TM_MASTER, name, signature, yogi.MessageType.Master, yogi.MessageType.Slave);
        }

        static get CompatibleTerminalType() {
            return SlaveTerminal;
        }
    }

    class SlaveTerminal extends SubscribableMixin(BinderMixin(PublisherMixin(PublishMessageReceiverMixin(ConvenienceTerminal)))) {
        constructor(session, name, signature) {
            super(session, TM_SLAVE, name, signature, yogi.MessageType.Slave, yogi.MessageType.Master);
        }

        static get CompatibleTerminalType() {
            return MasterTerminal;
        }
    }

    class CachedMasterTerminal extends SubscribableMixin(BinderMixin(PublisherMixin(PublishMessageReceiverMixin(ConvenienceTerminal)))) {
        constructor(session, name, signature) {
            super(session, TM_CACHED_MASTER, name, signature, yogi.MessageType.Master, yogi.MessageType.Slave);
        }

        static get CompatibleTerminalType() {
            return CachedSlaveTerminal;
        }
    }

    class CachedSlaveTerminal extends SubscribableMixin(BinderMixin(PublisherMixin(PublishMessageReceiverMixin(ConvenienceTerminal)))) {
        constructor(session, name, signature) {
            super(session, TM_CACHED_SLAVE, name, signature, yogi.MessageType.Slave, yogi.MessageType.Master);
        }

        static get CompatibleTerminalType() {
            return CachedMasterTerminal;
        }
    }

    class ServiceTerminal extends BinderMixin(ScatterMessageReceiverMixin(ConvenienceTerminal)) {
        constructor(session, name, signature) {
            super(session, TM_SERVICE, name, signature, yogi.MessageType.Scatter, yogi.MessageType.Gather);
        }

        static get CompatibleTerminalType() {
            return ClientTerminal;
        }

        get Request() {
            return ScatterMessage;
        }

        set onRequestReceived(fn) {
            this._onScatterMessageReceived = fn;
        }
    }

    class ClientTerminal extends SubscribableMixin(ScattererMixin(ConvenienceTerminal)) {
        constructor(session, name, signature) {
            super(session, TM_CLIENT, name, signature, yogi.MessageType.Scatter, yogi.MessageType.Gather);
        }

        static get CompatibleTerminalType() {
            return ServiceTerminal;
        }

        get Operation() {
            return Operation;
        }

        get Response() {
            return GatherMessage;
        }

        request(msg, gatherFn) {
            return this._scatterGather(msg, gatherFn);
        }
    }

    const TM_DEAF_MUTE                = 0;
    const TM_PUBLISH_SUBSCRIBE        = 1;
    const TM_SCATTER_GATHER           = 2;
    const TM_CACHED_PUBLISH_SUBSCRIBE = 3;
    const TM_PRODUCER                 = 4;
    const TM_CONSUMER                 = 5;
    const TM_CACHED_PRODUCER          = 6;
    const TM_CACHED_CONSUMER          = 7;
    const TM_MASTER                   = 8;
    const TM_SLAVE                    = 9;
    const TM_CACHED_MASTER            = 10;
    const TM_CACHED_SLAVE             = 11;
    const TM_SERVICE                  = 12;
    const TM_CLIENT                   = 13;

    const GatherFlags = {
        NoFlags          : 0,
        Finished         : 1 << 0,
        Ignored          : 1 << 1,
        Deaf             : 1 << 2,
        BindingDestroyed : 1 << 3,
        ConnectionLost   : 1 << 4
    };

    window.yogi = window.yogi || {};
    window.yogi.GatherFlags                    = GatherFlags;
    window.yogi.Terminal                       = Terminal;
    window.yogi.PrimitiveTerminal              = PrimitiveTerminal;
    window.yogi.ConvenienceTerminal            = ConvenienceTerminal;
    window.yogi.DeafMuteTerminal               = DeafMuteTerminal;
    window.yogi.PublishSubscribeTerminal       = PublishSubscribeTerminal;
    window.yogi.CachedPublishSubscribeTerminal = CachedPublishSubscribeTerminal;
    window.yogi.ScatterGatherTerminal          = ScatterGatherTerminal;
    window.yogi.ProducerTerminal               = ProducerTerminal;
    window.yogi.ConsumerTerminal               = ConsumerTerminal;
    window.yogi.CachedProducerTerminal         = CachedProducerTerminal;
    window.yogi.CachedConsumerTerminal         = CachedConsumerTerminal;
    window.yogi.MasterTerminal                 = MasterTerminal;
    window.yogi.SlaveTerminal                  = SlaveTerminal;
    window.yogi.CachedMasterTerminal           = CachedMasterTerminal;
    window.yogi.CachedSlaveTerminal            = CachedSlaveTerminal;
    window.yogi.ServiceTerminal                = ServiceTerminal;
    window.yogi.ClientTerminal                 = ClientTerminal;
})();
