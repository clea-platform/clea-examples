import { useEffect, useMemo, useState } from "react";
import { Card, Col, Container, Row } from "react-bootstrap";

import AstarteAPIClient, { DataPoint } from "./api/AstarteAPIClient";
import { setUpWebSocketConnection } from "./api/setUpWebSocketConnection";
import { pollData } from "./api/pollData";
import { interfaces } from "./api/astarteInterfaces";
import DataChart from "./components/DataChart";
import Loading from "./components/Loading";

export type AppProps = {
  astarteUrl: URL;
  realm: string;
  deviceId: string;
  token: string;
  portal: {
    baseApiUrl: URL;
    token: string;
    tenantSlug: string;
  };
  appliance: {
    id: string;
  };
  viewer: {
    name: string;
    email: string;
  };
};

const App = ({
  astarteUrl,
  realm,
  deviceId,
  token,
  portal,
  appliance,
  viewer,
}: AppProps) => {
  const [dataFetching, setDataFetching] = useState(false);
  const [cpuTempData, setCpuTempData] = useState<DataPoint[]>([]);
  const [cpuMetricsData, setCpuMetricsData] = useState<DataPoint[]>([]);
  const astarteClient = useMemo(() => {
    return new AstarteAPIClient({ astarteUrl, realm, token });
  }, [astarteUrl, realm, token]);

  const since = new Date(Date.now() - 60 * 60 * 1000);

  useEffect(() => {
    const { polling, websocket } = interfaces(
      setCpuTempData,
      setCpuMetricsData,
    );

    pollData({
      deviceId,
      astarteClient,
      interfaces: polling,
      since,
      setLoading: setDataFetching,
    }).then(() => {
      setUpWebSocketConnection({
        deviceId,
        astarteClient,
        interfaces: websocket,
      });
    });
  }, [astarteClient, deviceId]);

  return (
    <Container fluid>
      <Row className="g-1">
        <Card className="text-center my-3">
          <div className="mx-4">
            {dataFetching ? (
              <Loading />
            ) : (
              <Row className="justify-content-center">
                <Col md={6}>
                  <DataChart data={cpuMetricsData} title={"Cpu Load (%)"} />
                </Col>
                <Col md={6}>
                  <DataChart
                    data={cpuTempData}
                    title={"Cpu Temperature (°C)"}
                  />
                </Col>
              </Row>
            )}
          </div>
        </Card>
      </Row>
    </Container>
  );
};

export default App;
