import React from "react";
import ReactDOM from "react-dom";
import { IntlProvider } from "react-intl";
import moment from "moment";

import "bootstrap/dist/css/bootstrap.min.css";
import "bootstrap-icons/font/bootstrap-icons.css";
import "bootstrap";

import { Settings, UserPreferences } from "./types";
import App, { AppProps } from "./App";

import en from "./lang/en.json";
import it from "./lang/it.json";
import "moment/locale/it";
const messages = { en, it };
//TODO: add Intl messages in both languages

const customCss = `
.title-text {
  color: #0426c9;
  font-size: medium;
  font-weight: 500;
  margin-left: 0.5em;
}

.banner-card {
  background-color: #11b0ef;
  border: 0;
  border-radius: 0.8em;
}
.banner-title {
  color: white;
  font-size: 2.2em;
  font-weight: 450;
}
.banner-subtitle {
  color: rgba(255, 255, 255, 0.842);
  font-size: 1.2em;
  font-weight: bold;
}

.alert-banner-card {
  border: 0;
  border-radius: 0.8em;
}
.alert-banner-title {
  color: rgba(255, 255, 255, 0.842);
  font-size: 1.2em;
  font-weight: bold;
}
.alert-banner-subtitle {
  color: rgba(255, 255, 255, 0.842);
  font-size: 1em;
  font-weight: bold;
}
.alert-banner-value {
  color: white;
  font-size: 5.2em;
  font-weight: bolder;
}
.alert-banner-icon {
  width: 15%;
  position: absolute;
  bottom: 0;
  right: 0;
}

.title {
  font-size: medium;
  font-weight: 420;
  margin-left: 0.5em;
}

.unit {
  color: gray;
  font-size: 100;
  font-weight: 420;
}

.value {
  font-size: 3em;
  font-weight: 500;
}

#vertical-box-default {
  height: 40px;
  width: 8px;
  background-color: #11b0ef;
  border-radius: 10px;
  border-width: 0.7px;
  border-style: solid;
  border-color: rgb(175, 171, 171);
}
#vertical-box-danger {
  height: 40px;
  width: 8px;
  background-color: red;
  border-radius: 10px;
  border-width: 0.7px;
  border-style: solid;
  border-color: rgb(175, 171, 171);
}
#vertical-box-warning {
  height: 40px;
  width: 8px;
  background-color: orange;
  border-radius: 10px;
  border-width: 0.7px;
  border-style: solid;
  border-color: rgb(175, 171, 171);
}

#vertical-box-empty {
  height: 40px;
  width: 8px;
  border-radius: 10px;
  border-width: 0.7px;
  border-style: solid;
  border-color: rgb(175, 171, 171);
}

#value-perc {
  font-size: 3.5em;
  font-weight: 800;
}

.filter-toolbar > .btn-group {
  background-color: rgba(128, 128, 128, 0.05);
}
.filter-toolbar > .btn-group label {
  color: gray;
  border: 0;
  border-radius: 0.8em;
  margin-right: 5px;
}
.filter-toolbar > .btn-group input[type="radio"]:checked + label {
  font-weight: 500;
  border-radius: 0.8em;
  color: blue;
  background-color: white;
  box-shadow: 0.2em 0.2em 0.2em rgba(128, 128, 128, 0.4);
}
.filter-toolbar > .btn-group .react-daterange-picker__wrapper {
  border-radius: 0.8em;
}

.filter-toolbar > .btn-group input[type="checkbox"]:checked + label {
  font-weight: 500;
  border-radius: 0.8em;
  color: blue;
  background-color: white;
  box-shadow: 0.2em 0.2em 0.2em rgba(128, 128, 128, 0.4);
}

.filter-toolbar > .btn-group > button {
  border: 0;
  font-weight: 500;
  border-radius: 0.8em;
  color: gray;
  background-color: white;
  box-shadow: 0.2em 0.2em 0.2em rgba(128, 128, 128, 0.4);
}
.filter-toolbar > .btn-group > button:hover {
  border: 0;
  font-weight: 500;
  border-radius: 0.8em;
  color: blue;
  background-color: white;
  box-shadow: 0.2em 0.2em 0.2em rgba(128, 128, 128, 0.4);
}

.filter-toolbar > .btn-group label:hover {
  font-weight: 400;
  border-radius: 0.8em;
  color: blue;
  background-color: white;
  box-shadow: 0.2em 0.2em 0.2em rgba(128, 128, 128, 0.4);
}

.table-scrollbar {
  position: relative;
  height: 400px;
  overflow: auto;
}
.table-wrapper-scroll-y {
  display: block;
}
`;

/**
 * For testing purposes
 */
// const appProps: AppProps = {
//   astarteUrl: new URL("https://api.demo.clea.cloud"),
//   realm: "devenv0",
//   token:
//     "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJhX2FlYSI6WyIuKjo6LioiXSwiYV9jaCI6WyJXQVRDSDo6LioiLCJKT0lOOjouKiJdLCJleHAiOjE2NDgxMjEyMTQsImlhdCI6MTY0ODAzNDgxNCwiaXNzIjoiQXN0YXJ0ZSBDbG91ZCJ9.mh5f2sh2RsbmQLS1OS7Q2P3cMN0wYyswwxgFtHjWOL5aBriVR0LgM0E04q84qvgGKBdNse_UgYuLBtSS05sITA",
//   deviceId: "ypFF4lmVTJmtGjkd_Xwp2g",
// };

// const settings: Settings = {
//   themeUrl: "",
//   userPreferences: { language: "it" } as UserPreferences,
// };
// const { themeUrl, userPreferences } = settings;
// const { language } = userPreferences;

// moment.updateLocale(language, {
//   week: {
//     dow: 1, // Monday is the first day of the week.
//   },
// });

// ReactDOM.render(
//   <React.StrictMode>
//     <link href={themeUrl} type="text/css" rel="stylesheet" />
//     <IntlProvider messages={messages[language]} locale={language} defaultLocale="en">
//       <App {...appProps} />
//     </IntlProvider>
//   </React.StrictMode>,
//   document.getElementById("root")
// );

/**
 * For production purposes
 */
const AppLifecycle = {
  mount: (container: ShadowRoot, appProps: AppProps, settings: Settings) => {
    const { themeUrl, userPreferences } = settings;
    const { language } = userPreferences;

    moment.updateLocale(language, {
      week: {
        dow: 1, // Monday is the first day of the week.
      },
    });

    //TODO: remove these logs in production
    // console.log(appProps);
    // console.log(settings);

    ReactDOM.render(
      <>
        <link href={themeUrl} type="text/css" rel="stylesheet" />
        <style>{customCss}</style>
        <IntlProvider messages={messages[language]} locale={language} defaultLocale="en">
          <App {...appProps} />
        </IntlProvider>
      </>,
      container
    );
  },
  unmount: (container: ShadowRoot) => ReactDOM.unmountComponentAtNode(container),
};

export default AppLifecycle;
