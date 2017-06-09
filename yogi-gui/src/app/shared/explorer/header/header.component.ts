import {
  Component,
  Input,
  Output,
  EventEmitter,
} from '@angular/core';

@Component({
  selector: 'ce-header',
  templateUrl: 'header.component.html',
  styleUrls: ['header.component.css']
})
export class HeaderComponent {
  @Input() name: string;
  @Input() fontWeight: string = 'normal';
  @Input() tooltip: string;
  @Input() icon: string;
  @Input() iconColor: string = 'inherit';
  @Input() loading = false;
  @Input() greyedOut = false;
  @Output() clicked = new EventEmitter();

  get iconOpacity(): number {
    if (!this.loading || this.iconColor !== 'inherit') {
      return 1.0;
    }

    return 0.5;
  }
}
