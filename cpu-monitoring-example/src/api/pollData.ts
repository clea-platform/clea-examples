import AstarteAPIClient from "./AstarteAPIClient";

interface PollingInterface {
  interfaceName: string;
  fetchData: (
    client: AstarteAPIClient,
    deviceId: string,
    since: Date,
  ) => Promise<any>;
  onData: (data: any) => void;
}

interface PollDataOptions {
  deviceId: string;
  astarteClient: AstarteAPIClient;
  interfaces: PollingInterface[];
  since: Date;
  setLoading?: (loading: boolean) => void;
}

export const pollData = ({
  deviceId,
  astarteClient,
  interfaces,
  since,
  setLoading,
}: PollDataOptions): Promise<void> => {
  setLoading?.(true);

  return Promise.all(
    interfaces.map((iface) =>
      iface
        .fetchData(astarteClient, deviceId, since)
        .then((data) => iface.onData(data)),
    ),
  )
    .then(() => {})
    .catch((error) => {
      console.error("Error in pollData:", error);
    })
    .finally(() => {
      setLoading?.(false);
    });
};
