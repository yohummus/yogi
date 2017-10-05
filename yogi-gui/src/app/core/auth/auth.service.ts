import {
  Injectable,
  NgZone,
} from '@angular/core';

import {
  YogiService,
} from '../yogi/yogi.service';

import {
  ErrorDisplayService,
} from '../nav/error-display/error-display.service';

export enum LoginState {
  LOGGED_OUT,
  LOGGING_IN,
  LOGGED_IN
}

@Injectable()
export class AuthService {
  private _loginState = LoginState.LOGGED_OUT;

  constructor(private _yogiService: YogiService, private _errorDisplayService: ErrorDisplayService, private _ngZone: NgZone) {
    let username = window.localStorage.getItem('username');
    let password = window.localStorage.getItem('password');
    if (username && password) {
      this._yogiService.connectedPromise.then(() => {
        this._logInHashed(username, password, true)
        .catch(err => {});
      });
    }
  }

  get loginState(): LoginState {
    return this._loginState;
  }

  get username(): string | null {
    return this._yogiService.session.username;
  }

  get webSessionName(): string | null {
    return this._yogiService.session.webSessionName;
  }

  logIn(username: string, password: string, remember: boolean): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      this._hash(password)
      .then((hashedPassword) => {
        this._logInHashed(username, hashedPassword, remember)
        .then(resolve)
        .catch(reject);
      });
    });
  }

  logOut(): void {
    window.localStorage.removeItem('username');
    window.localStorage.removeItem('password');
    window.location.reload();
  }

  private _hash(str: string): Promise<string> {
    let data = new Uint8Array(str.length);
    for (let i = 0; i < str.length; i++) {
        data[i] = str.charCodeAt(i);
    }

    return new Promise((resolve, reject) => {
      window.crypto.subtle.digest({ name: 'SHA-1' }, data)
      .then((hash) => {
        let hashAsHexString = Array.prototype.map.call(new Uint8Array(hash), (x: number) => ('00' + x.toString(16)).slice(-2)).join('');
        resolve(hashAsHexString);
      });
    });
  }

  private _logInHashed(username: string, hashedPassword: string, remember: boolean): Promise<void> {
    if (this._yogiService.session.loggedIn) {
      throw new Error('Already logged in');
    }

    if (!remember) {
      window.localStorage.removeItem('username');
      window.localStorage.removeItem('password');
    }

    return new Promise<void>((resolve, reject) => {
      this._ngZone.run(() => {
        this._yogiService.session.logIn(username, hashedPassword)
        .then(() => {
          this._loginState = LoginState.LOGGED_IN;

          if (remember) {
            window.localStorage.setItem('username', username);
            window.localStorage.setItem('password', hashedPassword);
          }

          resolve();
        })
        .catch((err) => {
          this._loginState = LoginState.LOGGED_OUT;
          this._errorDisplayService.show('Login failed', `Failed to log in as ${username}.\n${err}`);
          reject(err);
        });

        this._loginState = LoginState.LOGGING_IN;
      });
    });
  }
}
