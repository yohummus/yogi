import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NgbModule } from '@ng-bootstrap/ng-bootstrap';

import { ExplorerComponent } from './explorer.component';
import { HeaderComponent } from './header/header.component';
import { TreeComponent } from './tree/tree.component';
import { TerminalControlModule } from '../terminal-control/terminal-control.module';
import { TerminalModule } from '../terminal/terminal.module';

@NgModule({
  imports: [
    CommonModule,
    NgbModule,
    TerminalControlModule,
    TerminalModule,
  ],
  exports: [
    ExplorerComponent,
  ],
  declarations: [
    ExplorerComponent,
    HeaderComponent,
    TreeComponent,
  ]
})
export class ExplorerModule { }

export { ExplorerComponent } from './explorer.component';
