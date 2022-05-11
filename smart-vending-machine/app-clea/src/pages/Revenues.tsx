import React from "react";
import { useIntl } from "react-intl";

import TableCard, {TableRow, TableTab} from "../components/TableCard";
import RevenuesChartCard from "../components/chartCards/RevenuesChartCard";

import { TransactionData, BeverageFromLongToShort, GenderFromLongToShort } from "../types";


const allBeverages = Object.keys(BeverageFromLongToShort);
const ageIntervals = ["10-19", "20-29", "30-39", "40-49", "50-59", "60-69", "70-79", "80-89", "90-99"];
const genders = Object.keys(GenderFromLongToShort);
const tabsTable: TableTab[] = [
    {
      dataField: 'beverage',
      text: 'Beverage'
    },
    {
      dataField: 'ordered',
      text: 'Ordered'
    },
    {
      dataField: 'revenue',
      text: 'Revenue',
    },
    {
      dataField: 'price',
      text: 'Price'
    },
    {
      dataField: 'popularGender',
      text: 'Popular gender'
    },
    {
      dataField: 'popularAge',
      text: 'Popular Age'
    },
]


type RevenuesProps= {
    transactions: TransactionData[]
}

const Revenues: React.FC<RevenuesProps> = ({transactions}) => {
    const intl = useIntl();

    const formerTable: { [key: string]: any } = {}
    allBeverages.forEach((beverage) => {
        formerTable[beverage] = {
            beverage: beverage,
            ordered: 0,
            revenue: 0,
            price: 0,
            popularGender: [0, 0],
            popularAge: [0, 0, 0, 0, 0, 0, 0, 0, 0],
        }
    })
    for(let transaction of transactions) {
      if (allBeverages.includes(transaction.choice)) {
        formerTable[transaction.choice].ordered += 1 
        formerTable[transaction.choice].revenue += transaction.price 
        formerTable[transaction.choice].price = transaction.price
        if (transaction.age) {
          formerTable[transaction.choice].popularGender[transaction.gender==="M" ? 1:0] += 1
          formerTable[transaction.choice].popularAge[Math.floor(transaction.age/10)%9] += 1
        }
      }
    }

    let rowTable: TableRow;
    const rowsTable: TableRow[] = Object.values(formerTable).map( (former, index) => {
        rowTable = {
            beverage: former.beverage,
            ordered: former.ordered.toFixed(0) + " Times",
            revenue: former.revenue.toFixed(2) + "€",
            price: former.ordered ? former.price.toFixed(2) + "€" : "-",
            popularGender: former.ordered ? genders[former.popularGender.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
            popularAge: former.ordered ? ageIntervals[former.popularAge.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
        }
        return rowTable
    })

    return (
        <div>
            <div className="row mb-3">
                { transactions.length ? <RevenuesChartCard data={transactions}/> : intl.formatMessage({ id: "loading" }) }
            </div>
            <div className="row">
                { transactions.length ? <TableCard tabs={tabsTable} rows={rowsTable} caption="Beverage Overview" /> : intl.formatMessage({ id: "loading" }) }
            </div>
        </div>
    );
};

export default Revenues