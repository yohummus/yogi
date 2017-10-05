import {
  Component,
  ViewChild,
  Input,
} from '@angular/core';

import {
  NgbModal,
  ModalDismissReasons,
} from '@ng-bootstrap/ng-bootstrap';

@Component({
  selector: 'ce-confirm-modal',
  templateUrl: 'confirm-modal.component.html',
  styleUrls: ['confirm-modal.component.css']
})
export class ConfirmModalComponent {
  @Input() title: string;

  @ViewChild('content') contentVc: any;

  constructor(private modalService: NgbModal) {
  }

  open(): Promise<void> {
    return new Promise<any>((resolve, reject) => {
      this.modalService.open(this.contentVc).result
      .then(resolve)
      .catch((err) => {
        reject('Canceled');
      });
    });
  }
}
