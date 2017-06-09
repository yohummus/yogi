import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NgbModule } from '@ng-bootstrap/ng-bootstrap';

import { TerminalComponent } from './terminal.component';
import { TerminalControlModule } from '../terminal-control/terminal-control.module';

@NgModule({
  imports: [
    CommonModule,
    NgbModule,
    TerminalControlModule,
  ],
  exports: [
    TerminalComponent,
  ],
  declarations: [
    TerminalComponent,
  ]
})
export class TerminalModule { }
