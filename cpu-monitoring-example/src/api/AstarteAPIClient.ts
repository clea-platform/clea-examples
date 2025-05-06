import axios from "axios";
import { Channel, Socket } from "phoenix";

type AstarteAPIClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteAPIClientProps & {
  appEngineUrl: URL;
  socketUrl: URL;
};

type DataPoint = {
  value: number;
  timestamp: Date;
};

type DatastreamDataParameters = {
  deviceId: string;
  since?: Date;
  to?: Date;
  limit?: number;
};

const REQUEST_SAMPLE_LIMIT = 10_000;
type AstarteClientEvent = "socketError" | "socketClose";

async function openNewSocketConnection(
  connectionParams: { socketUrl: string; realm: string; token: string },
  onErrorHandler: () => void,
  onCloseHandler: () => void,
): Promise<Socket> {
  const { socketUrl, realm, token } = connectionParams;

  return new Promise((resolve) => {
    const phoenixSocket = new Socket(socketUrl, {
      params: {
        realm,
        token,
      },
    });
    phoenixSocket.onError(onErrorHandler);
    phoenixSocket.onClose(onCloseHandler);
    phoenixSocket.onOpen(() => {
      resolve(phoenixSocket);
    });
    phoenixSocket.connect();
  });
}

async function joinChannel(
  phoenixSocket: Socket,
  channelString: string,
): Promise<Channel> {
  return new Promise((resolve, reject) => {
    const channel = phoenixSocket.channel(channelString, {});
    channel
      .join()
      .receive("ok", () => {
        resolve(channel);
      })
      .receive("error", (err: unknown) => {
        reject(err);
      });
  });
}

async function leaveChannel(channel: Channel): Promise<void> {
  return new Promise((resolve, reject) => {
    channel
      .leave()
      .receive("ok", () => {
        resolve();
      })
      .receive("error", (err: unknown) => {
        reject(err);
      });
  });
}

async function registerTrigger(
  channel: Channel,
  triggerPayload: object,
): Promise<void> {
  return new Promise((resolve, reject) => {
    channel
      .push("watch", triggerPayload)
      .receive("ok", () => {
        resolve();
      })
      .receive("error", (err: unknown) => {
        reject(err);
      });
  });
}

class AstarteAPIClient {
  config: Config;
  private phoenixSocket: Socket | null;

  private joinedChannels: {
    [roomName: string]: Channel;
  };

  private listeners: {
    [eventName: string]: Array<() => void>;
  };

  constructor({ astarteUrl, realm, token }: AstarteAPIClientProps) {
    const appEngineUrl = new URL("appengine/", astarteUrl);
    const socketUrl = new URL("v1/socket", appEngineUrl);

    this.config = {
      astarteUrl,
      realm,
      token,
      appEngineUrl,
      socketUrl,
    };
    socketUrl.protocol = socketUrl.protocol === "https:" ? "wss:" : "ws:";

    this.phoenixSocket = null;
    this.joinedChannels = {};
    this.listeners = {};
  }

  addListener(eventName: AstarteClientEvent, callback: () => void): void {
    if (!this.listeners[eventName]) {
      this.listeners[eventName] = [];
    }

    this.listeners[eventName].push(callback);
  }

  removeListener(eventName: AstarteClientEvent, callback: () => void): void {
    const previousListeners = this.listeners[eventName];
    if (previousListeners) {
      this.listeners[eventName] = previousListeners.filter(
        (listener) => listener !== callback,
      );
    }
  }

  private dispatch(eventName: AstarteClientEvent): void {
    const listeners = this.listeners[eventName];
    if (listeners) {
      listeners.forEach((listener) => listener());
    }
  }

  private async openSocketConnection(): Promise<Socket> {
    if (this.phoenixSocket) {
      return Promise.resolve(this.phoenixSocket);
    }

    const { socketUrl, realm, token } = this.config;

    return new Promise((resolve) => {
      openNewSocketConnection(
        { socketUrl: socketUrl.toString(), realm, token },
        () => {
          this.dispatch("socketError");
        },
        () => {
          this.dispatch("socketClose");
        },
      ).then((socket) => {
        this.phoenixSocket = socket;
        resolve(socket);
      });
    });
  }

