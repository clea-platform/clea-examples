/*
   Copyright 2021 SECO Mind srl

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

import React from "react";
import type { ChangeEvent } from "react";
import ReactDOM from "react-dom";
import Chart from "react-apexcharts";
import type { ApexOptions } from "apexcharts";
import { IntlProvider, FormattedMessage, useIntl } from "react-intl";

import AstarteClient from "./AstarteClient";

// @ts-ignore
import chartsStyle from "../node_modules/apexcharts/dist/apexcharts.css";

import en from "./lang/en.json";
import it from "./lang/it.json";

const messages = { en, it };

const { useCallback, useEffect, useMemo, useState } = React;

type SensorSelect = {
  options: string[];
  value: string;
  onChange: (e: ChangeEvent<HTMLSelectElement>) => void;
};

const SensorSelect = ({ options, value, onChange }: SensorSelect) => {
  const intl = useIntl();

  if (options.length === 0) {
    return (
      <div>
        <FormattedMessage
          id="no_sensors"
          defaultMessage="No available sensors"
        />
      </div>
    );
  }

  return (
    <select value={value} onChange={onChange}>
      <option value={""} disabled>
        {intl.formatMessage({
          id: "select_sensor",
          defaultMessage: "Select a sensor",
        })}
      </option>
      {options.map((option) => (
        <option key={option} value={option}>
          {option}
        </option>
      ))}
    </select>
  );
};

const chartOptions: ApexOptions = {
  xaxis: {
    type: "datetime",
  },
};

type AstarteData = {
  value: number;
  timestamp: number;
};

type DataChartProps = {
  sensorId: string;
  data: AstarteData[];
};

const DataChart = ({ sensorId, data }: DataChartProps) => {
  if (data.length === 0) {
    return (
      <div>
        <FormattedMessage id="no_data" defaultMessage="No recent data" />
      </div>
    );
  }

  const series = useMemo(
    () => [
      {
        name: sensorId,
        data: data.map((d) => [new Date(d.timestamp), d.value]),
      },
    ],
    [data]
  );

  return (
    <div className="chart-container">
      <Chart type="line" width={768} options={chartOptions} series={series} />
    </div>
  );
};

type AppProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
  deviceId: string;
};

const App = ({ astarteUrl, realm, token, deviceId }: AppProps) => {
  const [sensors, setSensors] = useState<string[]>([]);
  const [selectedSensorId, setSelectedSensorId] = useState("");
  const [deviceData, setDeviceData] = useState<AstarteData[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const astarteClient = useMemo(() => {
    return new AstarteClient({ astarteUrl, realm, token });
  }, [astarteUrl, realm, token]);

  useEffect(() => {
    astarteClient
      .getSensors({ deviceId })
      .then((data) => setSensors(data))
      .catch(() => setSensors([]))
      .finally(() => setIsLoading(false));
  }, []);

  const handleSensorChange = useCallback((e) => {
    const sensorId = e.target.value;
    setSelectedSensorId(sensorId);
    astarteClient
      .getSensorData({ deviceId, sensorId, limit: 1000 })
      .then((data) => setDeviceData(data))
      .catch(() => setDeviceData([]));
  }, []);

  return (
    <div className="p-4">
      {isLoading ? (
        <div>
          <FormattedMessage id="loading" defaultMessage="Loading..." />
        </div>
      ) : (
        <div>
          {Array.isArray(sensors) && (
            <SensorSelect
              options={sensors}
              value={selectedSensorId}
              onChange={handleSensorChange}
            />
          )}
          {Array.isArray(deviceData) && (
            <DataChart sensorId={selectedSensorId} data={deviceData} />
          )}
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
        <style>{chartsStyle.toString()}</style>
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
