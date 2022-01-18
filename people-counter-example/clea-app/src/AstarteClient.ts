import axios from "axios";

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
};

type CameraDataParameters = {
  deviceId: string;
  cameraId: string;
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
      appEngineUrl: new URL("/", astarteUrl),
    };
  }

  
  async getCameras({ deviceId }: { deviceId: string }) {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "ai.clea.examples.PeopleCounter";
    const path = `appengine/v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}`;
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

  async getCameraData({
    deviceId,
    cameraId,
    sinceAfter,
    since,
    to,
    limit,
  }: CameraDataParameters) {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "ai.clea.examples.PeopleCounter";
    const path = `appengine/v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/${cameraId}`;
    const requestUrl = new URL(path, appEngineUrl.toString());
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
