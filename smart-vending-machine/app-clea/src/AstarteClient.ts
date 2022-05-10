import axios from "axios";
import moment from "moment"

import { TransactionData } from "./types/index"

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
};


type GetTransactionValuesParams = {
  deviceId: string;
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

  async getTransactionData({ deviceId, sinceAfter, since, to, limit, downsamplingTo }: GetTransactionValuesParams) : Promise<TransactionData[]> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "devenv0.face.emotion.detection.Transaction";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/transaction`;
    const requestUrl = new URL(path, appEngineUrl);
    const query: Record<string, string> = {};
    if (sinceAfter) {
      query.sinceAfter = sinceAfter.toISOString();
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
      // console.log("Got response from Astarte:", response);
      if (response.data.data) {
        response.data.data.forEach( (datapoint: any) => {
          datapoint.timestamp = moment.utc(datapoint.timestamp).unix();
        });
        return response.data.data
      }
      return [];
    });
  }
}

export default AstarteClient;
