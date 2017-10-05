import {
  NgModule,
  Optional,
  SkipSelf,
} from '@angular/core';

import {
  YogiService,
} from './yogi/yogi.service';

import {
  AuthService,
} from './auth/auth.service';

import {
  AccountStorageService,
} from './storage/account-storage.service';

import {
  SessionStorageService,
} from './storage/session-storage.service';

import {
  WindowStorageService,
} from './storage/window-storage.service';

import {
  BrowserStorageService,
} from './storage/browser-storage.service';

import {
  NavModule,
  ErrorDisplayService,
} from './nav/nav.module';

import {
  ModalsModule,
} from './modals/modals.module';

import {
  throwIfAlreadyLoaded,
} from './module-import-guard';

@NgModule({
  imports: [
    NavModule,
    ModalsModule,
  ],
  exports: [
    NavModule,
    ModalsModule,
  ],
  declarations: [
  ],
  providers: [
    YogiService,
    AuthService,
    AccountStorageService,
    SessionStorageService,
    WindowStorageService,
    BrowserStorageService,
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

export {
  AuthService,
  LoginState,
} from './auth/auth.service';

export {
  AccountStorageService,
} from './storage/account-storage.service';

export {
  SessionStorageService,
} from './storage/session-storage.service';

export {
  WindowStorageService,
} from './storage/window-storage.service';

export {
  BrowserStorageService,
} from './storage/browser-storage.service';
