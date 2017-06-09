import { NgModule } from '@angular/core';

import { ExplorerModule } from './explorer/explorer.module';
import { TerminalModule } from './terminal/terminal.module';
import { TerminalControlModule } from './terminal-control/terminal-control.module';

@NgModule({
  imports: [
    ExplorerModule,
    TerminalControlModule,
    TerminalModule,
  ],
  exports: [
    ExplorerModule,
    TerminalControlModule,
    TerminalModule,
  ],
  declarations: [
  ]
})
export class SharedModule { }
