
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
import _ from "lodash";

const messages                          = { en, it };
const { useEffect, useMemo, useState }  = React;


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

    const astarte_client    = useMemo(() => {
        const astarte_params    = {
            astarte_url : astarteUrl,
            realm,
            token
        }
        return new AstarteClient(astarte_params);
    }, [astarteUrl, realm, token]);

    return (
        <Fragment>
            <MainApp astarte_client={astarte_client} device_id={deviceId}/>
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
