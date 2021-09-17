import React from "react";
import ReactDOM from "react-dom";
import { IntlProvider, FormattedMessage } from "react-intl";

import AstarteClient from "./AstarteClient";

import en from "./lang/en.json";
import it from "./lang/it.json";

const messages = { en, it };

const { useCallback, useEffect, useMemo, useState } = React;

type AppProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
  deviceId: string;
};

const App = ({ astarteUrl, realm, token, deviceId }: AppProps) => {
  const [ledValue, setLedValue] = useState(false);
  const [isSettingLedValue, setIsSettingLedValue] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const astarteClient = useMemo(() => {
    return new AstarteClient({ astarteUrl, realm, token });
  }, [astarteUrl, realm, token]);

  const handleValueChange: React.ChangeEventHandler<HTMLInputElement> =
    useCallback(
      (event) => {
        const value = event.target.checked;
        setLedValue(value);
        setIsSettingLedValue(true);
        astarteClient
          .setLEDValue({ deviceId, value })
          .catch(() => setLedValue(!value))
          .finally(() => setIsSettingLedValue(false));
      },
      [astarteClient, deviceId]
    );

  useEffect(() => {
    astarteClient
      .getLEDValues({ deviceId })
      .then((data) => {
        setLedValue(data ? data.value : false);
      })
      .catch(() => setLedValue(false))
      .finally(() => setIsLoading(false));
  }, []);

  return (
    <div className="p-4">
      {isLoading ? (
        <div>
          <FormattedMessage id="loading" defaultMessage="Loading..." />
        </div>
      ) : (
        <div className="p-3">
          <div className="custom-control custom-switch">
            <input
              type="checkbox"
              className="custom-control-input"
              id="ledStatus"
              disabled={isSettingLedValue}
              checked={ledValue}
              onChange={handleValueChange}
            />
            <label className="custom-control-label" htmlFor="ledStatus">
              <FormattedMessage id="led_status" defaultMessage="LED Status" />
            </label>
          </div>
        </div>
      )}
    </div>
  );
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
    const { themeUrl, userPreferences } = settings;
    const { language } = userPreferences;

    ReactDOM.render(
      <>
        <link href={themeUrl} type="text/css" rel="stylesheet" />
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
