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
    .then(() => {
      this.yogiService.session.getVersion()
      .then((version) => {
        this.tooltip = `YOGI version ${version}`;
      });
    })
    .catch((err) => {
      this.tooltip = `Could not connect to YOGI-Hub:\n${err}\nRefresh this page when the error is fixed.`;
    });
  }
}
