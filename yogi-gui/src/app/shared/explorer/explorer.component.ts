import {
  Component,
  Input,
} from '@angular/core';

import {
  TreeComponent,
} from './tree/tree.component';

@Component({
  selector: 'ce-explorer',
  templateUrl: 'explorer.component.html',
  styleUrls: ['explorer.component.css']
})
export class ExplorerComponent {
  @Input() root: string = '/';
  @Input() showRoot: boolean = false;
  @Input() filter: string = '.*';
  @Input() expandAll: boolean = false;
}
