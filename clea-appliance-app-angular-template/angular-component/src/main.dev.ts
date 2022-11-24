import { createCustomElement } from '@angular/elements';
import { platformBrowserDynamic } from '@angular/platform-browser-dynamic';

import { AppModule } from './app/app.module';
import { MyWebAppComponent } from './app/my-web-app/my-web-app.component';

const app = platformBrowserDynamic();
app.bootstrapModule(AppModule)
  .catch(err => console.error(err));

const myWebAppElement = createCustomElement(MyWebAppComponent, {
  injector: app.injector,
});

customElements.define('my-web-app', myWebAppElement);
