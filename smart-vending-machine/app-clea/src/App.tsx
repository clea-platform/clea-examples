import React, { useMemo, useState, useEffect } from "react";
import { HashRouter, Routes, Route } from "react-router-dom"

import Overview from "./pages/Overview";
import Revenues from "./pages/Revenues";
import Gender from "./pages/Gender";
import Emotions from "./pages/Emotions";
import SideMenu from "./components/SideMenu";

import AstarteClient from "./AstarteClient";
import { TransactionData } from "./types";
import Age from "./pages/Age";

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

  const [transactions, setTransactions] = useState<TransactionData[]>([]);

  const routes = [
    {
      path: "/",
      exact: true,
      sidebar: "Overview",
      main: <Overview transactions={transactions} />
    },
    {
      path: "/revenues",
      sidebar: "Revenues",
      main: <Revenues transactions={transactions} />
    },
    {
      path: "/gender",
      sidebar: "Gender",
      main: <Gender transactions={transactions} />
    },
    {
      path: "/age",
      sidebar: "Age",
      main: <Age transactions={transactions} />
    },
    {
      path: "/emotions",
      sidebar: "Emotions",
      main: <Emotions transactions={transactions} />
    },
  ];
  const titlesSideBar: Array<string> = []
  const paths: Array<string> = []
  routes.forEach((route) => {
    titlesSideBar.push(route.sidebar);
    paths.push(route.path);
  });

  useEffect(() => {
    getTransactions();

    const t = setInterval(getTransactions, 10000);
    return () => clearInterval(t); // clear
  }, [] );

  const getTransactions = async () => {
    const data = await astarteClient.getTransactionData({deviceId});
    // console.log(data)
    setTransactions(data);
  };

  return (
    <HashRouter>
      <div className="row mt-3">
        <div className='col-3'>
          <SideMenu titles={titlesSideBar} paths={paths}/>
        </div>
        <div className="col me-3">
          <Routes>
            {routes.map((route, index) => (
              <Route
                key={index}
                path={route.path}
                element={route.main}
              />
            ))}
          </Routes>
        </div>
      </div>
    </HashRouter>
  )
};

export default App;