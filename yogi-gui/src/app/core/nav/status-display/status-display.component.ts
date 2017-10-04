import {
  Component,
  OnInit,
} from '@angular/core';

import {
  YogiService,
  ConnectionStatus,
} from '../../yogi/yogi.service';

@Component({
  selector: 'ce-status-display',
  templateUrl: 'status-display.component.html',
  styleUrls: ['status-display.component.css']
})
export class StatusDisplayComponent implements OnInit {
  private ConnectionStatus = ConnectionStatus;
  private tooltip: string;

  constructor(private yogiService: YogiService) {
  }

  ngOnInit() {
    this.tooltip = 'Downloading YOGI lib from the configured YOGI-Hub and creating a YOGI session...';

    this.yogiService.connectedPromise
    .then(this.onConnected.bind(this))
    .catch(this.onConnectFailed.bind(this));

    this.yogiService.disconnectedPromise
    .then(this.onDisconnected.bind(this));
  }

  onConnected() {
    this.yogiService.session.getVersion()
    .then(this.onVersionReceived.bind(this));
  }

  onVersionReceived(version: string) {
    this.tooltip = `Connected to YOGI-Hub.\nYOGI version: ${version}`;
  }

  onConnectFailed(err: string) {
    this.tooltip = `Could not connect to YOGI-Hub:\n${err}\nRefresh this page when the error is fixed.`;
  }

  onDisconnected() {
    this.tooltip = 'Connection to YOGI-Hub lost.';
  }
}
