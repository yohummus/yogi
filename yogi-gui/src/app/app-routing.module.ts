import {
  NgModule,
  Type,
} from '@angular/core';

import {
  Route,
  Routes,
  RouterModule,
} from '@angular/router';

import {
  YogiResolve,
} from './core/yogi/yogi-resolve.service';

import {
  mainPages,
  additionalPages,
} from '../../solution/pages.config';

export const configuredRoutes: Routes = mainPages.concat(additionalPages)
  .map((component: Type<any>) => {
    let path: string = (component as any).name.replace('PageComponent', '').toLowerCase();

    let resolve = {
      yogi: YogiResolve
    };

    let data = {
      pageTitle: (component as any).pageTitle
    };

    return { path, component, resolve, data };
  });

const routes: Routes = [
  {
    path: '',
    redirectTo: configuredRoutes[0].path,
    pathMatch: 'full'
  } as Route
].concat(configuredRoutes);

@NgModule({
  imports: [
    RouterModule.forRoot(routes)
  ],
  exports: [
    RouterModule
  ],
  providers: [
    YogiResolve
  ]
})
export class AppRoutingModule { }

export { mainPages, additionalPages } from '../../solution/pages.config';
