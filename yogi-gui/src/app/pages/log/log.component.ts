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
  timestampToString,
} from '../../helpers/helpers.barrel';

interface LogEntry {
  processPath: string;
  timestamp: string;
  rawTimestamp: number;
  severity: string;
  component: string;
  message: string;
  description: string;
}

const LOG_LIMIT = 10000;

@Component({
  selector: 'log-page',
  templateUrl: 'log.component.html',
  styleUrls: ['log.component.css']
})
export class LogPageComponent implements OnInit, OnDestroy {
  public static pageTitle = 'Log';

  private entries: LogEntry[] = [];
  private knownTerminalsChangedHandler: KnownTerminalsChangedHandler;
  private terminals = new Map<string, yogi.ConsumerTerminal>();
  private boundTerminals = 0;
  private terminalsTagText: string;
  private terminalsTagTitle: string;

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

    this.yogiService.knownTerminalsObserver.find('/Log', true).then((terminals) => {
      terminals.forEach((terminal) => this.onNewTerminal(terminal));
    });

    this.updateTerminalsTag();
  }

  ngOnDestroy() {
    this.yogiService.removeKnownTerminalsChangedHandler(this.knownTerminalsChangedHandler);
    this.terminals.forEach((terminal) => terminal.destroy());
  }

  onNewTerminal(info: yogi.TerminalInfo) {
    if (!info.name.endsWith('/Log')) {
      return;
    }

    if (info.type !== yogi.ProducerTerminal || info.signature.raw !== 0x9cd) {
      return;
    }

    if (this.terminals.has(info.name)) {
      return;
    }

    let terminal = new yogi.ConsumerTerminal(this.yogiService.session, info.name, info.signature);
    terminal.onMessageReceived = (msg) => this.onLogMessageReceived(terminal.name, msg);
    terminal.onBindingStateChanged = (established) => {
      this.boundTerminals += established ? 1 : -1;
      this.updateTerminalsTag();
    };
    this.terminals.set(info.name, terminal);
  }

  onLogMessageReceived(terminalName: string, msg: yogi.Message) {
    let timestamp;
    let message;
    let info;
    let description;

    try {
      timestamp = timestampToString(msg.timestamp, true);
      message = msg.value.first;
      info = JSON.parse(msg.value.second);
      description = this.makeDescription(message, timestamp, info);
    }
    catch (err) {
      this.errorDisplayService.show(
        `Parsing log message information failed`,
        `Terminal: ${terminalName}\nMessage: ${msg.value.first}\nInformation: ${msg.value.second}`
          + `\nParse error: ${err}`
        );
    }

    this.addEntry({
      severity     : info.severity,
      component    : info.component,
      rawTimestamp : msg.timestamp,
      processPath  : terminalName.substr(0, terminalName.indexOf('/Process/Log')) || terminalName,
      timestamp,
      message,
      description
    });
  }

  makeDescription(message: string, timestamp: string, info: any): string {
    let lines = [
      `Timestamp: ${timestamp}`,
      `Severity: ${info.severity}`,
      `Component: ${info.component}`
    ];

    for (let key in info) {
      if (key !== 'timestamp' && key !== 'severity' && key !== 'component') {
        lines.push(`${key[0].toUpperCase()}${key.substr(1)}: ${info[key]}`);
      }
    }

    lines.push('----------');
    lines.push(message);

    return lines.join('\n');
  }

  addEntry(entry: LogEntry) {
    this.entries.unshift(entry);

    if (this.entries.length > LOG_LIMIT) {
      this.entries.pop();
    }
  }

  onDownloadButtonClicked(event: MouseEvent) {
    event.preventDefault();

    let content = this.entries.reduceRight((cont, entry) => {
      let line = timestampToString(entry.rawTimestamp, true);

      switch (entry.severity) {
        case 'TRACE':   line += ' TRC '; break;
        case 'DEBUG':   line += ' DBG '; break;
        case 'INFO':    line += ' IFO '; break;
        case 'WARNING': line += ' WRN '; break;
        case 'ERROR':   line += ' ERR '; break;
        case 'FATAL':   line += ' FAT '; break;
        default:        line += ' ??? '; break;
      }

      line += entry.component + ': ';
      line += entry.message;

      return cont + line + '\n';
    }, '');
    yogi.downloadTextFile('log.txt', content);
  }

  updateTerminalsTag() {
    if (this.boundTerminals === 1) {
      this.terminalsTagText = '1 Terminal';
      this.terminalsTagTitle = 'There is currently 1 Terminal being monitored for log messages.';
    }
    else {
      this.terminalsTagText = `${this.boundTerminals} Terminals`;
      this.terminalsTagTitle = `There are currently ${this.boundTerminals} Terminals being `
                             + 'monitored for log messages.';
    }
  }
}
