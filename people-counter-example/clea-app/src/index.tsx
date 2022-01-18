import React, { Fragment } from "react";
import ReactDOM from "react-dom";
import { IntlProvider, FormattedMessage, useIntl } from "react-intl";

import AstarteClient from "./AstarteClient";

import en from "./lang/en.json";
import it from "./lang/it.json";

// include all style
// @ts-ignore
import appStyle from './css/app.css';
// @ts-ignore
import chartsStyle from "../node_modules/apexcharts/dist/apexcharts.css";

import { MainApp } from "./MainApp";

const messages = { en, it };
const { useEffect, useMemo, useState } = React;

type AppProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
  deviceId: string;
};

type Settings = {
  themeUrl: string;
  userPreferences: UserPreferences;
}

const App = ({ astarteUrl, realm, token, deviceId }: AppProps) => {
  const [cameras, setCameras] = useState<string[]>([]);

  const astarteClient = useMemo(() => {
    return new AstarteClient({ astarteUrl, realm, token });
  }, [astarteUrl, realm, token]);
 
  useEffect(() => {
    if(cameras.length == 0){
      let mount = true;
      const intervalID = setInterval(() => {
        astarteClient
          .getCameras({ deviceId })
          .then((data) => {
            if (mount){
              setCameras(data);
            }
          })
          .catch(() => {
            if (mount){
              setCameras([]);
            }
          });
      }, 5000);

      return () => {
        clearInterval(intervalID);
        mount = false;
      }
    }
  },[cameras]);

  return (
    <Fragment>
      <MainApp cameras={cameras} astarteClient={astarteClient} deviceId={deviceId} />
    </Fragment>
  );
};

type UserPreferences = {
  language: "en" | "it";
};

const AppLifecycle = {
  mount: (
    container: ShadowRoot,
    appProps: AppProps,
    settings: Settings
  ) => {
    const { themeUrl, userPreferences } = settings;
    const { language } = userPreferences;

    ReactDOM.render(
      <>
        <link href={themeUrl} type="text/css" rel="stylesheet" />
        <style>{chartsStyle.toString()}</style>
        <style>{appStyle.toString()}</style>
        <IntlProvider
          messages={messages[language]}
          locale={language}
          defaultLocale="en"
        >
          <App {...appProps} />
        </IntlProvider>
      </>,
      container
    );
  },
  unmount: (container: ShadowRoot) =>
    ReactDOM.unmountComponentAtNode(container),
};

export default AppLifecycle;
