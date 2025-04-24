import AstarteAPIClient, { DataPoint } from "./AstarteAPIClient";

export const interfaces = (
  setCpuTempData: React.Dispatch<React.SetStateAction<DataPoint[]>>,
  setCpuMetricsData: React.Dispatch<React.SetStateAction<DataPoint[]>>,
) => {
  return {
    polling: [
      {
        interfaceName: "com.example.poc.CpuTemp",
        fetchData: (client: AstarteAPIClient, deviceId: string, since: Date) =>
          client.getCpuTemp({ deviceId: deviceId, since: since }),
        onData: setCpuTempData,
      },
      {
        interfaceName: "com.example.poc.CpuMetrics",
        fetchData: (client: AstarteAPIClient, deviceId: string, since: Date) =>
          client.getCpuMetrics({ deviceId: deviceId, since: since }),
        onData: setCpuMetricsData,
      },
    ],
    websocket: [
      {
        interfaceName: "com.example.poc.CpuTemp",
        interfaceMajor: 0,
        onEvent: (event: any) => {
          const newData: DataPoint = {
            value: event.event.value,
            timestamp: event.timestamp,
          };
          setCpuTempData((prev) => [...prev, newData]);
        },
      },
      {
        interfaceName: "com.example.poc.CpuMetrics",
        interfaceMajor: 0,
        onEvent: (event: any) => {
          const newData: DataPoint = {
            value: event.event.value,
            timestamp: event.timestamp,
          };
          setCpuMetricsData((prev) => [...prev, newData]);
        },
      },
    ],
  };
};
