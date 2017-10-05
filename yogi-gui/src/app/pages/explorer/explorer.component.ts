import {
  Component,
} from '@angular/core';

import {
  YogiService,
  SessionStorageService,
} from '../../core/core.module';

@Component({
  selector: 'ce-explorer-page',
  templateUrl: 'explorer.component.html',
  styleUrls: ['explorer.component.css']
})
export class ExplorerPageComponent {
  public static pageTitle = 'Explorer';

  constructor(private yogiService: YogiService, private _asdf: SessionStorageService) {
  }

  onCreateTerminalsButtonClicked() {
    let ti = new yogi.TestInterface(this.yogiService.session);
    // ti.run('createTreeTestTerminals');
    ti.run('createSignatureTestTerminals');
  }
}
