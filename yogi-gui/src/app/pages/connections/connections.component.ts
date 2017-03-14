import {
  Component,
  OnInit,
  OnDestroy,
} from '@angular/core';

import {
  YogiService,
  ConnectionsChangedHandler,
  ErrorDisplayService,
} from '../../core/core.module';

import {
  dateToString,
  secondsToDurationString,
} from '../../helpers/helpers.barrel';

interface ConnectionInfo extends yogi.ConnectionInfo {
  remoteHost: string;
  remotePort: number;
  timeString: string;
  durationString: string;
}

interface FactoryInfo {
  id: number;
  address: string;
  port: number;
  connections: ConnectionInfo[];
}

@Component({
  selector: 'ce-connections-page',
  templateUrl: 'connections.component.html',
  styleUrls: ['connections.component.css']
})
export class ConnectionsPageComponent implements OnInit {
  public static pageTitle = 'Connections';

  private connectionsChangedHandler: ConnectionsChangedHandler;
  private tcpServers: FactoryInfo[] = [];
  private tcpClients: FactoryInfo[] = [];
  private timeUpdateTimer: NodeJS.Timer;
  private serverClientTimeDeltaInMs = 0;

  constructor(private YogiService: YogiService,
              private errorDisplayService: ErrorDisplayService) {
  }

  ngOnInit() {
    this.YogiService.session.getServerTime()
    .then((time) => {
      this.serverClientTimeDeltaInMs = time.getTime() - new Date().getTime();
    });

    this.YogiService.connectionsObserver.getFactories()
    .then((factories) => {
      this.tcpServers = factories.tcpServers.map((server) => {
        return {
          id          : server.id,
          address     : server.address,
          port        : server.port,
          connections : []
        };
      });

      this.tcpClients = factories.tcpClients.map((client) => {
        return {
          id          : client.id,
          address     : client.host,
          port        : client.port,
          connections : []
        };
      });
    });

    this.YogiService.connectionsObserver.getAll()
    .then((connections) => {
      for (let connection of connections) {
        this.onConnectionUpdated(connection);
      }
    });

    this.YogiService.addConnectionsChangedHandler((connection) => {
      this.onConnectionUpdated(connection);
    });

    this.timeUpdateTimer = setInterval(() => this.updateTimesAndDurations(), 1000);
  }

  ngOnDestroy() {
    clearInterval(this.timeUpdateTimer);
    this.YogiService.removeConnectionsChangedHandler(this.connectionsChangedHandler);
  }

  get allFactories(): FactoryInfo[] {
    return this.tcpServers.concat(this.tcpClients);
  }

  onConnectionUpdated(info: yogi.ConnectionInfo) {
    let factory = this.allFactories.find((fact) => fact.id === info.factoryId);
    let conns = factory.connections;

    let idx = conns.findIndex((con) => con.description === info.description);
    if (idx !== -1) {
      conns[idx].connected       = info.connected;
      conns[idx].stateChangeTime = info.stateChangeTime;
      conns[idx].remoteVersion   = info.remoteVersion;
      conns[idx].timeString      = dateToString(info.stateChangeTime);
    }
    else {
      let conn = {
        factoryId       : info.factoryId,
        connected       : info.connected,
        description     : info.description,
        remoteVersion   : info.remoteVersion,
        stateChangeTime : info.stateChangeTime,
        remoteHost      : '',
        remotePort      : 0,
        timeString      : '',
        durationString  : ''
      };

      if (info.description.indexOf(':') === -1) {
        conn.remoteHost = factory.address;
        conn.remotePort = factory.port;
      }
      else {
        conn.remoteHost = info.description.substr(0, info.description.lastIndexOf(':'));
        conn.remotePort = Number.parseInt(info.description.substr(
          info.description.lastIndexOf(':') + 1));
        this.YogiService.dnsService.lookup(conn.remoteHost)
        .then((ifo) => {
          conn.remoteHost = ifo.hostname;
        });
      }

      conns.push(conn);
    }

    this.updateTimesAndDurations();
    this.sortConnections();
  }

  updateTimesAndDurations() {
    for (let factory of this.allFactories) {
      for (let conn of factory.connections) {
        conn.timeString = dateToString(conn.stateChangeTime);

        let now = new Date();
        let delta = now.getTime() - conn.stateChangeTime.getTime();
        let duration = delta + this.serverClientTimeDeltaInMs;
        conn.durationString = secondsToDurationString(duration / 1000);
      }
    }
  }

  sortConnections() {
    for (let factory of this.allFactories) {
      factory.connections.sort((a, b) => {
        if (a.connected === b.connected) {
          if (a.stateChangeTime === b.stateChangeTime) {
            return 0;
          }
          else if (a.stateChangeTime > b.stateChangeTime) {
            return -1;
          }
          else {
            return 1;
          }
        }
        else if (a.connected) {
          return -1;
        }
        else {
          return 1;
        }
      });
    }
  }
}
