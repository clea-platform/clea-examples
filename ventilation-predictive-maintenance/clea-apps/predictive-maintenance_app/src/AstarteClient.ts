import axios from "axios";
import moment from "moment";

import { AirflowData, AirFluxEvent, AirPollutionData, AirVelocityData } from "./types/index";

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
};

type AirFluxDataParameters = {
  deviceId: string;
  endpoint?: string;
  sinceAfter?: Date;
  since?: Date;
  to?: Date;
  limit?: number;
  downsamplingTo?: number;
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

  async getAirFlowValues({ deviceId, sinceAfter, since, to, limit, downsamplingTo }: AirFluxDataParameters): Promise<AirflowData[]> {
    return this.getAirFluxValues({ deviceId, sinceAfter, since, to, limit, downsamplingTo, endpoint: "flow" });
  }
  async getPollutionValues({ deviceId, sinceAfter, since, to, limit, downsamplingTo }: AirFluxDataParameters): Promise<AirPollutionData[]> {
    return this.getAirFluxValues({ deviceId, sinceAfter, since, to, limit, downsamplingTo, endpoint: "pollution" });
  }
  async getVelocityValues({ deviceId, sinceAfter, since, to, limit, downsamplingTo }: AirFluxDataParameters): Promise<AirVelocityData[]> {
    return this.getAirFluxValues({ deviceId, sinceAfter, since, to, limit, downsamplingTo, endpoint: "velocity" });
  }

  private async getAirFluxValues({
    deviceId,
    endpoint,
    sinceAfter,
    since,
    to,
    limit,
    downsamplingTo,
  }: AirFluxDataParameters): Promise<AirflowData[] | AirPollutionData[] | AirVelocityData[]> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "com.seco.AirData";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/${endpoint}`;
    const requestUrl = new URL(path, appEngineUrl);
    const query: Record<string, string> = {};
    if (sinceAfter) {
      query.since_after = sinceAfter.toISOString();
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
    if (downsamplingTo) {
      if (downsamplingTo > 2) {
        query.downsample_to = downsamplingTo.toString();
      } else {
        console.warn("[AstarteClient] downsamplingTo must be > 2");
      }
    }
    requestUrl.search = new URLSearchParams(query).toString();
    return axios({
      method: "get",
      url: requestUrl.toString(),
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json;charset=UTF-8",
      },
    }).then((response) => {
      // console.log("[AirFluxQuery] Got response from Astarte:", response);
      if (response.data.data) {
        return response.data.data.map((datapoint: any) => {
          return { value: datapoint.value, timestamp: moment.utc(datapoint.timestamp).valueOf() };
        });
      }
      return [];
    });
  }

  async getFluxEventsHistoryValues({
    deviceId,
    endpoint,
    sinceAfter,
    since,
    to,
    limit,
    downsamplingTo,
  }: AirFluxDataParameters): Promise<AirFluxEvent[]> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "com.seco.EventsHistory";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/${endpoint}`;
    const requestUrl = new URL(path, appEngineUrl);
    const query: Record<string, string> = {};
    if (sinceAfter) {
      query.since_after = sinceAfter.toISOString();
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
    if (downsamplingTo) {
      if (downsamplingTo > 2) {
        query.downsample_to = downsamplingTo.toString();
      } else {
        console.warn("[AstarteClient] downsamplingTo must be > 2");
      }
    }
    requestUrl.search = new URLSearchParams(query).toString();
    return axios({
      method: "get",
      url: requestUrl.toString(),
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json;charset=UTF-8",
      },
    }).then((response) => {
      // console.log("[EventsHistoryQuery] Got response from Astarte:", response);
      if (response.data.data) {
        return response.data.data.map((datapoint: any) => {
          return {
            detection: datapoint.detection,
            measure: datapoint.measure,
            noteCode: datapoint.noteCode,
            timestamp: moment.utc(datapoint.timestamp).valueOf(),
          };
        });
      }
      return [];
    });
  }
}

export default AstarteClient;
