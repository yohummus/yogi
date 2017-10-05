import {
  Injectable,
} from '@angular/core';

import {
  yogiHubLibUri,
  yogiSessionName
} from '../../../../solution/yogi.config';

export enum ConnectionStatus {
  CONNECTING,
  CONNECTED,
  CONNECTION_LOST,
  CONNECTION_FAILED
}

export type KnownTerminalsChangedHandler = (info: yogi.KnownTerminalChangeInfo) => any;
export type ConnectionsChangedHandler = (info: yogi.ConnectionInfo) => any;

@Injectable()
export class YogiService {
  private _connectedPromise: Promise<void>;
  private _disconnectedPromise: Promise<void>;
  private _connectionStatus = ConnectionStatus.CONNECTING;
  private _session: yogi.Session;
  private _knownTerminalsObserver: yogi.KnownTerminalsObserver;
  private _knownTerminalsChangedHandlers: KnownTerminalsChangedHandler[] = [];
  private _connectionsObserver: yogi.ConnectionsObserver;
  private _connectionsChangedHandlers: ConnectionsChangedHandler[] = [];

  constructor() {
    this._setupPromises();
  }

  get connectedPromise(): Promise<void> {
    return this._connectedPromise;
  }

  get disconnectedPromise(): Promise<void> {
    return this._disconnectedPromise;
  }

  get connectionStatus(): ConnectionStatus {
    return this._connectionStatus;
  }

  get connected(): boolean {
    return this._connectionStatus === ConnectionStatus.CONNECTED;
  }

  get session(): yogi.Session {
    return this._session;
  }

  get knownTerminalsObserver(): yogi.KnownTerminalsObserver {
    return this._knownTerminalsObserver;
  }

  addKnownTerminalsChangedHandler(fn: KnownTerminalsChangedHandler): void {
    this._knownTerminalsChangedHandlers.push(fn);
  }

  removeKnownTerminalsChangedHandler(fn: KnownTerminalsChangedHandler): void {
    let idx = this._knownTerminalsChangedHandlers.indexOf(fn);
    this._knownTerminalsChangedHandlers.splice(idx, 1);
  }

  get connectionsObserver(): yogi.ConnectionsObserver {
    return this._connectionsObserver;
  }

  addConnectionsChangedHandler(fn: ConnectionsChangedHandler): void {
    this._connectionsChangedHandlers.push(fn);
  }

  removeConnectionsChangedHandler(fn: ConnectionsChangedHandler): void {
    let idx = this._connectionsChangedHandlers.indexOf(fn);
    this._connectionsChangedHandlers.splice(idx, 1);
  }

  private _setupPromises() {
    this._disconnectedPromise = new Promise<void>((disconnResolve) => {
      this._connectedPromise = new Promise<void>((connResolve, connReject) => {
        this._loadLib()
        .then(() => {
          this._session = new yogi.Session(yogiSessionName);
          this._session.comeAlivePromise
          .then(() => {
            this._connectionStatus = ConnectionStatus.CONNECTED;
            connResolve();
          })
          .catch((err) => {
            this._connectionStatus = ConnectionStatus.CONNECTION_FAILED;
            connReject(err);
          });

          this._session.diePromise
          .then(() => {
            this._connectionStatus = ConnectionStatus.CONNECTION_LOST;
            console.error('Connection to YOGI-Hub lost');
            disconnResolve();
          });

          this._knownTerminalsObserver = new yogi.KnownTerminalsObserver(this._session);
          this._knownTerminalsObserver.onChanged = (info) => this._onKnownTerminalsChanged(info);

          this._connectionsObserver = new yogi.ConnectionsObserver(this._session);
          this._connectionsObserver.onChanged = (info) => this._onConnectionsChanged(info);
        })
        .catch((err) => {
          this._connectionStatus = ConnectionStatus.CONNECTION_FAILED;
          connReject(err);
        });
      });
    });
  }

  private _loadLib() {
    return new Promise((resolve, reject) => {
      let uri = (yogiHubLibUri + (yogiHubLibUri.endsWith('/') ? '' : '/') + 'yogi-hub-all.min.js')
        .replace('${HOSTNAME}', window.location.hostname)
        .replace('${HOST}', window.location.host)
        .replace('${PORT}', window.location.port)
        .replace('${ORIGIN}', window.location.origin)
        .replace('${PROTOCOL}', window.location.protocol);

      let script = document.createElement('script');
      script.type = 'text/javascript';
      script.src = uri;
      script.onload = resolve;
      script.onerror = () => {
        reject('Could not download ' + uri);
      };

      document.getElementsByTagName('head')[0].appendChild(script);
    });
  }

  private _onKnownTerminalsChanged(info: yogi.KnownTerminalChangeInfo) {
    for (let fn of this._knownTerminalsChangedHandlers) {
      fn(info);
    }
  }

  private _onConnectionsChanged(info: yogi.ConnectionInfo) {
    for (let fn of this._connectionsChangedHandlers) {
      fn(info);
    }
  }
}