  async joinRoom(roomName: string): Promise<Channel> {
    const { phoenixSocket } = this;
    if (!phoenixSocket) {
      return new Promise((resolve) => {
        this.openSocketConnection().then(() => {
          resolve(this.joinRoom(roomName));
        });
      });
    }

    const channel = this.joinedChannels[roomName];
    if (channel) {
      return Promise.resolve(channel);
    }

    return new Promise((resolve) => {
      joinChannel(phoenixSocket, `rooms:${this.config.realm}:${roomName}`).then(
        (joinedChannel) => {
          this.joinedChannels[roomName] = joinedChannel;
          resolve(joinedChannel);
        },
      );
    });
  }

  async listenForEvents(
    roomName: string,
    eventHandler: (event: any) => void,
  ): Promise<void> {
    const channel = this.joinedChannels[roomName];
    if (!channel) {
      return Promise.reject(
        new Error("Can't listen for room events before joining it first"),
      );
    }

    channel.on("new_event", (jsonEvent: unknown) => {
      eventHandler(jsonEvent);
    });
    return Promise.resolve();
  }

  async registerVolatileTrigger(
    roomName: string,
    triggerPayload: object,
  ): Promise<void> {
    const channel = this.joinedChannels[roomName];
    if (!channel) {
      return Promise.reject(
        new Error("Room not joined, couldn't register trigger"),
      );
    }

    return registerTrigger(channel, triggerPayload);
  }

  async leaveRoom(roomName: string): Promise<void> {
    const channel = this.joinedChannels[roomName];
    if (!channel) {
      return Promise.reject(
        new Error("Can't leave a room without joining it first"),
      );
    }

    return leaveChannel(channel).then(() => {
      delete this.joinedChannels[roomName];
    });
  }

  get joinedRooms(): string[] {
    const rooms: string[] = [];
    Object.keys(this.joinedChannels).forEach((roomName) => {
      rooms.push(roomName);
    });
    return rooms;
  }

  async getCpuTemp({
    deviceId,
    since,
    to,
    limit = REQUEST_SAMPLE_LIMIT,
  }: DatastreamDataParameters): Promise<DataPoint[]> {
    const { appEngineUrl, realm, token } = this.config;
    const path = `v1/${realm}/devices/${deviceId}/interfaces/com.example.poc.CpuTemp/temp`;
    const requestUrl = new URL(path, appEngineUrl);
    const query: Record<string, string> = {
      limit: limit.toString(),
    };

    if (to) {
      query.to = to.toISOString();
    }

    let allData: DataPoint[] = [];
    let hasMore = true;

    while (hasMore) {
      if (since) {
        query.since = since.toISOString();
      }

      requestUrl.search = new URLSearchParams(query).toString();

      await axios({
        method: "get",
        url: requestUrl.toString(),
        headers: {
          Authorization: `Bearer ${token}`,
          "Content-Type": "application/json;charset=UTF-8",
        },
      })
        .then((response) => {
          const data: DataPoint[] = response.data.data;
          allData = allData.concat(data);
          if (data.length < REQUEST_SAMPLE_LIMIT) {
            hasMore = false;
          } else {
            since = new Date(data[data.length - 1].timestamp);
          }
        })
        .catch((error) => {
          if (error.response?.status === 404) {
            hasMore = false;
          } else {
            throw error;
          }
        });
    }

    return allData;
  }

  async getCpuMetrics({
    deviceId,
    since,
    to,
    limit = REQUEST_SAMPLE_LIMIT,
  }: DatastreamDataParameters): Promise<DataPoint[]> {
    const { appEngineUrl, realm, token } = this.config;
    const path = `v1/${realm}/devices/${deviceId}/interfaces/com.example.poc.CpuMetrics/loadavg`;
    const requestUrl = new URL(path, appEngineUrl);
    const query: Record<string, string> = {
      limit: limit.toString(),
    };

    if (to) {
      query.to = to.toISOString();
    }

    let allData: DataPoint[] = [];
    let hasMore = true;

    while (hasMore) {
      if (since) {
        query.since = since.toISOString();
      }

      requestUrl.search = new URLSearchParams(query).toString();

      await axios({
        method: "get",
        url: requestUrl.toString(),
        headers: {
          Authorization: `Bearer ${token}`,
          "Content-Type": "application/json;charset=UTF-8",
        },
      })
        .then((response) => {
          const data: DataPoint[] = response.data.data || [];
          allData = allData.concat(data);

          if (data.length < REQUEST_SAMPLE_LIMIT) {
            hasMore = false;
          } else {
            since = new Date(data[data.length - 1].timestamp);
          }
        })
        .catch((error) => {
          if (error.response?.status === 404) {
            hasMore = false;
          } else {
            throw error;
          }
        });
    }

    return allData;
  }
}

export default AstarteAPIClient;
export type { DataPoint };
