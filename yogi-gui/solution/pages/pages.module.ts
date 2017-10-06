import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { SharedModule } from '../shared/shared.module';

import { ExamplePageComponent } from './example/example.component';

@NgModule({
  imports: [
    CommonModule,
    SharedModule
  ],
  exports: [
  ],
  declarations: [
    ExamplePageComponent
  ]
})
export class PagesModule { }

export { ExamplePageComponent } from './example/example.component';
