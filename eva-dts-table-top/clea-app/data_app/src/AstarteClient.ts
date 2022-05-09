import axios from "axios";
import moment from "moment";

import { UnitsData, RevenuesData } from "./types/index";

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
};

type BeverageDataParameters = {
  deviceId: string;
  beverageId: string;
  sinceAfter?: string;
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

  async getUnitsData({ deviceId, beverageId, sinceAfter, since, to, limit, downsamplingTo }: BeverageDataParameters): Promise<UnitsData[]> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "ai.clea.examples.evsdtstabletop.BeverageData";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/${beverageId}/units`;
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
      console.log("[Units] Got response from Astarte:", response);
      if (response.data.data) {
        return response.data.data.map((datapoint: any) => {
          return { value: datapoint.value, timestamp: moment.utc(datapoint.timestamp).unix() };
        });
      }
      return [];
    });
  }

  async getRevenuesData({
    deviceId,
    beverageId,
    sinceAfter,
    since,
    to,
    limit,
    downsamplingTo,
  }: BeverageDataParameters): Promise<RevenuesData[]> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "ai.clea.examples.evsdtstabletop.BeverageData";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/${beverageId}/revenues`;
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
      console.log("[Revenues] Got response from Astarte:", response);
      if (response.data.data) {
        return response.data.data.map((datapoint: any) => {
          return { value: datapoint.value, timestamp: moment.utc(datapoint.timestamp).unix() };
        });
      }
      return [];
    });
  }
}

export default AstarteClient;
