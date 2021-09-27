import axios from "axios";

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
};

type GetLEDValuesParams = {
  deviceId: string;
};

type SetLEDValueParams = {
  deviceId: string;
  value: boolean;
};

type LedData = {
  value: boolean;
  timestamp: string;
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

  async getLEDValues({
    deviceId,
  }: GetLEDValuesParams): Promise<LedData | null> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "org.astarteplatform.esp32.ServerDatastream";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}`;
    const requestUrl = new URL(path, appEngineUrl);
    return axios({
      method: "get",
      url: requestUrl.toString(),
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json;charset=UTF-8",
      },
    }).then((response) => {
      return response.data.data?.led || null;
    });
  }

  async setLEDValue({ deviceId, value }: SetLEDValueParams): Promise<LedData> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "org.astarteplatform.esp32.ServerDatastream";
    const path = `v1/${realm}/devices/${deviceId}/interfaces/${interfaceName}/led`;
    const requestUrl = new URL(path, appEngineUrl);
    return axios({
      method: "post",
      url: requestUrl.toString(),
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json;charset=UTF-8",
      },
      data: { data: value },
    }).then((response) => response.data.data);
  }
}

export default AstarteClient;
