import React, { useEffect, useState } from "react";
import moment from "moment";
import { maxBy, sortBy } from "lodash";
import { FormattedMessage, useIntl } from "react-intl";

import AstarteClient from "../AstarteClient";
import Card from "../components/common/Card";
import Table from "../components/common/Table";
import Banner from "../components/Banner";
import Filter, { BeverageData } from "../components/Filter";

import "./Data.css";

//TODO: this can be removed eventually
// const colors = ["red", "blue", "yellow", "green", "pink", "cyan", "orange", "lime", "grape", "indigo", "teal", "violet"];
// const pickColor = (idx: number) => {
//   return colors[idx % colors.length];
// };

type DataProps = {
  deviceId: string;
  astarteClient: AstarteClient;
};

const Data: React.FC<DataProps> = ({ deviceId, astarteClient }: DataProps) => {
  const intl = useIntl();

  const beverages = [
    {
      name: "2x_espresso",
      color: "red",
    },
    {
      name: "espresso",
      color: "blue",
    },
    {
      name: "kaffee_creme",
      color: "yellow",
    },
    {
      name: "2x_kaffee_creme",
      color: "green",
    },
    {
      name: "americano_large",
      color: "pink",
    },
    {
      name: "cappuccino",
      color: "cyan",
    },
    {
      name: "latte_macchiato",
      color: "orange",
    },
    {
      name: "hot_water",
      color: "lime",
    },
  ];

  const beverageNames = new Map([
    ["2x_espresso", intl.formatMessage({ id: "beverages_full.2x_espresso" })],
    ["espresso", intl.formatMessage({ id: "beverages_full.espresso" })],
    ["kaffee_creme", intl.formatMessage({ id: "beverages_full.kaffee_creme" })],
    ["2x_kaffee_creme", intl.formatMessage({ id: "beverages_full.2x_kaffee_creme" })],
    ["americano_large", intl.formatMessage({ id: "beverages_full.americano_large" })],
    ["cappuccino", intl.formatMessage({ id: "beverages_full.cappuccino" })],
    ["latte_macchiato", intl.formatMessage({ id: "beverages_full.latte_macchiato" })],
    ["hot_water", intl.formatMessage({ id: "beverages_full.hot_water" })],
  ]);
  const getBeverageFullName = (bev: string): string | undefined => {
    return beverageNames.get(bev);
  };

  const [data, setData] = useState<BeverageData[]>([]);
  const [productsOverview, setProductsOverview] = useState<string[][]>([]);
  const [totalUnits, setTotalUnits] = useState<number>();
  const [totalRevenues, setTotalRevenues] = useState<number>();
  const [mostServed, setMostServed] = useState<string>();

  const [csvFileName, setCsvFileName] = useState<string>();
  const [csvFileData, setCsvFileData] = useState<string>();
  const setCsvDownloadLink = (fileName: string, fileData: string) => {
    setCsvFileName(fileName);
    setCsvFileData(fileData);
  };

  useEffect(() => {
    const fetchIntervalTime = 5000; // ms

    // Get data immediately the first time
    getBeverageData();

    // Start fetching every N milliseconds
    console.log("Start beverage data interval");
    const beverageInterval = setInterval(() => getBeverageData(), fetchIntervalTime);

    // Clear interval
    return () => {
      clearInterval(beverageInterval);
    };
    // eslint-disable-next-line
  }, []);

  // useEffect(() => {
  //   // Get beverage data only once
  //   getBeverageData();
  // }, []);

  const getBeverageData = async () => {
    let fetchedData: BeverageData[] = [];
    let globals: { beverage_name: string; total_units: number; total_revenues: number }[] = [];
    await Promise.all(
      beverages.map(async (bev) => {
        const [units, revenues] = await Promise.all([
          astarteClient.getUnitsData({ deviceId, beverageId: bev.name }),
          astarteClient.getRevenuesData({ deviceId, beverageId: bev.name }),
        ]);
        // console.log(units);
        // console.log(revenues);

        fetchedData.push({ beverage_name: bev.name, color: bev.color, units, revenues });
        globals.push({
          beverage_name: bev.name,
          total_units: units.reduce((acc, unitData) => acc + unitData.value, 0),
          total_revenues: revenues.reduce((acc, revenueData) => acc + revenueData.value, 0),
        });
      })
    );

    // Sort and set overall data
    fetchedData = sortBy(fetchedData, ["beverage_name"]);
    setData(fetchedData);

    // Set global cards values
    const totalUnits = globals.reduce((acc, data) => acc + data.total_units, 0);
    const totalRevenues = globals.reduce((acc, data) => acc + data.total_revenues, 0);
    const mostServed = getBeverageFullName(maxBy(globals, "total_units")?.beverage_name!);
    // console.log(globals);
    // console.log(productsOverview);
    // console.log(totalUnits, totalRevenues, mostServed);
    setTotalUnits(totalUnits);
    setTotalRevenues(totalRevenues);
    setMostServed(mostServed);

    // Set products overview
    let productsOverview: string[][] = [];
    globals.map((data) =>
      productsOverview.push([
        getBeverageFullName(data.beverage_name)!,
        data.total_units.toString(),
        data.total_revenues.toFixed(1) + "€",
        (data.total_revenues / data.total_units).toFixed(1) + "€",
        moment().format("DD/MM/YYYY hh:mm"),
      ])
    );
    setProductsOverview(productsOverview);
  };

  return (
    <div className="container-fluid">
      {/* Global Counters */}
      <div className="row mt-3">
        <div className="col">
          <Banner
            subtitle={intl.formatMessage({ id: "real_time_counter" })}
            title={intl.formatMessage({ id: "beverages" })}
            value={totalUnits ? intl.formatNumber(totalUnits) : intl.formatMessage({ id: "loading" })}
          />
        </div>
        <div className="col">
          <Banner
            subtitle={intl.formatMessage({ id: "real_time" })}
            title={intl.formatMessage({ id: "revenues" })}
            value={totalRevenues ? intl.formatNumber(totalRevenues) + "€" : intl.formatMessage({ id: "loading" })}
          />
        </div>
        <div className="col">
          <Banner
            subtitle={intl.formatMessage({ id: "most_served" })}
            title={intl.formatMessage({ id: "beverage" })}
            value={mostServed ? mostServed : intl.formatMessage({ id: "loading" })}
          />
        </div>
      </div>

      {/* Filter and Data Graph */}
      <div className="row mt-3">
        <div className="col">
          <Card className="filter-container-card">
            {/* TODO: find out what this is all about */}
            <div className="d-grid justify-content-end pb-3">
              <a href={csvFileData} download={csvFileName} className="btn btn-primary fw-bold" role="button">
                <i className="bi bi-filetype-csv me-2"></i>
                <FormattedMessage id="download" />
              </a>
            </div>

            {data.length ? <Filter data={data} setDownloadLink={setCsvDownloadLink} /> : intl.formatMessage({ id: "loading" })}
          </Card>
        </div>
      </div>

      {/* Products Overview */}
      <div className="row mt-3">
        <div className="col">
          <Card title={intl.formatMessage({ id: "tab_title" })} className="products-table-card">
            <Table
              tabs={[
                intl.formatMessage({ id: "tab_header.beverage" }),
                intl.formatMessage({ id: "tab_header.total_units" }),
                intl.formatMessage({ id: "tab_header.total_revenues" }),
                intl.formatMessage({ id: "tab_header.price" }),
                intl.formatMessage({ id: "tab_header.last_update" }),
              ]}
              rows={productsOverview}
            />
          </Card>
        </div>
      </div>
    </div>
  );
};

export default Data;
