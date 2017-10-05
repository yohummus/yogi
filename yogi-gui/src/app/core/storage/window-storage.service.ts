import {
  Injectable,
} from '@angular/core';

@Injectable()
export class WindowStorageService {
  private _store: any;

  constructor() {
    let win = (window as any);
    if (typeof win.windowStorage === 'undefined') {
      win.windowStorage = {};
    }

    this._store = win.windowStorage;
  }

  store(variable: string, data: any): void {
    this._store[variable] = data;
  }

  read(variable: string): any | null {
    let data = this._store[variable];
    return typeof data === 'undefined' ? null : data;
  }

  erase(variable: string): void {
    delete this._store[variable];
  }
}
