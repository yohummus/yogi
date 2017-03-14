import {
  Component,
  OnInit,
} from '@angular/core';

import {
  Router,
  NavigationEnd,
  ActivatedRoute,
} from '@angular/router';

import {
  Title,
}  from '@angular/platform-browser';

import {
  yogiSessionName
} from '../../solution/yogi.config';

import 'font-awesome/css/font-awesome.css';
import 'bootstrap/dist/css/bootstrap-flex.css';

@Component({
  selector: 'ce-app',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})

export class AppComponent implements OnInit {
  constructor(
    private titleService: Title,
    private router: Router,
    private activatedRoute: ActivatedRoute) {
  }

  ngOnInit() {
    this.setupTitleService();
  }

  setupTitleService() {
    this.titleService.setTitle(yogiSessionName);
    this.router.events
    .filter((event) => event instanceof NavigationEnd)
    .map(() => this.activatedRoute)
    .map((route) => {
      while (route.firstChild) {
        route = route.firstChild;
      }

      return route;
    })
    .filter((route) => route.outlet === 'primary')
    .mergeMap((route) => route.data)
    .subscribe((routeData) => {
      this.titleService.setTitle(routeData['pageTitle']);
    });
  }
}
