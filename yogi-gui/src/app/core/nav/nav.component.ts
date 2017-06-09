import {
  Component,
} from '@angular/core';

import {
  configuredRoutes,
  mainPages,
  additionalPages,
} from '../../app-routing.module';

interface PageInfo {
  routerLink: string;
  title: string;
  icon?: string;
}

@Component({
  selector: 'ce-main-nav',
  templateUrl: 'nav.component.html',
  styleUrls: ['nav.component.css']
})
export class NavComponent {
  private mainPages: PageInfo[] = [];
  private addonPages: PageInfo[] = [];

  constructor() {
    for (let route of configuredRoutes) {
      let page = {
        routerLink: route.path,
        title: (route.component as any).pageTitle,
        icon: (route.component as any).navIcon
      };

      let isMainPage = !!mainPages.find((comp) => comp === route.component);
      if (isMainPage) {
        this.mainPages.push(page);
      }
      else {
        this.addonPages.push(page);
      }
    }
  }
}
