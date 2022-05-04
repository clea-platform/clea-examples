import React, { useMemo } from "react";

import Data from "./pages/Data";

import AstarteClient from "./AstarteClient";

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

  return <Data deviceId={deviceId} astarteClient={astarteClient} />;
};

export default App;
