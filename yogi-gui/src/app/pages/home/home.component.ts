import {
  Component,
} from '@angular/core';

@Component({
  selector: 'ce-home-page',
  templateUrl: 'home.component.html',
  styleUrls: ['home.component.css']
})
export class HomePageComponent {
  public static pageTitle = 'Home';
  public static navIcon = 'fa-anchor';
}
