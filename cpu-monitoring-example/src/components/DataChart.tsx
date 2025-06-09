import { useMemo } from "react";
import { ApexOptions } from "apexcharts";
import Chart from "react-apexcharts";

import { DataPoint } from "../api/AstarteAPIClient";

type DataChartProps = {
  data: DataPoint[];
  title: string;
};

const DataChart = ({ data, title }: DataChartProps) => {
  const series = useMemo(() => {
    const dataSeries = data.map((element) => ({
      x: element.timestamp,
      y: element.value,
    }));
    return [{ name: title, data: dataSeries }];
  }, [data]);

  const options: ApexOptions = useMemo(
    () => ({
      chart: {
        type: "line",
        toolbar: {
          show: false,
        },
        animations: {
          enabled: false,
        },
      },
      title: {
        text: title,
        align: "center",
        offsetY: 20,
      },
      xaxis: {
        type: "datetime",
        labels: { datetimeUTC: false },
      },
      yaxis: {
        min: 0,
        forceNiceScale: true,
        labels: {
          formatter: (value) => value.toFixed(2),
        },
      },
      tooltip: {
        x: {
          show: true,
          format: "dd/MM/yy HH:mm:ss",
        },
      },
      stroke: {
        curve: "straight",
        width: 1,
      },
      grid: {
        xaxis: {
          lines: {
            show: true,
          },
        },
        yaxis: {
          lines: {
            show: true,
          },
        },
      },
    }),
    [],
  );

  return <Chart options={options} series={series} />;
};

export default DataChart;
