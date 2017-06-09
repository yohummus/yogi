(function () {
    class Session {
        constructor(name = '') {
            this._name           = name;
            this._alive          = false;
            this._transactions   = [];
            this._dnsQueries     = new Map();
            this._terminals      = new Map();
            this._bindings       = new Map();
            this._customCommands = new Map();

            this._requestWsUriAndConnect();
        }

        get name() {
            return this._name;
        }

        get alive() {
            return this._alive;
        }

        get comeAlivePromise() {
            return this._comeAlivePromise;
        }

        get diePromise() {
            return this._diePromise;
        }

        close() {
            this._socket.close();
        }

        getVersion() {
            return this._versionPromise || (this._versionPromise = this._request(REQ_VERSION, null, this._parseStringResponse));
        }

        getServerTime() {
            return this._request(REQ_CURRENT_TIME, null, this._parseTimeResponse);
        }

        getClientAddress() {
            return this._request(REQ_CLIENT_ADDRESS, null, this._parseStringResponse);
        }

        _registerConnectionsObserver(observer) {
            if (this._connectionsObserver) {
                throw new Error('A ConnectionsObserver has already been created for this session');
            }

            this._connectionsObserver = observer;
        }

        _registerKnownTerminalsObserver(observer) {
            if (this._knownTerminalsObserver) {
                throw new Error('A KnownTerminalsObserver has already been created for this session');
            }

            this._knownTerminalsObserver = observer;
        }

        _requestWsUriAndConnect() {
            let request = new XMLHttpRequest();

            this._comeAlivePromise = new Promise((resolveComeAlive, rejectComeAlive) => {
                this._diePromise = new Promise((resolveDie) => {
                    request.onreadystatechange = () => {
                        if (request.readyState == XMLHttpRequest.DONE) {
                            if (request.status == 201) {
                                this._connect(request.responseText, resolveComeAlive, rejectComeAlive, resolveDie);
                            }
                            else {
                                this._socket = null;
                                rejectComeAlive(`Requesting WebSocket URI failed with status code ${request.status}.`);
                            }
                        }
                    };
                });
            });

            request.open('GET', window.yogi.hubUri + '/query/ws-uri', true);
            request.send();
        }

        _connect(uri, resolveComeAlive, rejectComeAlive, resolveDie) {
            this._socket = new WebSocket(uri.replace('${HOST}', window.yogi.hubHost));

            this._socket.addEventListener('open', (evt) => {
                this._alive = true;
                resolveComeAlive();
            });

            this._socket.addEventListener('close', (evt) => {
                this._alive = false;
                this._onSocketClosed();
                resolveDie('The session has been closed');
            });

            this._socket.addEventListener('message', (evt) => {
                this._onMessageReceived(evt.data);
            });
        }

        _onSocketClosed() {
            for (let fn of this._transactions) {
                fn('The session has been closed', null);
            }

            this._transactions.length = 0;

            for (let fn of this._dnsQueries) {
                fn('The session has been closed', null);
            }

            this._dnsQueries.clear();
        }

        _onMessageReceived(data) {
            let fileReader = new FileReader();
            fileReader.onload = (evt) => {
                this._handleBatchMessage(evt.target.result);
            }
            fileReader.readAsArrayBuffer(data);
        }

        _handleBatchMessage(data) {
            let offset = 0;
            while (offset < data.byteLength) {
                let msgSize = (new Uint32Array(data.slice(offset, offset + 4)))[0];
                offset += 4;
                let msg = data.slice(offset, offset + msgSize);
                offset += msgSize;
                this._handleMessage(msg);
            }
        }

        _handleMessage(msg) {
            let status = (new Uint8Array(msg))[0];
            switch (status) {
                case RES_OK:
                    this._transactions.shift()(null, msg);
                    break;

                case RES_INVALID_REQUEST:
                    this._transactions.shift()('Invalid request', null);
                    break;

                case RES_API_ERROR:
                    let error = String.fromCharCode.apply(null, new Uint8Array(msg, 1));
                    this._transactions.shift()(`API error: ${error}`, null);
                    break;

                case RES_ALREADY_MONITORING:
                    this._transactions.shift()('Already monitoring the requested resource', null);
                    break;

                case RES_INVALID_TERMINAL_ID:
                    this._transactions.shift()('Invalid terminal ID', null);
                    break;

                case RES_INVALID_BINDING_ID:
                    this._transactions.shift()('Invalid binding ID', null);
                    break;

                case RES_INVALID_OPERATION_ID:
                    this._transactions.shift()('Invalid operation ID', null);
                    break;

                case RES_INVALID_OPERATION_ID:
                    this._transactions.shift()('Invalid command ID', null);
                    break;

                case RES_INVALID_TERMINAL_TYPE:
                    this._transactions.shift()('Invalid terminal type', null);
                    break;

                case ASY_DNS_LOOKUP:
                    this._handleDnsQueryCompletion(msg);
                    break;

                case ASY_CUSTOM_COMMAND_STATE:
                    this._handleCustomCommandState(msg);
                    break;

                case MON_CONNECTION_CHANGED:
                    this._handleConnectionsChangedNotification(msg);
                    break;

                case MON_BINDING_STATE_CHANGED:
                    this._handleBindingStateChangedNotification(msg, this._bindings);
                    break;

                case MON_BUILTIN_BINDING_STATE_CHANGED:
                    this._handleBindingStateChangedNotification(msg, this._terminals);
                    break;

                case MON_SUBSCRIPTION_STATE_CHANGED:
                    this._handleSubscriptionStateChangedNotification(msg);
                    break;

                case MON_PUBLISHED_MESSAGE_RECEIVED:
                    this._handlePublishedMessageReceivedNotification(msg, false);
                    break;

                case MON_CACHED_PUBLISHED_MESSAGE_RECEIVED:
                    this._handlePublishedMessageReceivedNotification(msg, true);
                    break;

                case MON_SCATTERED_MESSAGE_RECEIVED:
                    this._handleScatteredMessageReceivedNotification(msg);
                    break;

                case MON_GATHERED_MESSAGE_RECEIVED:
                    this._handleGatheredMessageReceivedNotification(msg);
                    break;

                case MON_KNOWN_TERMINALS_CHANGED:
                    this._handleKnownTerminalsChangedNotification(msg);
                    break;

                default:
                    console.error('Invalid message header');
                    break;
            }
        }

        _request(requestType, data, responseParserFn) {
            return new Promise((resolve, reject) => {
                let msg;
                if (data) {
                    msg = new Uint8Array(1 + data.byteLength);
                    msg.set(data, 1);
                }
                else {
                    msg = new Uint8Array(1);
                }

                msg[0] = requestType;

                this._comeAlivePromise.then(() => {
                    if (!this._alive) {
                        reject('The session is closed');
                    }
                    else {
                        this._socket.send(msg);
                        this._transactions.push((err, data) => {
                            if (err) {
                                reject(err);
                            }
                            else {
                                if (responseParserFn) {
                                    resolve(responseParserFn(data));
                                }
                                else {
                                    resolve();
                                }
                            }
                        });
                    }
                });
            });
        }

        _requestTestCommand(command) {
            return this._request(REQ_TEST_COMMAND, this._stringToBuffer(command));
        }

        _requestDnsLookup(hostOrIp) {
            return new Promise((resolve, reject) => {
                this._request(REQ_START_DNS_LOOKUP, this._stringToBuffer(hostOrIp), this._parseIdResponse)
                .then((id) => {
                    this._dnsQueries.set(id, (err, data) => {
                        if (err) {
                            reject(err);
                        }
                        else {
                            resolve(data);
                        }
                    });
                })
                .catch(reject);
            });
        }

        _requestStartCustomCommand(customCommand) {
            let strings = [customCommand.command].concat(customCommand.args);
            let dataSize = strings.reduce((n, str) => { return n + str.length + 1; }, 0);
            let buffer = new Uint8Array(dataSize);

            strings.reduce((offset, str) => {
                buffer.set(this._stringToBuffer(str), offset);
                return offset + str.length + 1;
            }, 0);

            return new Promise((resolve, reject) => {
                this._request(REQ_START_CUSTOM_COMMAND, buffer, this._parseIdResponse)
                .then((cmdId) => {
                    this._customCommands.set(cmdId, customCommand);
                    resolve(cmdId);
                })
                .catch(reject);
            });
        }

        _requestTerminateCustomCommand(cmdId) {
            let buffer = new ArrayBuffer(4);
            (new Uint32Array(buffer))[0] = cmdId;

            return this._request(REQ_TERMINATE_CUSTOM_COMMAND, new Uint8Array(buffer));
        }

        _requestWriteCustomCommandStdin(cmdId, str) {
            let data = this._stringToBuffer(str, false);

            let cmdIdData = new ArrayBuffer(4);
            (new Uint32Array(cmdIdData))[0] = cmdId;

            let buffer = new Uint8Array(4 + data.byteLength);
            buffer.set(new Uint8Array(cmdIdData), 0);
            buffer.set(data, 4);

            return this._request(REQ_WRITE_CUSTOM_COMMAND_STDIN, new Uint8Array(buffer));
        }

        _requestGetConnectionFactories() {
            return this._request(REQ_CONNECTION_FACTORIES, null, this._parseConnectionFactoriesResponse);
        }

        _requestGetConnections() {
            return this._request(REQ_CONNECTIONS, null, this._parseConnectionsResponse);
        }

        _requestMonitorConnections() {
            return this._request(REQ_MONITOR_CONNECTIONS, null, this._parseConnectionsResponse);
        }

        _requestCreateTerminal(terminal, type) {
            let signatureData = new ArrayBuffer(4);
            (new Uint32Array(signatureData))[0] = terminal.signature.raw;

            let data = new Uint8Array(1 + 4 + terminal.name.length+1);
            data[0] = type;
            data.set(new Uint8Array(signatureData), 1);
            data.set(this._stringToBuffer(terminal.name), 5)

            let promise = this._request(REQ_CREATE_TERMINAL, data, this._parseIdResponse);
            promise.then((terminalId) => {
                this._terminals.set(terminalId, terminal);
            });

            return promise;
        }

        _requestDestroyTerminal(terminal) {
            let data = new ArrayBuffer(4);
            (new Uint32Array(data))[0] = terminal._id;

            let promise = this._request(REQ_DESTROY_TERMINAL, new Uint8Array(data));
            promise.then(() => {
                this._terminals.delete(terminal._id);
            });

            return promise;
        }

        _requestCreateBinding(binding) {
            let terminalIdData = new ArrayBuffer(4);
            (new Uint32Array(terminalIdData))[0] = binding.terminal._id;

            let data = new Uint8Array(4 + binding.targets.length+1);
            data.set(new Uint8Array(terminalIdData), 0);
            data.set(this._stringToBuffer(binding.targets), 4)

            let promise = this._request(REQ_CREATE_BINDING, data, this._parseIdResponse);
            promise.then((bindingId) => {
                this._bindings.set(bindingId, binding);
            });

            return promise;
        }

        _requestDestroyBinding(binding) {
            let data = new ArrayBuffer(4);
            (new Uint32Array(data))[0] = binding._id;

            let promise = this._request(REQ_DESTROY_BINDING, new Uint8Array(data));
            promise.then(() => {
                this._bindings.delete(binding._id);
            });

            return promise;
        }

        _requestMonitor(requestType, id, responseParserFn) {
            let data = new ArrayBuffer(4);
            (new Uint32Array(data))[0] = id;

            return this._request(requestType, new Uint8Array(data), responseParserFn);
        }

        _requestMonitorBindingState(bindingId) {
            return this._requestMonitor(REQ_MONITOR_BINDING_STATE, bindingId);
        }

        _requestMonitorBuiltinBindingState(terminalId) {
            return this._requestMonitor(REQ_MONITOR_BUILTIN_BINDING_STATE, terminalId);
        }

        _requestMonitorSubscriptionState(terminalId) {
            return this._requestMonitor(REQ_MONITOR_SUBSCRIPTION_STATE, terminalId);
        }

        _requestMonitorReceivedPublishMessages(terminalId) {
            return this._requestMonitor(REQ_MONITOR_RECEIVED_PUBLISH_MESSAGES, terminalId, this._parsePotentiallyCachedMessageResponse);
        }

        _requestMonitorReceivedScatterMessages(terminalId) {
            return this._requestMonitor(REQ_MONITOR_RECEIVED_SCATTER_MESSAGES, terminalId);
        }

        _requestPublish(terminalId, data) {
            let terminalIdData = new ArrayBuffer(4);
            (new Uint32Array(terminalIdData))[0] = terminalId;

            let dataAsBuffer = new Uint8Array(data);

            let buffer = new Uint8Array(4 + dataAsBuffer.byteLength);
            buffer.set(new Uint8Array(terminalIdData), 0);
            buffer.set(dataAsBuffer, 4);

            return this._request(REQ_PUBLISH_MESSAGE, buffer);
        }

        _requestReactToScatteredMessage(terminalId, operationId, data, ignore) {
            let idsData = new ArrayBuffer(8);
            let idsDataAsInt = new Uint32Array(idsData)
            idsDataAsInt[0] = terminalId;
            idsDataAsInt[1] = operationId;

            let dataAsBuffer = new Uint8Array(data);

            let buffer = new Uint8Array(8 + dataAsBuffer.byteLength);
            buffer.set(new Uint8Array(idsData), 0);
            buffer.set(dataAsBuffer, 8);

            return this._request(ignore ? REQ_IGNORE_TO_SCATTERED_MESSAGE : REQ_RESPOND_TO_SCATTERED_MESSAGE, buffer);
        }

        _requestScatterGather(terminal, data) {
            let terminalIdData = new ArrayBuffer(4);
            (new Uint32Array(terminalIdData))[0] = terminal._id;

            let dataAsBuffer = new Uint8Array(data);

            let buffer = new Uint8Array(4 + dataAsBuffer.byteLength);
            buffer.set(new Uint8Array(terminalIdData), 0);
            buffer.set(dataAsBuffer, 4);

            return new Promise((resolve, reject) => {
                this._request(REQ_SCATTER_GATHER, buffer, this._parseIdResponse)
                .then((opId) => {
                    resolve(new terminal.Operation(terminal, opId));
                })
                .catch(reject)
            });
        }

        _requestKnownTerminals() {
            return this._request(REQ_KNOWN_TERMINALS, null, this._parseKnownTerminalsResponse);
        }

        _requestMonitorKnownTerminals() {
            return this._request(REQ_MONITOR_KNOWN_TERMINALS, null, this._parseKnownTerminalsResponse);
        }

        _requestKnownTerminalsSubtree(path) {
            let pathData = this._stringToBuffer(path);
            let buffer = new Uint8Array(1 + pathData.byteLength);
            buffer.set(pathData, 1);

            let absolute = path.length > 0 && path[0] === '/';
            buffer[0] = absolute ? 1 : 0;

            return this._request(REQ_KNOWN_TERMINALS_SUBTREE, buffer, this._parseKnownTerminalsSubtreeResponse);
        }

        _requestFindKnownTerminals(nameSubstr, caseSensitive) {
            let data = new Uint8Array(1 + nameSubstr.length+1);
            data[0] = caseSensitive ? 1 : 0;
            data.set(this._stringToBuffer(nameSubstr), 1);
            return this._request(REQ_FIND_KNOWN_TERMINALS, data, this._parseKnownTerminalsResponse);
        }

        _handleDnsQueryCompletion(msg) {
            let id = (new Int32Array(msg.slice(1, 5)))[0]
            let str = String.fromCharCode.apply(null, new Uint8Array(msg, 5));
            let obj = JSON.parse(str);

            let fn = this._dnsQueries.get(id);
            this._dnsQueries.delete(id);

            if (obj.error) {
                fn(obj.error, null);
            }
            else {
                fn(null, {
                    addresses: obj.addresses,
                    hostname: obj.hostname
                });
            }
        }

        _handleCustomCommandState(msg) {
            let cmdId = (new Uint32Array(msg.slice(1, 5)))[0];
            let running = (new Uint8Array(msg))[5] != 0;
            let exitCode = (new Int32Array(msg.slice(6, 10)))[0];
            let outSize = (new Int32Array(msg.slice(10, 14)))[0];
            let out = String.fromCharCode.apply(null, (new Uint8Array(msg)).slice(14, 14 + outSize));
            let errSize = (new Int32Array(msg.slice(14 + outSize, 18 + outSize)))[0];
            let err = String.fromCharCode.apply(null, (new Uint8Array(msg)).slice(18 + outSize, 18 + outSize + errSize));
            let error = String.fromCharCode.apply(null, (new Uint8Array(msg)).slice(18 + outSize + errSize, msg.byteLength - 1));

            let customCommand = this._customCommands.get(cmdId);

            // TODO: customCommand is occasionally undefined although is should always be defined.
            //       The corresponding test passes but an error is logged in the browser.
            if (!customCommand) {
                return;
            }

            customCommand._updateState(running, exitCode, out, err, error);

            if (!running) {
                this._customCommands.delete(cmdId);
            }
        }

        _handleConnectionsChangedNotification(msg) {
            if (this._connectionsObserver) {
                let connection = this._parseConnectionsResponse(msg)[0];
                this._connectionsObserver._notifyChange(connection);
            }
        }

        _handleBindingStateChangedNotification(msg, bindersMap) {
            let id = (new Int32Array(msg.slice(1, 5)))[0];
            let established = (new Uint8Array(msg))[5] != 0;
            let binder = bindersMap.get(id);
            if (binder) {
                binder._updateBindingState(established);
            }
        }

        _handleSubscriptionStateChangedNotification(msg) {
            let id = (new Int32Array(msg.slice(1, 5)))[0];
            let subscribed = (new Uint8Array(msg))[5] != 0;
            let subscribable = this._terminals.get(id);
            if (subscribable) {
                subscribable._updateSubscriptionState(subscribed);
            }
        }

        _handlePublishedMessageReceivedNotification(msg, terminalHasCache) {
            let id = (new Int32Array(msg.slice(1, 5)))[0];
            let terminal = this._terminals.get(id);
            if (terminal) {
                if (terminalHasCache) {
                    let cached = (new Uint8Array(msg.slice(5, 6)))[0] != 0;
                    terminal._notifyReceivedMessage(new Uint8Array(msg, 6), cached);
                }
                else {
                    terminal._notifyReceivedMessage(new Uint8Array(msg, 5));
                }
            }
        }

        _handleScatteredMessageReceivedNotification(msg) {
            let id = (new Int32Array(msg.slice(1, 5)))[0];
            let opId = (new Int32Array(msg.slice(5, 9)))[0];
            let terminal = this._terminals.get(id);
            if (terminal) {
                terminal._notifyReceivedScatterMessage(opId, new Uint8Array(msg, 9));
            }
        }

        _handleGatheredMessageReceivedNotification(msg) {
            let id = (new Int32Array(msg.slice(1, 5)))[0];
            let opId = (new Int32Array(msg.slice(5, 9)))[0];
            let flags = (new Uint8Array(msg.slice(9, 10)))[0];
            let terminal = this._terminals.get(id);
            if (terminal) {
                terminal._notifyReceivedGatherMessage(opId, new Uint8Array(msg, 10), flags);
            }
        }

        _handleKnownTerminalsChangedNotification(msg) {
            let info = {
                added    : (new Uint8Array(msg))[1] === 1,
                terminal : this._parseKnownTerminalsResponse(msg, 2)[0]
            };

            if (this._knownTerminalsObserver) {
                this._knownTerminalsObserver._notifyChange(info);
            }
        }

        _parseStringResponse(response) {
            return String.fromCharCode.apply(null, new Uint8Array(response, 1));
        }

        _parsePotentiallyCachedMessageResponse(response) {
            if (response.byteLength > 1 && new Uint8Array(response.slice(1, 2))[0] === 1) {
                return new Uint8Array(response, 2);
            }
            else {
                return null;
            }
        }

        _parseTimeResponse(response) {
            let time = new Date();
            time.setTime((new Uint32Array(response.slice(1, 5)))[0] * 1000);
            return time;
        }

        _parseIdResponse(response) {
            return (new Int32Array(response.slice(1)))[0];
        }

        _parseConnectionFactoriesResponse(response) {
            let dataAsByteArray = new Uint8Array(response);
            let factories = {
                tcpServers: [],
                tcpClients: []
            };

            let offset = 1;
            while (offset < dataAsByteArray.length) {
                let isTcpServer = dataAsByteArray[offset] === 1;
                offset += 1;

                let factory = {
                    id: dataAsByteArray[offset],
                    port: (new Uint32Array(response.slice(offset + 1, offset + 5)))[0]
                };
                offset += 5;

                let addressOrHostEnd = dataAsByteArray.indexOf(0, offset);
                let addressOrHost = String.fromCharCode.apply(null, dataAsByteArray.subarray(offset, addressOrHostEnd));
                offset = addressOrHostEnd + 1;

                if (isTcpServer) {
                    factory.address = addressOrHost;
                    factories.tcpServers.push(factory);
                }
                else {
                    factory.host = addressOrHost;
                    factories.tcpClients.push(factory);
                }
            }

            return factories;
        }

        _parseConnectionsResponse(response) {
            let dataAsByteArray = new Uint8Array(response);
            let connections = [];

            let offset = 1;
            while (offset < dataAsByteArray.length) {
                let factoryId = dataAsByteArray[offset];
                offset += 1;

                let connected = dataAsByteArray[offset] ? true : false;
                offset += 1;

                let descriptionEnd = dataAsByteArray.indexOf(0, offset);
                let description = String.fromCharCode.apply(null, dataAsByteArray.subarray(offset, descriptionEnd));
                offset = descriptionEnd + 1;

                let remoteVersionEnd = dataAsByteArray.indexOf(0, offset);
                let remoteVersion = String.fromCharCode.apply(null, dataAsByteArray.subarray(offset, remoteVersionEnd));
                offset = remoteVersionEnd + 1;

                let stateChangedTime = new Date();
                stateChangedTime.setTime((new Uint32Array(response.slice(offset, offset + 4)))[0] * 1000);
                offset += 4;

                connections.push({
                    factoryId       : factoryId,
                    connected       : connected,
                    description     : description,
                    remoteVersion   : remoteVersion,
                    stateChangeTime : stateChangedTime
                });
            }

            return connections;
        }

        _parseKnownTerminalsResponse(response, offset = 1) {
            let dataAsByteArray = new Uint8Array(response);
            let terminals = [];

            while (offset < dataAsByteArray.length) {
                let typeNumber = dataAsByteArray[offset];
                offset += 1;

                let signature = new window.yogi.Signature((new Uint32Array(response.slice(offset, offset + 4)))[0]);
                offset += 4;

                let nameEnd = dataAsByteArray.indexOf(0, offset);
                let name = String.fromCharCode.apply(null, dataAsByteArray.subarray(offset, nameEnd));
                offset = nameEnd + 1;

                terminals.push({
                    type      : window.yogi.Terminal._terminalTypeToTerminalClass(typeNumber),
                    signature : signature,
                    name      : name
                });
            }

            return terminals;
        }

        _parseKnownTerminalsSubtreeResponse(response) {
            let dataAsByteArray = new Uint8Array(response);
            let children = [];

            let offset = 1;
            while (offset < dataAsByteArray.length) {
                let nameEnd = dataAsByteArray.indexOf(0, offset);
                let name = String.fromCharCode.apply(null, dataAsByteArray.subarray(offset, nameEnd));
                offset = nameEnd + 1;

                let child = {
                    name      : name,
                    terminals : []
                };

                while (dataAsByteArray[offset] != 0xFF) {
                    let typeNumber = dataAsByteArray[offset];
                    offset += 1;

                    let signature = new window.yogi.Signature(new Uint32Array(response.slice(offset, offset + 4))[0]);
                    offset += 4;

                    child.terminals.push({
                        type      : window.yogi.Terminal._terminalTypeToTerminalClass(typeNumber),
                        signature : signature
                    });
                }

                offset += 1; // skip 0xFF

                children.push(child);
            }

            return children;
        }

        _stringToBuffer(str, trailingZero = true)
        {
            let data = new Uint8Array(str.length + (trailingZero ? 1 : 0));
            for (let i = 0; i < str.length; i++) {
                data[i] = str.charCodeAt(i);
            }

            if (trailingZero) {
                data[data.length - 1] = 0;
            }

            return data;
        }
    }

    const REQ_VERSION                           = 0;
    const REQ_CURRENT_TIME                      = 1;
    const REQ_TEST_COMMAND                      = 2;
    const REQ_KNOWN_TERMINALS                   = 3;
    const REQ_KNOWN_TERMINALS_SUBTREE           = 4;
    const REQ_FIND_KNOWN_TERMINALS              = 5;
    const REQ_MONITOR_KNOWN_TERMINALS           = 6;
    const REQ_CONNECTION_FACTORIES              = 7;
    const REQ_CONNECTIONS                       = 8;
    const REQ_MONITOR_CONNECTIONS               = 9;
    const REQ_CLIENT_ADDRESS                    = 10;
    const REQ_START_DNS_LOOKUP                  = 11;
    const REQ_CREATE_TERMINAL                   = 12;
    const REQ_DESTROY_TERMINAL                  = 13;
    const REQ_CREATE_BINDING                    = 14;
    const REQ_DESTROY_BINDING                   = 15;
    const REQ_MONITOR_BINDING_STATE             = 16;
    const REQ_MONITOR_BUILTIN_BINDING_STATE     = 17;
    const REQ_MONITOR_SUBSCRIPTION_STATE        = 18;
    const REQ_PUBLISH_MESSAGE                   = 19;
    const REQ_MONITOR_RECEIVED_PUBLISH_MESSAGES = 20;
    const REQ_SCATTER_GATHER                    = 21;
    const REQ_MONITOR_RECEIVED_SCATTER_MESSAGES = 22;
    const REQ_RESPOND_TO_SCATTERED_MESSAGE      = 23;
    const REQ_IGNORE_TO_SCATTERED_MESSAGE       = 24;
    const REQ_START_CUSTOM_COMMAND              = 25;
    const REQ_TERMINATE_CUSTOM_COMMAND          = 26;
    const REQ_WRITE_CUSTOM_COMMAND_STDIN        = 27;

    const RES_OK                                = 0;
    const RES_INTERNAL_SERVER_ERROR             = 1;
    const RES_INVALID_REQUEST                   = 2;
    const RES_API_ERROR                         = 3;
    const RES_ALREADY_MONITORING                = 4;
    const RES_INVALID_TERMINAL_ID               = 5;
    const RES_INVALID_BINDING_ID                = 6;
    const RES_INVALID_OPERATION_ID              = 7;
    const RES_INVALID_COMMAND_ID                = 8;
    const RES_INVALID_TERMINAL_TYPE             = 9;

    const ASY_DNS_LOOKUP                        = 10;
    const ASY_CUSTOM_COMMAND_STATE              = 11;

    const MON_CONNECTION_CHANGED                = 12;
    const MON_KNOWN_TERMINALS_CHANGED           = 13;
    const MON_BINDING_STATE_CHANGED             = 14;
    const MON_BUILTIN_BINDING_STATE_CHANGED     = 15;
    const MON_SUBSCRIPTION_STATE_CHANGED        = 16;
    const MON_PUBLISHED_MESSAGE_RECEIVED        = 17;
    const MON_CACHED_PUBLISHED_MESSAGE_RECEIVED = 18;
    const MON_SCATTERED_MESSAGE_RECEIVED        = 19;
    const MON_GATHERED_MESSAGE_RECEIVED         = 20;

    window.yogi = window.yogi || {};
    window.yogi.Session = Session;
})();
