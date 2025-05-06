import { useState } from "react";
import { Card, Container, Row, Spinner } from "react-bootstrap";

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
  const [dataFetching, _] = useState(false);

  return (
    <Container fluid>
      <Row className="g-1">
        <Card className="text-center my-3">
          <div className="mx-4">
            {
              dataFetching ? (
                <div className="p-2 p-md-4 text-center">
                  <Spinner />
                </div>
              ) : (
                "Cpu Monitoring Example App"
              ) // TODO: implement components
            }
          </div>
        </Card>
      </Row>
    </Container>
  );
};

export default App;
