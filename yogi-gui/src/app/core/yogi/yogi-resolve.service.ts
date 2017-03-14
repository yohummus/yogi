import {
  Injectable,
} from '@angular/core';

import {
  Router,
  Resolve,
  ActivatedRouteSnapshot,
} from '@angular/router';

import {
  YogiService,
} from './yogi.service';

@Injectable()
export class YogiResolve implements Resolve<void> {
  constructor(private yogiService: YogiService, private router: Router) {
  }

  resolve(route: ActivatedRouteSnapshot): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      this.yogiService.connectedPromise
      .then(() => {
        if (this.yogiService.connected) {
          resolve();
        }
        else {
          reject('Connection to YOGI-Hub lost');
        }
      })
      .catch((err) => {
        console.error(err);
      });
    });
  }
}
