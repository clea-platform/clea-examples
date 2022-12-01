import { createCustomElement } from '@angular/elements';
import { createApplication } from '@angular/platform-browser';
import { MyWebAppComponent } from './app/my-web-app/my-web-app.component';

(async () => {

  const app = await createApplication({
    providers: [
      /* your global providers here */
    ],
  });

  const myWebAppElement = createCustomElement(MyWebAppComponent, {
    injector: app.injector,
  });

  customElements.define('my-web-app', myWebAppElement);

})();