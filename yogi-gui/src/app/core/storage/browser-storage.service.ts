import {
  Injectable,
} from '@angular/core';

@Injectable()
export class BrowserStorageService {
  store(variable: string, data: any): void {
    let str = typeof data === 'string' ? data : JSON.stringify(data);
    window.localStorage.setItem(variable, str);
  }

  read(variable: string): any | null {
    let data = window.localStorage.getItem(variable);

    try {
      return JSON.parse(data);
    }
    catch (err) {
      return data;
    }
  }

  erase(variable: string): void {
    window.localStorage.removeItem(variable);
  }
}
