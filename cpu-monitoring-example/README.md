# Cpu Monitoring Example App


This is a React-based dashboard that monitors metrics from devices via [Astarte](https://github.com/astarte-platform/astarte). The app supports polling and WebSocket subscriptions to interfaces like `com.example.poc.CpuTemp` and `com.example.poc.CpuMetrics`.

---

## 🔄 Modifying the App for Different Interfaces

You can easily adapt this app to monitor **different interfaces** by updating a few key files.

---

## ✅ Step-by-Step Instructions

### 1. Add Interface Fetch Logic in `AstarteAPIClient.ts`

Create a method that fetches historical data for the new interface. Here's an example for a MemoryUsage interface:

```ts
async getMemoryUsage({ deviceId, since, to, limit = REQUEST_SAMPLE_LIMIT }: DatastreamDataParameters): Promise<DataPoint[]> {
  // Build the request URL and fetch logic here...
}
```

### 2. Register the Interface in `AstarteInterfaces.ts`

Edit the `polling` and `websocket` arrays to add the new interface:

`Polling` example:
```ts
{
  interfaceName: "com.example.poc.MemoryUsage",
  fetchData: (client, deviceId, since) =>
    client.getMemoryUsage({ deviceId, since }),
  onData: setMemoryUsageData,
}

```
`websocket` example:
```ts
{
  interfaceName: "com.example.poc.MemoryUsage",
  interfaceMajor: 0,
  onEvent: (event) => {
    const newData = {
      value: event.event.value,
      timestamp: event.timestamp,
    };
    setMemoryUsageData((prev) => [...prev, newData]);
  },
}

```
### 3. Add State Hook in `App.tsx`

```ts 
const [memoryUsageData, setMemoryUsageData] = useState<DataPoint[]>([]);
```
Add to interfaces() usage

```ts
const { polling, websocket } = interfaces(
  setCpuTempData,
  setCpuMetricsData,
  setMemoryUsageData // New one
);
```

### 4. Display the data in `App.tsx`
Finally, render a chart (or any component) to display the data. Example:

```ts 
<Col md={6}>
  <DataChart data={memoryUsageData} title="Memory Usage (MB)" />
</Col>

```



