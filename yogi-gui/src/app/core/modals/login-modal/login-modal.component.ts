import {
  Component,
  ViewChild,
} from '@angular/core';

import {
  NgbModal,
} from '@ng-bootstrap/ng-bootstrap';

export interface LoginDetails {
  username: string;
  password: string;
  remember: boolean;
}

@Component({
  selector: 'ce-login-modal',
  templateUrl: 'login-modal.component.html',
  styleUrls: ['login-modal.component.css']
})
export class LoginModalComponent {
  @ViewChild('content') contentVc: any;

  private username: string;
  private password: string;
  private remember: boolean = false;

  constructor(private modalService: NgbModal) {
  }

  open(): Promise<LoginDetails> {
    return new Promise<any>((resolve, reject) => {
      this.modalService.open(this.contentVc).result
      .then(() => {
        resolve({
          username: this.username,
          password: this.password,
          remember: this.remember
        });

        this.password = '';
      })
      .catch((err) => {
        reject('Canceled');
        this.password = '';
      });
    });
  }
}
