import { Component } from '@angular/core';

import * as builtinPages from '../src/app/pages/pages.module';
import * as addonPages from './pages/pages.module';

export const mainPages: Component[] = [
  builtinPages.HomePageComponent,
  builtinPages.ExplorerPageComponent,
  builtinPages.ProcessesPageComponent,
  builtinPages.AnomaliesPageComponent,
  builtinPages.LogPageComponent,
  builtinPages.ConnectionsPageComponent
];

export const additionalPages: Component[] = [
  builtinPages.SignaturesPageComponent,
  addonPages.ExamplePageComponent
];
