import React, { useEffect, useRef, useState } from "react";
import moment from "moment";
import { FormattedMessage, useIntl } from "react-intl";
// @ts-ignore
import DateRangePicker from "@wojtekmaj/react-daterange-picker";

import BarChart, { Dataset } from "./BarChart";

import "./Filter.css";

import { groupByDay, groupByMonth, groupByHour } from "../utils/aggregate";
import { getCsvString, CsvData, CsvFile } from "../utils/csv";
import { DataType, RevenuesData, UnitsData } from "../types";

// The start and end date with Unix timestamp
type DateRange = {
  start: number;
  end: number;
};

enum DateGranularity {
  DAY = "day",
  WEEK = "week",
  MONTH = "month",
  YEAR = "year",
}
const inferDateGranularity = (start: moment.Moment, end: moment.Moment): DateGranularity => {
  const diffInDays = Math.abs(end.diff(start, "days"));
  if (diffInDays === 0) {
    return DateGranularity.DAY;
  }
  if (diffInDays < 7) {
    return DateGranularity.WEEK;
  }
  if (diffInDays < 31) {
    return DateGranularity.MONTH;
  }
  return DateGranularity.YEAR;
};

const getDateRange = (dateType: DateGranularity): DateRange => {
  switch (dateType) {
    case DateGranularity.DAY:
      return { start: moment().startOf("day").unix(), end: moment().endOf("day").unix() };
    case DateGranularity.WEEK:
      return { start: moment().subtract(6, "days").startOf("day").unix(), end: moment().endOf("day").unix() };
    case DateGranularity.MONTH:
      return { start: moment().subtract(30, "days").startOf("day").unix(), end: moment().endOf("day").unix() };
    case DateGranularity.YEAR:
      return { start: moment().subtract(364, "days").startOf("day").unix(), end: moment().endOf("day").unix() };
  }
};

export type BeverageData = {
  beverage_name: string;
  color: string;
  units: UnitsData[];
  revenues: RevenuesData[];
};

type FilterProps = {
  data: BeverageData[];
  setDownloadLink: (fileName: string, fileData: string) => void;
};

