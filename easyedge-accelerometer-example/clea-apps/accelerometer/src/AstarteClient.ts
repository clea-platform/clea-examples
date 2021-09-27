import axios from "axios";
import { Channel, Socket } from "phoenix";

type AstarteClientEvent = 'socketError' | 'socketClose';

type AstarteClientProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
};

type Config = AstarteClientProps & {
  appEngineUrl: URL;
  socketUrl: URL;
};

type SensorDataParameters = {
  deviceId: string;
  sensorId: string;
};

type Sensors = Record<string, SensorData>

type SensorData = {
  x: number;
  y: number;
  z: number;
  timestamp: string;
};

// Wrap phoenix lib calls in promise for async handling
async function openNewSocketConnection(
  connectionParams: { socketUrl: string; realm: string; token: string },
  onErrorHanlder: () => void,
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
    phoenixSocket.onError(onErrorHanlder);
    phoenixSocket.onClose(onCloseHandler);
    phoenixSocket.onOpen(() => {
      resolve(phoenixSocket);
    });
    phoenixSocket.connect();
  });
}

async function joinChannel(phoenixSocket: Socket, channelString: string): Promise<Channel> {
  return new Promise((resolve, reject) => {
    const channel = phoenixSocket.channel(channelString, {});
    channel
      .join()
      .receive('ok', () => {
        resolve(channel);
      })
      .receive('error', (err: unknown) => {
        reject(err);
      });
  });
}

async function leaveChannel(channel: Channel): Promise<void> {
  return new Promise((resolve, reject) => {
    channel
      .leave()
      .receive('ok', () => {
        resolve();
      })
      .receive('error', (err: unknown) => {
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
      .push('watch', triggerPayload)
      .receive('ok', () => {
        resolve();
      })
      .receive('error', (err: unknown) => {
        reject(err);
      });
  });
}

class AstarteClient {
  config: Config;

  private phoenixSocket: Socket | null;

  private joinedChannels: {
    [roomName: string]: Channel;
  };

  private listeners: {
    [eventName: string]: Array<() => void>;
  };

  constructor({ astarteUrl, realm, token }: AstarteClientProps) {
    const appEngineUrl = new URL("appengine/", astarteUrl);
    const socketUrl = new URL("v1/socket", appEngineUrl);
    socketUrl.protocol = socketUrl.protocol === 'https:' ? 'wss:' : 'ws:';

    this.config = {
      astarteUrl,
      realm,
      token,
      appEngineUrl,
      socketUrl,
    };

    this.phoenixSocket = null;
    this.joinedChannels = {};
    this.listeners = {};
  }

  async getSensors(deviceId: string): Promise<Sensors> {
    const { appEngineUrl, realm, token } = this.config;
    const interfaceName = "ai.clea.examples.Accelerometer";
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
      const sensors: Record<string, SensorData[]> = response.data.data;
      const filteredData: Sensors = {};
      Object.entries(sensors).forEach(([key,values]) => {
        filteredData["/" + key] = values[values.length - 1];
      });
      return filteredData;
    });
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
      this.listeners[eventName] = previousListeners.filter((listener) => listener !== callback);
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
          this.dispatch('socketError');
        },
        () => {
          this.dispatch('socketClose');
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
      joinChannel(phoenixSocket, `rooms:${this.config.realm}:${roomName}`).then((joinedChannel) => {
        this.joinedChannels[roomName] = joinedChannel;
        resolve(joinedChannel);
      });
    });
  }

  async listenForEvents(
    roomName: string,
    eventHandler: (event: any) => void,
  ): Promise<void> {
    const channel = this.joinedChannels[roomName];
    if (!channel) {
      return Promise.reject(new Error("Can't listen for room events before joining it first"));
    }

    channel.on('new_event', (jsonEvent: unknown) => {
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
      return Promise.reject(new Error("Room not joined, couldn't register trigger"));
    }

    return registerTrigger(channel, triggerPayload);
  }

  async leaveRoom(roomName: string): Promise<void> {
    const channel = this.joinedChannels[roomName];
    if (!channel) {
      return Promise.reject(new Error("Can't leave a room without joining it first"));
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
}

export default AstarteClient;
export type {AstarteClientEvent, Sensors, SensorData};
