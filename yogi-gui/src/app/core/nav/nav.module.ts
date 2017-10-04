import {
  NgModule,
} from '@angular/core';

import {
  CommonModule,
} from '@angular/common';

import {
  RouterModule,
} from '@angular/router';

import {
  BrowserAnimationsModule
} from '@angular/platform-browser/animations'

import {
  NavComponent,
} from './nav.component';

import {
  StatusDisplayComponent,
} from './status-display/status-display.component';

import {
  ErrorDisplayService,
} from './error-display/error-display.service';

import {
  ErrorDisplayComponent,
} from './error-display/error-display.component';

import {
  AccountDisplayComponent,
} from './account-display/account-display.component';

@NgModule({
  imports: [
    CommonModule,
    RouterModule,
    BrowserAnimationsModule,
  ],
  exports: [
    NavComponent,
  ],
  declarations: [
    NavComponent,
    StatusDisplayComponent,
    ErrorDisplayComponent,
    AccountDisplayComponent,
  ],
  providers: [
    ErrorDisplayService,
  ],
})
export class NavModule {}

export {
  NavComponent,
} from './nav.component';

export {
  ErrorDisplayService
} from './error-display/error-display.service';
