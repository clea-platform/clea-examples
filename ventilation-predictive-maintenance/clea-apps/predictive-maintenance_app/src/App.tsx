import React from "react";

import PredictiveMaintenance from "./pages/PredictiveMaintenance";

import AstarteClient from "./AstarteClient";

const { useMemo } = React;

export type AppProps = {
  astarteUrl: URL;
  realm: string;
  token: string;
  deviceId: string;
};

const App = ({ astarteUrl, realm, token, deviceId }: AppProps) => {
  const astarteClient = useMemo(() => {
    return new AstarteClient({ astarteUrl, realm, token });
  }, [astarteUrl, realm, token]);

  return <PredictiveMaintenance thresholds={{ warning: 0.7, danger: 0.5 }} deviceId={deviceId} astarteClient={astarteClient} />;
};

export default App;
