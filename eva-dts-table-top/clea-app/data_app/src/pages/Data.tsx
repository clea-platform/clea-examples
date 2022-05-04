import React, { useEffect, useState } from "react";
import moment from "moment";
import { maxBy } from "lodash";
import { FormattedMessage, useIntl } from "react-intl";

import AstarteClient from "../AstarteClient";
import Card from "../components/common/Card";
import Table from "../components/common/Table";
import Banner from "../components/Banner";
import Filter, { BeverageData } from "../components/Filter";

import "./Data.css";

const colors = ["red", "blue", "yellow", "green", "pink", "cyan", "orange", "lime", "grape", "indigo", "teal", "violet"];
const pickColor = (idx: number) => {
  return colors[idx % colors.length];
};

type DataProps = {
  deviceId: string;
  astarteClient: AstarteClient;
};

const Data: React.FC<DataProps> = ({ deviceId, astarteClient }: DataProps) => {
  const intl = useIntl();

  const beverages = ["mac", "esp", "cap", "cho", "the"];
  const beverageNames = new Map([
    ["mac", intl.formatMessage({ id: "beverages_full.mac" })],
    ["esp", intl.formatMessage({ id: "beverages_full.esp" })],
    ["cap", intl.formatMessage({ id: "beverages_full.cap" })],
    ["cho", intl.formatMessage({ id: "beverages_full.cho" })],
    ["the", intl.formatMessage({ id: "beverages_full.the" })],
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
    // Get beverage data only once
    getBeverageData();
  }, []);

  const getBeverageData = async () => {
    let fetchedData: BeverageData[] = [];
    let globals: { beverage_name: string; total_units: number; total_revenues: number }[] = [];
    await Promise.all(
      beverages.map(async (name, idx) => {
        const [units, revenues] = await Promise.all([
          astarteClient.getUnitsData({ deviceId, beverageId: name }),
          astarteClient.getRevenuesData({ deviceId, beverageId: name }),
        ]);
        // console.log(units);
        // console.log(revenues);

        fetchedData.push({ beverage_name: name, color: pickColor(idx), units, revenues });
        globals.push({
          beverage_name: name,
          total_units: units.reduce((acc, unitData) => acc + unitData.value, 0),
          total_revenues: revenues.reduce((acc, revenueData) => acc + revenueData.value, 0),
        });
      })
    );

    // Set overall data
    setData(fetchedData);
    // console.log(fetchedData);

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
