import React from "react";

import { Chart as ChartJS, CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend, BarElement} from "chart.js";
import { Bar } from "react-chartjs-2";
import ChartDataLabels from 'chartjs-plugin-datalabels';

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend, BarElement, ChartDataLabels);


export type DataPoint = {
    label: string;
    value: number;
  };
export type Dataset = {
    name: string;
    color: string;
    points: DataPoint[];
};
type ChartProps = {
    datasets: Dataset[];
    measure?: string;
    legend?: boolean;
};


const BarChart: React.FC<ChartProps> = ( { datasets, measure="", legend=true } ) => {

    const labels = datasets[0].points.map((point) => point.label);
    const data = {
        labels,
        datasets: datasets.map((dataset) => {
        return {
            label: dataset.name,
            backgroundColor: dataset.color,
            data: dataset.points.map((point) => point.value),
        };
        }),
    };

    const options: any = {
        responsive: true,
        plugins: {
            legend: {
              display: legend
            },
            datalabels: {
              display: true,
              color: "black",
              align: "end",
              anchor: "end",
              clamp: true,
              clip: false,
              padding: {
                right: 2
              },
              labels: {
                padding: { top: 10 },
                title: {
                  font: {
                    weight: "bold"
                  }
                },
                value: {
                  color: "green"
                }
              },
              formatter: function (value: string) {
                return "\n" + value;
              }
            }
        },
        scales: {
            units: {
                display: true,
                grace: '10%',
                position: "left" as const,
                ticks: {
                  callback: (val: any) => {
                    return val + measure;
                  },
                },
            },
        }
      };

    return (
        <>
            <Bar options={options} data={data} />
        </>
    )
}

export default BarChart