const Filter: React.FC<FilterProps> = ({ data, setDownloadLink }) => {
  const intl = useIntl();

  const beverages = data.map((bevData) => bevData.beverage_name);
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
  const getBeverageShortName = (bev: string): string | undefined => {
    return beverageNames.get(bev);
  };

  const [currentBeverages, setCurrentBeverages] = useState<string[]>(beverages);
  const [currentDataType, setCurrentDataType] = useState<DataType>(DataType.UNITS);
  // Set by default the whole current day
  const [dateRange, setDateRange] = useState<DateRange>(getDateRange(DateGranularity.DAY));
  const [dateGranularity, setDateGranularity] = useState<DateGranularity>(DateGranularity.DAY);
  const customDateRange = useRef<boolean>(false);

  const [datasets, setDatasets] = useState<Dataset[]>([]);

  // Trigger filtering on new Beverage or DateRange change
  useEffect(() => {
    updateDatasets(data);
    const csvFile = createCsvFile(data);
    setDownloadLink(csvFile.filename, "data:text/plain;charset=utf-8," + encodeURIComponent(getCsvString(csvFile.data)));
  }, [data, currentBeverages, currentDataType, dateRange, dateGranularity]);

  const updateDatasets = (data: BeverageData[]) => {
    var newDatasets: Dataset[] = [];

    data.forEach((beverageData) => {
      // Filter on beverage name
      if (!currentBeverages.includes(beverageData.beverage_name)) {
        // console.log("Filtering out by beverage name", beverageData.beverage);
        return;
      }

      // Get data based on type
      const dataToFilter: UnitsData[] | RevenuesData[] = currentDataType === DataType.UNITS ? beverageData.units : beverageData.revenues;

      // Filter on date range
      const filteredData = dataToFilter.filter((item) => item.timestamp >= dateRange.start && item.timestamp <= dateRange.end);

      // Create the aggregates
      const aggregateHour = groupByHour(filteredData, "timestamp", "value");
      const aggregateDay = groupByDay(filteredData, "timestamp", "value");
      const aggregateMonth = groupByMonth(filteredData, "timestamp", "value");

      // Construct the dataset based on the granularity selected
      let finalData: any[] = [];
      switch (dateGranularity) {
        case DateGranularity.DAY:
          finalData = Object.entries(aggregateHour).map(([key, value]) => {
            return {
              label: moment.unix(parseInt(key)).format("HH - DD/MM - dd"),
              value: value,
            };
          });
          break;
        case DateGranularity.WEEK:
          finalData = Object.entries(aggregateDay).map(([key, value]) => {
            return {
              label: moment.unix(parseInt(key)).format("DD/MM - dddd"),
              value: value,
            };
          });
          break;
        case DateGranularity.MONTH:
          finalData = Object.entries(aggregateDay).map(([key, value]) => {
            return {
              label: moment.unix(parseInt(key)).format("DD - MMMM"),
              value: value,
            };
          });
          break;
        case DateGranularity.YEAR:
          finalData = Object.entries(aggregateMonth).map(([key, value]) => {
            return {
              label: moment.unix(parseInt(key)).format("MMMM - YYYY"),
              value: value,
            };
          });
          break;
      }

      newDatasets.push({ name: beverageData.beverage_name, color: beverageData.color, points: finalData });
    });

    setDatasets(newDatasets);
  };

  const createCsvFile = (data: BeverageData[]): CsvFile => {
    let csvData: CsvData[] = [];

    data.forEach((beverageData) => {
      // Filter on beverage name
      if (!currentBeverages.includes(beverageData.beverage_name)) {
        console.log("Filtering out by beverage name", beverageData.beverage_name);
        return;
      }
      // Get data based on type
      const dataToFilter: UnitsData[] | RevenuesData[] = currentDataType === DataType.UNITS ? beverageData.units : beverageData.revenues;
      // Filter on date range
      const filteredData = dataToFilter.filter((item) => item.timestamp >= dateRange.start && item.timestamp <= dateRange.end);

      filteredData.forEach((data) => {
        csvData.push({
          beverage_name: beverageData.beverage_name,
          datetime: moment.unix(data.timestamp).format("YYYY-MM-DDThh:mm:ss"),
          value: data.value,
        });
      });
    });

    const getFileName = (dataType: DataType): string => {
      let filename = dataType === DataType.UNITS ? intl.formatMessage({ id: "units" }) : intl.formatMessage({ id: "revenues" });
      filename += "_" + moment.unix(dateRange.start).format("DD-MM-YYYY") + "_" + moment.unix(dateRange.end).format("DD-MM-YYYY");
      filename += ".csv";
      return filename;
    };

    return {
      filename: getFileName(currentDataType),
      data: csvData,
    };
  };

  return (
    <>
      <div className="btn-toolbar justify-content-between filter-toolbar" role="toolbar" aria-label="Toolbar for filtering data">
        {/* Data Filter */}
        <div className="btn-group p-2" role="group" aria-label="Data type group">
          <input
            type="radio"
            className="btn-check"
            name="data_type"
            id="units"
            autoComplete="off"
            checked={currentDataType === DataType.UNITS}
            onChange={() => setCurrentDataType(DataType.UNITS)}
          />
          <label className="btn btn-outline-primary" htmlFor="units">
            <FormattedMessage id="units" />
          </label>

          <input
            type="radio"
            className="btn-check"
            name="data_type"
            id="revenues"
            autoComplete="off"
            checked={currentDataType === DataType.REVENUES}
            onChange={() => setCurrentDataType(DataType.REVENUES)}
          />
          <label className="btn btn-outline-primary" htmlFor="revenues">
            <FormattedMessage id="revenues" />
          </label>
        </div>

        {/* Beverage Filter */}
        <div className="btn-group p-2" role="group" aria-label="Beverage type group">
          <input
            type="checkbox"
            className="btn-check"
            id="all-check"
            autoComplete="off"
            checked={currentBeverages.length === beverages.length}
            onChange={() => {
              setCurrentBeverages(beverages);
            }}
          />
          <label className="btn btn-outline-primary" htmlFor="all-check">
            <FormattedMessage id="all" />
          </label>

          {beverages.sort().map((bev) => {
            return (
              <div key={bev}>
                <input
                  type="checkbox"
                  className="btn-check"
                  id={bev}
                  autoComplete="off"
                  checked={currentBeverages.includes(bev)}
                  onChange={() => {
                    setCurrentBeverages(
                      currentBeverages.includes(bev) ? currentBeverages.filter((curBev) => curBev !== bev) : [...currentBeverages, bev]
                    );
                  }}
                />
                <label className="btn btn-outline-primary text-capitalize" htmlFor={bev}>
                  {getBeverageShortName(bev)}
                </label>
              </div>
            );
          })}
        </div>

        {/* Date Granularity Filter */}
        <div className="btn-group p-2" role="group" aria-label="Date granularity group">
          <input
            type="radio"
            className="btn-check"
            name="date_granularity"
            id="day"
            autoComplete="off"
            checked={dateGranularity === DateGranularity.DAY && !customDateRange.current}
            onChange={() => {
              customDateRange.current = false;
              setDateRange(getDateRange(DateGranularity.DAY));
              setDateGranularity(DateGranularity.DAY);
            }}
          />
          <label className="btn btn-outline-primary" htmlFor="day">
            <FormattedMessage id="day" />
          </label>
          <input
            type="radio"
            className="btn-check"
            name="date_granularity"
            id="week"
            autoComplete="off"
            checked={dateGranularity === DateGranularity.WEEK && !customDateRange.current}
            onChange={() => {
              customDateRange.current = false;
              setDateRange(getDateRange(DateGranularity.WEEK));
              setDateGranularity(DateGranularity.WEEK);
            }}
          />
          <label className="btn btn-outline-primary" htmlFor="week">
            <FormattedMessage id="week" />
          </label>
          <input
            type="radio"
            className="btn-check"
            name="date_granularity"
            id="month"
            autoComplete="off"
            checked={dateGranularity === DateGranularity.MONTH && !customDateRange.current}
            onChange={() => {
              customDateRange.current = false;
              setDateRange(getDateRange(DateGranularity.MONTH));
              setDateGranularity(DateGranularity.MONTH);
            }}
          />
          <label className="btn btn-outline-primary" htmlFor="month">
            <FormattedMessage id="month" />
          </label>
          <input
            type="radio"
            className="btn-check"
            name="date_granularity"
            id="year"
            autoComplete="off"
            checked={dateGranularity === DateGranularity.YEAR && !customDateRange.current}
            onChange={() => {
              customDateRange.current = false;
              setDateRange(getDateRange(DateGranularity.YEAR));
              setDateGranularity(DateGranularity.YEAR);
            }}
          />
          <label className="btn btn-outline-primary" htmlFor="year">
            <FormattedMessage id="year" />
          </label>

          <DateRangePicker
            value={[moment.unix(dateRange.start).toDate(), moment.unix(dateRange.end).toDate()]}
            onChange={(range: any) => {
              if (range) {
                customDateRange.current = true;
                setDateRange({ start: moment(range[0]).unix(), end: moment(range[1]).unix() });
                setDateGranularity(inferDateGranularity(moment(range[0]), moment(range[1])));
              } else {
                customDateRange.current = false;
                setDateRange(getDateRange(DateGranularity.DAY));
                setDateGranularity(DateGranularity.DAY);
              }
            }}
          />
        </div>
      </div>

      {/* Chart plot */}
      {datasets.length && <BarChart datasets={datasets} dataType={currentDataType} />}
    </>
  );
};

export default Filter;
