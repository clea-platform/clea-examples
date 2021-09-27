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

import axios from "axios";

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
};

type SensorDataParameters = {
  deviceId: string;
  sensorId: string;
  sinceAfter?: string;
  since?: Date;
  to?: Date;
  limit?: number;
};

class AstarteClient {
  config: Config;

  constructor({ astarteUrl, realm, token }: AstarteClientProps) {
    this.config = {
      astarteUrl,
      realm,
      token,
      appEngineUrl: new URL("appengine/", astarteUrl),
    };
  }

  async getSensors({ deviceId }: { deviceId: string }) {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "org.astarte-platform.genericsensors.Values";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}`;
    const requestUrl = new URL(path, appEngineUrl);
    return axios({
      method: "get",
      url: requestUrl.toString(),
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json;charset=UTF-8",
      },
    }).then((response) => Object.keys(response.data.data));
  }

  async getSensorData({
    deviceId,
    sensorId,
    sinceAfter,
    since,
    to,
    limit,
  }: SensorDataParameters) {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "org.astarte-platform.genericsensors.Values";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/${sensorId}/value`;
    const requestUrl = new URL(path, appEngineUrl);
    const query: Record<string, string> = {};
    if (sinceAfter) {
      query.sinceAfter = sinceAfter;
    }
    if (since) {
      query.since = since.toISOString();
    }
    if (to) {
      query.to = to.toISOString();
    }
    if (limit) {
      query.limit = limit.toString();
    }
    requestUrl.search = new URLSearchParams(query).toString();
    return axios({
      method: "get",
      url: requestUrl.toString(),
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json;charset=UTF-8",
      },
    }).then((response) => response.data.data);
  }
}

export default AstarteClient;
