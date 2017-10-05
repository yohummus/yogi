import {
  Component,
  ViewChild,
} from '@angular/core';

import {
  AuthService,
  LoginState,
} from '../../auth/auth.service';

import {
  LoginModalComponent,
  LoginDetails,
  ConfirmModalComponent,
} from '../../modals/modals.module';

@Component({
  selector: 'ce-account-display',
  templateUrl: 'account-display.component.html',
  styleUrls: ['account-display.component.css']
})
export class AccountDisplayComponent {
  @ViewChild('loginModal') loginModal: LoginModalComponent;
  @ViewChild('logoutConfirmModal') logoutConfirmModal: ConfirmModalComponent;

  private LoginState = LoginState;

  constructor(private authService: AuthService) {
  }

  onClicked() {
    if (this.authService.loginState === LoginState.LOGGED_OUT) {
      this.loginModal.open()
      .then((details) => this.onLoginDetailsSubmitted(details))
      .catch(() => {});
    }
    else if (this.authService.loginState === LoginState.LOGGED_IN) {
      this.logoutConfirmModal.open()
      .then(() => this.onLogOutConfirmed())
      .catch(() => {});
    }
  }

  onLoginDetailsSubmitted(details: LoginDetails) {
    this.authService.logIn(details.username, details.password, details.remember)
    .catch(err => {})
  }

  onLogOutConfirmed() {
    this.logoutConfirmModal.open();
    this.authService.logOut();
  }
}
