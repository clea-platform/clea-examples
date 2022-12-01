/*
   Copyright 2022 SECO Mind srl

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

import app from "./angular/main.js";
// @ts-ignore
import polyfills from "./angular/polyfills.js";

type AppProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
  deviceId: string;
};

type UserPreferences = {
  language: "en" | "it";
};

type Settings = {
  themeUrl: string;
  userPreferences: UserPreferences;
};

const AppLifecycle = {
  mount: (container: ShadowRoot, appProps: AppProps, settings: Settings) => {
    const { themeUrl } = settings;

    const cleaStyleElement = document.createElement("link");
    cleaStyleElement.rel = "stylesheet";
    cleaStyleElement.href = themeUrl;
    container.appendChild(cleaStyleElement);

    const polyfillsScriptElement = document.createElement("script");
    polyfillsScriptElement.type = "module";
    polyfillsScriptElement.text = polyfills;
    container.appendChild(polyfillsScriptElement);

    const appScriptElement = document.createElement("script");
    appScriptElement.type = "module";
    appScriptElement.text = app;
    container.appendChild(appScriptElement);

    const angularComponent = document.createElement("my-web-app"); // Change this with the imported component name
    angularComponent.setAttribute("id", "my-web-app-id");
    angularComponent.dataset.appProps = JSON.stringify(appProps);
    angularComponent.dataset.settings = JSON.stringify(settings);
    container.appendChild(angularComponent);
  },
  unmount: (container: ShadowRoot) =>
    document.getElementById("my-web-app-id")?.remove(),
};

export default AppLifecycle;
