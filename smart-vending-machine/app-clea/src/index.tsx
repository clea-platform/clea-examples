import React from 'react';
import ReactDOM from 'react-dom';
import { IntlProvider } from "react-intl";
import moment from "moment";

import './index.css';
import 'bootstrap/dist/css/bootstrap.min.css';

import { Settings, UserPreferences } from "./types";
import App, { AppProps }from './App';
import reportWebVitals from './reportWebVitals';

import en from "./lang/en.json";
import it from "./lang/it.json";
import "moment/locale/it";
const messages = { en, it };

// Custom CSS to inject into the Clea App
const customCss = `
  .button-style-active{
    color: #0631cc;
    font-weight: 500;
    background-color: white;
    border-radius: 12px;
  }
  .button-style-disabled {
    color: #abb0ba;
    font-weight: 500;
    background-color: #f8f9fa;
    border-radius: 12px;
    border-color: #f8f9fa;
    border-width: 0px;
  }
  .button-style-disabled:hover {
    color: #0631cc;
    font-weight: 500;
    background-color: white;
    border-radius: 12px;
  }
  .card-custom {
    box-shadow: 0.3em 0.3em 0.8em rgba(128, 128, 128, 0.295);
    border-radius: 15px
  }
  .table>:not(:first-child) {
    border-top: 0px solid #dadada;
  }
  .table-bordered>:not(caption)>*>* {
    border-width: 0 0px;
  }
`;


const dateRangePickerCss = `
.react-daterange-picker {
  display: inline-flex;
  position: relative;
}
.react-daterange-picker,
.react-daterange-picker *,
.react-daterange-picker *:before,
.react-daterange-picker *:after {
  -moz-box-sizing: border-box;
  -webkit-box-sizing: border-box;
  box-sizing: border-box;
}
.react-daterange-picker--disabled {
  background-color: #f0f0f0;
  color: #6d6d6d;
}
.react-daterange-picker__wrapper {
  display: flex;
  flex-grow: 1;
  flex-shrink: 0;
  align-items: center;
  border: thin solid gray;
}
.react-daterange-picker__inputGroup {
  min-width: calc((4px * 3) +  0.54em * 8  +  0.217em * 2);
  height: 100%;
  flex-grow: 1;
  padding: 0 2px;
  box-sizing: content-box;
}
.react-daterange-picker__inputGroup__divider {
  padding: 1px 0;
  white-space: pre;
}
.react-daterange-picker__inputGroup__input {
  min-width: 0.54em;
  height: 100%;
  position: relative;
  padding: 0 1px;
  border: 0;
  background: none;
  font: inherit;
  box-sizing: content-box;
  -moz-appearance: textfield;
}
.react-daterange-picker__inputGroup__input::-webkit-outer-spin-button,
.react-daterange-picker__inputGroup__input::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}
.react-daterange-picker__inputGroup__input:invalid {
  background: rgba(255, 0, 0, 0.1);
}
.react-daterange-picker__inputGroup__input--hasLeadingZero {
  margin-left: -0.54em;
  padding-left: calc(1px +  0.54em);
}
.react-daterange-picker__button {
  border: 0;
  background: transparent;
  padding: 4px 6px;
}
.react-daterange-picker__button:enabled {
  cursor: pointer;
}
.react-daterange-picker__button:enabled:hover .react-daterange-picker__button__icon,
.react-daterange-picker__button:enabled:focus .react-daterange-picker__button__icon {
  stroke: #0078d7;
}
.react-daterange-picker__button:disabled .react-daterange-picker__button__icon {
  stroke: #6d6d6d;
}
.react-daterange-picker__button svg {
  display: inherit;
}
.react-daterange-picker__calendar {
  width: 350px;
  max-width: 100vw;
  position: absolute;
  top: 100%;
  left: 0;
  z-index: 1;
}
.react-daterange-picker__calendar--closed {
  display: none;
}
.react-daterange-picker__calendar .react-calendar {
  border-width: thin;
}

.react-calendar {
  width: 350px;
  max-width: 100%;
  background: white;
  border: 1px solid #a0a096;
  font-family: Arial, Helvetica, sans-serif;
  line-height: 1.125em;
}
.react-calendar--doubleView {
  width: 700px;
}
.react-calendar--doubleView .react-calendar__viewContainer {
  display: flex;
  margin: -0.5em;
}
.react-calendar--doubleView .react-calendar__viewContainer > * {
  width: 50%;
  margin: 0.5em;
}
.react-calendar,
.react-calendar *,
.react-calendar *:before,
.react-calendar *:after {
  -moz-box-sizing: border-box;
  -webkit-box-sizing: border-box;
  box-sizing: border-box;
}
.react-calendar button {
  margin: 0;
  border: 0;
  outline: none;
}
.react-calendar button:enabled:hover {
  cursor: pointer;
}
.react-calendar__navigation {
  display: flex;
  height: 44px;
  margin-bottom: 1em;
}
.react-calendar__navigation button {
  min-width: 44px;
  background: none;
}
.react-calendar__navigation button:disabled {
  background-color: #f0f0f0;
}
.react-calendar__navigation button:enabled:hover,
.react-calendar__navigation button:enabled:focus {
  background-color: #e6e6e6;
}
.react-calendar__month-view__weekdays {
  text-align: center;
  text-transform: uppercase;
  font-weight: bold;
  font-size: 0.75em;
}
.react-calendar__month-view__weekdays__weekday {
  padding: 0.5em;
}
.react-calendar__month-view__weekNumbers .react-calendar__tile {
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 0.75em;
  font-weight: bold;
}
.react-calendar__month-view__days__day--weekend {
  color: #d10000;
}
.react-calendar__month-view__days__day--neighboringMonth {
  color: #757575;
}
.react-calendar__year-view .react-calendar__tile,
.react-calendar__decade-view .react-calendar__tile,
.react-calendar__century-view .react-calendar__tile {
  padding: 2em 0.5em;
}
.react-calendar__tile {
  max-width: 100%;
  padding: 10px 6.6667px;
  background: none;
  text-align: center;
  line-height: 16px;
}
.react-calendar__tile:disabled {
  background-color: #f0f0f0;
}
.react-calendar__tile:enabled:hover,
.react-calendar__tile:enabled:focus {
  background-color: #e6e6e6;
}
.react-calendar__tile--now {
  background: #ffff76;
}
.react-calendar__tile--now:enabled:hover,
.react-calendar__tile--now:enabled:focus {
  background: #ffffa9;
}
.react-calendar__tile--hasActive {
  background: #76baff;
}
.react-calendar__tile--hasActive:enabled:hover,
.react-calendar__tile--hasActive:enabled:focus {
  background: #a9d4ff;
}
.react-calendar__tile--active {
  background: #006edc;
  color: white;
}
.react-calendar__tile--active:enabled:hover,
.react-calendar__tile--active:enabled:focus {
  background: #1087ff;
}
.react-calendar--selectRange .react-calendar__tile--hover {
  background-color: #e6e6e6;
}
`;

