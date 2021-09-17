import React from "react";
import type { ChangeEvent } from "react";
import ReactDOM from "react-dom";
import { IntlProvider, FormattedMessage, useIntl } from "react-intl";

import AstarteClient from "./AstarteClient";
import Plot from "./Plot";
import type { Sensors, SensorData } from "./AstarteClient";

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
      {options.map((sensorId) => (
        <option key={sensorId} value={sensorId}>
          {sensorId}
        </option>
      ))}
    </select>
  );
};

const DataViewer = ({ value }: { value: SensorData }) => {
  const { x, y, z } = value;

  return (
    <div className="row">
      <div className="col col-sm-2">
        <table className="table table-borderless">
          <tr>
            <td>x</td>
            <td className="text-right">{x.toFixed(6)}</td>
          </tr>
          <tr>
            <td>y</td>
            <td className="text-right">{y.toFixed(6)}</td>
          </tr>
          <tr>
            <td>z</td>
            <td className="text-right">{z.toFixed(6)}</td>
          </tr>
        </table>
      </div>
      <div className="col col-sm-10" style={{ height: "50em" }}>
        <Plot acceleration={value} />
      </div>
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
  const [sensors, setSensors] = useState<Sensors>({});
  const [selectedSensorId, setSelectedSensorId] = useState("");
  const [isLoading, setIsLoading] = useState(true);
  const astarteClient = useMemo(() => {
    return new AstarteClient({ astarteUrl, realm, token });
  }, [astarteUrl, realm, token]);

  const handleChannelEvent = (e: any) => {
    setSensors((prevState) => {
      return {
        ...prevState,
        [e.event.path]: e.event.value,
      };
    });
  };

  useEffect(() => {
    astarteClient
      .getSensors(deviceId)
      .then((data) => {
        setSensors(data);
        const sensorIds = Object.keys(data);
        if (sensorIds.length > 0) {
          setSelectedSensorId(sensorIds[0]);
        }
      })
      .catch(() => setSensors({}))
      .finally(() => setIsLoading(false));

    const salt = Math.floor(Math.random() * 10000);
    const roomName = `accelerometer_${deviceId}_${salt}`;

    astarteClient
      .joinRoom(roomName)
      .then(() => {
        console.log(`Joined room for device ${deviceId}`);
        astarteClient.listenForEvents(roomName, handleChannelEvent);

        const dataTriggerPayload = {
          name: `datatrigger-${deviceId}`,
          device_id: deviceId,
          simple_trigger: {
            type: "data_trigger",
            on: "incoming_data",
            interface_name: "*",
            value_match_operator: "*",
            match_path: "/*",
          },
        };

        astarteClient
          .registerVolatileTrigger(roomName, dataTriggerPayload)
          .then(() => {
            console.log("Watching for device data events");
          })
          .catch(() => {
            console.log("Coulnd't watch for device data events");
          });
      })
      .catch(() => {
        console.log(`Couldn't join device ${deviceId} room`);
      });
  }, []);

  const handleSensorChange = useCallback((e) => {
    const sensorId = e.target.value;
    setSelectedSensorId(sensorId);
  }, []);

  const sensorIds = Object.keys(sensors);

  return (
    <div className="p-4">
      {isLoading ? (
        <div>
          <FormattedMessage id="loading" defaultMessage="Loading..." />
        </div>
      ) : (
        <div>
          <SensorSelect
            options={sensorIds}
            value={selectedSensorId}
            onChange={handleSensorChange}
          />
          {selectedSensorId && <DataViewer value={sensors[selectedSensorId]} />}
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
