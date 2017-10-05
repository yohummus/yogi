import {
  NgModule,
} from '@angular/core';

import {
  CommonModule,
} from '@angular/common';

import {
  FormsModule,
} from '@angular/forms';

import {
  LoginModalComponent,
} from './login-modal/login-modal.component';

import {
  ConfirmModalComponent,
} from './confirm-modal/confirm-modal.component';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
  ],
  exports: [
    LoginModalComponent,
    ConfirmModalComponent,
  ],
  declarations: [
    LoginModalComponent,
    ConfirmModalComponent,
  ],
  providers: [,
  ],
})
export class ModalsModule {}

export {
  LoginModalComponent,
  LoginDetails,
} from './login-modal/login-modal.component';

export {
  ConfirmModalComponent,
} from './confirm-modal/confirm-modal.component';
