import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { NgbModule } from '@ng-bootstrap/ng-bootstrap';

import { InputGroupComponent } from './input-group/input-group.component';
import { TerminalControlComponent } from './terminal-control.component';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    NgbModule,
  ],
  exports: [
    TerminalControlComponent
  ],
  declarations: [
    TerminalControlComponent,
    InputGroupComponent,
  ]
})
export class TerminalControlModule { }
