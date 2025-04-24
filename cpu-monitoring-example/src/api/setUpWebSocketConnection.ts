import AstarteAPIClient from "./AstarteAPIClient";

interface WebSocketInterface {
  interfaceName: string;
  interfaceMajor: number;
  onEvent: (event: any) => void;
}

interface WebSocketOptions {
  deviceId: string;
  astarteClient: AstarteAPIClient;
  interfaces: WebSocketInterface[];
}

export const setUpWebSocketConnection = async ({
  deviceId,
  astarteClient,
  interfaces,
}: WebSocketOptions) => {
  await Promise.all(
    interfaces.map(async ({ interfaceName, interfaceMajor, onEvent }) => {
      const salt = Math.random().toString(36).slice(2);
      const roomName = `${salt}:devices:${deviceId}:interfaces:${interfaceName}`;

      try {
        await astarteClient.joinRoom(roomName);

        await astarteClient.listenForEvents(roomName, (event: any) => {
          if (event.event.interface === interfaceName) {
            onEvent(event);
          }
        });

        const trigger = {
          name: `trigger-${deviceId}-${interfaceName.replace(/\./g, "-")}`,
          device_id: deviceId,
          simple_trigger: {
            type: "data_trigger",
            on: "incoming_data",
            interface_name: interfaceName,
            interface_major: interfaceMajor,
            match_path: "/*",
            value_match_operator: "*",
          },
        } as const;

        await astarteClient.registerVolatileTrigger(roomName, trigger);
        console.log(`Subscribed to ${interfaceName}`);
      } catch (error) {
        console.error(`WebSocket setup failed for ${interfaceName}:`, error);
      }
    }),
  );
};
