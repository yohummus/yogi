import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { NgbModule } from '@ng-bootstrap/ng-bootstrap';

import { CoreModule } from '../core/core.module';
import { SharedModule } from '../shared/shared.module';

import { AnomaliesPageComponent } from './anomalies/anomalies.component';
import { ConnectionsPageComponent } from './connections/connections.component';
import { ExplorerPageComponent } from './explorer/explorer.component';
import { HomePageComponent } from './home/home.component';
import { LogPageComponent } from './log/log.component';
import { ProcessesPageComponent } from './processes/processes.component';
import { SignaturesPageComponent } from './signatures/signatures.component';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    NgbModule,
    CoreModule,
    SharedModule,
  ],
  exports: [
  ],
  declarations: [
    AnomaliesPageComponent,
    ConnectionsPageComponent,
    ExplorerPageComponent,
    HomePageComponent,
    LogPageComponent,
    ProcessesPageComponent,
    SignaturesPageComponent,
  ]
})
export class PagesModule { }

export { AnomaliesPageComponent } from './anomalies/anomalies.component';
export { ConnectionsPageComponent } from './connections/connections.component';
export { ExplorerPageComponent } from './explorer/explorer.component';
export { HomePageComponent } from './home/home.component';
export { LogPageComponent } from './log/log.component';
export { ProcessesPageComponent } from './processes/processes.component';
export { SignaturesPageComponent } from './signatures/signatures.component';
