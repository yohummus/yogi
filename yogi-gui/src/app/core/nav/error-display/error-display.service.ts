import {
  Injectable,
} from '@angular/core';

const SHOW_DURATION_MS = 10000;

export interface ErrorInfo {
  title: string;
  description: string;
}

@Injectable()
export class ErrorDisplayService {
  private _activeError: ErrorInfo = null;
  private _lastActiveError: ErrorInfo = null;
  private _timerId: NodeJS.Timer = null;

  get activeError(): ErrorInfo | void {
    return this._activeError;
  }

  get lastActiveError(): ErrorInfo | void {
    return this._lastActiveError;
  }

  show(title: string, description: string): void {
    this._activeError = { title, description };
    this._lastActiveError = this._activeError;
    console.error('Error:', this._activeError);

    if (this._timerId) {
      clearInterval(this._timerId);
    }

    this._timerId = setInterval(() => {
      this._activeError = null;
    }, SHOW_DURATION_MS);
  }
}
