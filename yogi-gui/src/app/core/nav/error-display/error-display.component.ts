import {
  Component,
  trigger,
  state,
  style,
  transition,
  animate,
} from '@angular/core';

import {
  ErrorDisplayService,
  ErrorInfo,
} from './error-display.service';

@Component({
  selector: 'ce-error-display',
  templateUrl: 'error-display.component.html',
  styleUrls: ['error-display.component.css'],
  animations: [
    trigger('displayState', [
      state('true' , style({ transform: 'scale(1)', visibility: 'visible' })),
      state('false', style({ transform: 'scale(0)', visibility: 'hidden' })),
      transition('1 => 0', animate('500ms ease-out'))
    ])
  ]
})
export class ErrorDisplayComponent {
  private hovering = false;

  constructor(private errorDisplayService: ErrorDisplayService) {
  }

  get active() {
    return this.hovering || !!this.errorDisplayService.activeError;
  }

  get title() {
    let err = this.errorDisplayService.lastActiveError;
    return err ? (err as ErrorInfo).title : null;
  }

  get description() {
    let err = this.errorDisplayService.lastActiveError;
    return err ? (err as ErrorInfo).description : null;
  }
}
