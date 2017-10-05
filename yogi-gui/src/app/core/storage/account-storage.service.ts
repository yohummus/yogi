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
export class AccountStorageService {
  private _store: yogi.AccountStorage;

  constructor(private _yogiService: YogiService, private _errorDisplayService: ErrorDisplayService) {
    this._yogiService.connectedPromise
    .then(() => {
      this._store = new yogi.AccountStorage(this._yogiService.session);
    });
  }

  store(variable: string, data: any): Promise<void> {
    let str = typeof data === 'string' ? data : JSON.stringify(data);

    let arr = new Uint8Array(str.length);
    for (let i =0; i < str.length; i++) {
      arr[i] = str.charCodeAt(i);
    }

    return new Promise<void>((resolve, reject) => {
      this._store.store(variable, arr.buffer)
      .then(resolve)
      .catch((err) => {
        this._errorDisplayService.show('Storing data failed',
          `Could not store ${arr.byteLength} bytes of data in the account variable '${variable}':`
          + `\n${err}`);
        reject(err);
      });
    });
  }

  read(variable: string): Promise<any> {
    return new Promise<any>((resolve, reject) => {
      this._store.read(variable)
      .then((data) => {
        if (data.byteLength === 0) {
          resolve(null);
        }
        else {
          let str = String.fromCharCode.apply(null, new Uint8Array(data));
          try {
            resolve(JSON.parse(str));
          }
          catch (err) {
            resolve(str);
          }
        }
      })
      .catch((err) => {
        this._errorDisplayService.show('Reading data failed',
          `Could not read account variable '${variable}':\n${err}`);
        reject(err);
      });
    });
  }
}