/**
 * For testing purposes
 */
/* const appProps: AppProps = {
  astarteUrl: new URL("https://api.demo.clea.cloud"),
  realm: "devenv0",
  token:
  "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJhX2FlYSI6WyIuKjo6LioiXSwiYV9jaCI6WyJXQVRDSDo6LioiLCJKT0lOOjouKiJdLCJleHAiOjE2NDcwMTM0NDAsImlhdCI6MTY0NjkyNzA0MCwiaXNzIjoiQXN0YXJ0ZSBDbG91ZCJ9.uqgphLvNPcS9Rhe39CbH8wPdpN3NNLOicZ_SakylU-_IdE-ea5OQ9qNBXemUP66Pxt6FmqX4oPBmNxJO_u_rZw",
  deviceId: "TABLETnwTeS5tUV3UGw1eQ",
};

const settings: Settings = {
  themeUrl: "",
  userPreferences: { language: "it" } as UserPreferences,
};
const { themeUrl, userPreferences } = settings;
const { language } = userPreferences;

moment.updateLocale(language, {
  week: {
    dow: 1, // Monday is the first day of the week.
  },
});

ReactDOM.render(
    <IntlProvider messages={messages[language]} locale={language} defaultLocale="en">
      <link href={themeUrl} type="text/css" rel="stylesheet" />
      <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css"/>
      <style>{customCss}</style>
      <style>{dateRangePickerCss}</style>
      <App {...appProps} />
    </IntlProvider>,
  document.getElementById("app")
); */


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

    // console.log(appProps);
    // console.log(settings);

    ReactDOM.render(
      <>
        <link href={themeUrl} type="text/css" rel="stylesheet" />
        <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css"/>
        <style>{customCss}</style>
        <style>{dateRangePickerCss}</style>
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
