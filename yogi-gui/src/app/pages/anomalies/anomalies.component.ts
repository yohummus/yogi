import {
  Component,
  OnInit,
  OnDestroy,
} from '@angular/core';

import {
  YogiService,
  ErrorDisplayService,
  KnownTerminalsChangedHandler,
} from '../../core/core.module';

import {
  naturalSort,
  timestampToString,
} from '../../helpers/helpers.barrel';

interface Anomalies {
  processPath: string;
  errors: string[];
  warnings: string[];
}

@Component({
  selector: 'ce-anomalies-page',
  templateUrl: 'anomalies.component.html',
  styleUrls: ['anomalies.component.css']
})
export class AnomaliesPageComponent implements OnInit, OnDestroy {
  public static pageTitle = 'Anomalies';

  private knownTerminalsChangedHandler: KnownTerminalsChangedHandler;
  private terminals = new Map<string, yogi.CachedConsumerTerminal>();
  private anomalies: Anomalies[] = [];
  private boundErrorTerminals = 0;
  private processesTagText: string;
  private processesTagTitle: string;

  constructor(private yogiService: YogiService,
              private errorDisplayService: ErrorDisplayService) {
  }

  ngOnInit() {
    this.knownTerminalsChangedHandler = (info) => {
      if (info.added) {
        this.onNewTerminal(info.terminal);
      }
    };
    this.yogiService.addKnownTerminalsChangedHandler(this.knownTerminalsChangedHandler);

    this.yogiService.knownTerminalsObserver.find('/Process/Errors', true).then((terminals) => {
      terminals.forEach((terminal) => this.onNewTerminal(terminal));
    });

    this.yogiService.knownTerminalsObserver.find('/Process/Warnings', true).then((terminals) => {
      terminals.forEach((terminal) => this.onNewTerminal(terminal));
    });

    this.updateTerminalsTag();
  }

  ngOnDestroy() {
    this.yogiService.removeKnownTerminalsChangedHandler(this.knownTerminalsChangedHandler);
    this.terminals.forEach((terminal) => terminal.destroy());
  }

  onNewTerminal(info: yogi.TerminalInfo) {
    if (!(info.name.endsWith('/Process/Errors') || info.name.endsWith('/Process/Warnings'))
       || info.type !== yogi.CachedProducerTerminal || info.signature.raw !== 0x40d) {
      return;
    }

    if (this.terminals.has(info.name)) {
      return;
    }

    let processPath = info.name.substr(0, info.name.lastIndexOf('/Process/'));
    let isError = info.name.endsWith('/Errors');

    let terminal = new yogi.CachedConsumerTerminal(this.yogiService.session, info.name,
      info.signature);
    terminal.onMessageReceived = (msg) => this.onMessageReceived(processPath, isError, msg);
    terminal.onBindingStateChanged = (established) => {
      if (!established) {
        this.onBindingReleased(processPath, isError);
      }

      if (isError) {
        this.boundErrorTerminals += established ? 1 : -1;
        this.updateTerminalsTag();
      }
    };
    this.terminals.set(info.name, terminal);

    if (!this.anomalies.find((entry) => entry.processPath === processPath)) {
      this.anomalies.push({
        processPath,
        errors: [],
        warnings: []
      });

      this.anomalies.sort((a, b) => naturalSort(a.processPath, b.processPath));
    }
  }

  onMessageReceived(processPath: string, isError: boolean, msg: yogi.Message) {
    let anomalies = this.anomalies.find((entry) => entry.processPath === processPath);
    if (isError) {
      anomalies.errors = msg.value;
    }
    else {
      anomalies.warnings = msg.value;
    }
  }

  onBindingReleased(processPath: string, isError: boolean) {
    let anomalies = this.anomalies.find((entry) => entry.processPath === processPath);
    if (isError) {
      anomalies.errors = [];
    }
    else {
      anomalies.warnings = [];
    }
  }

  updateTerminalsTag() {
    if (this.boundErrorTerminals === 1) {
      this.processesTagText = '1 Process';
      this.processesTagTitle = 'There is currently 1 Process being monitored for anomalies.';
    }
    else {
      this.processesTagText = `${this.boundErrorTerminals} Processes`;
      this.processesTagTitle = `There are currently ${this.boundErrorTerminals} Processes being`
                             + ' monitored for anomalies.';
    }
  }
}
