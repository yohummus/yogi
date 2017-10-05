import {
  Injectable,
} from '@angular/core';

import {
  YogiService,
} from '../yogi/yogi.service';

import {
  ErrorDisplayService,
} from '../nav/error-display/error-display.service';

@Injectable()
export class DnsService {
  private _service: yogi.DnsService;

  constructor(private _yogiService: YogiService, private _errorDisplayService: ErrorDisplayService) {
    this._yogiService.connectedPromise
    .then(() => {
      this._service = new yogi.DnsService(this._yogiService.session);
    });
  }

  lookup(hostOrIp: string): Promise<yogi.DnsLookupInfo> {
    return this._service.lookup(hostOrIp);
  }
}
