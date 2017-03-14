import {
  NgModule,
  Optional,
  SkipSelf,
} from '@angular/core';

import {
  YogiService,
} from './yogi/yogi.service';

import {
  NavModule,
  ErrorDisplayService,
} from './nav/nav.module';

import {
  throwIfAlreadyLoaded,
} from './module-import-guard';

@NgModule({
  imports: [
    NavModule,
  ],
  exports: [
    NavModule,
  ],
  declarations: [
  ],
  providers: [
    YogiService,
  ],
})
export class CoreModule {
  constructor(@Optional() @SkipSelf() parentModule: CoreModule) {
    throwIfAlreadyLoaded(parentModule, 'CoreModule');
  }
}

export {
  ErrorDisplayService,
} from './nav/nav.module';

export {
  YogiService,
  ConnectionsChangedHandler,
  KnownTerminalsChangedHandler,
} from './yogi/yogi.service';
