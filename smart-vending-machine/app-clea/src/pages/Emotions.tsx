import React from "react";
import { useIntl } from "react-intl";

import TableCard, {TableRow, TableTab} from "../components/TableCard";

import { TransactionData, BeverageFromLongToShort, GenderFromLongToShort, GenderFromShortToLong, EmotionFromLongToShort } from "../types";
import EmotionsChartCard from "../components/chartCards/EmotionsChartCard";
import moment from "moment";


const allBeverages = Object.keys(BeverageFromLongToShort);
const ageIntervals = ["10-19", "20-29", "30-39", "40-49", "50-59", "60-69", "70-79", "80-89", "90-99"];
const emotions = Object.keys(EmotionFromLongToShort);
const genders = Object.keys(GenderFromLongToShort);

const tabsTable: TableTab[] = [
    {
      dataField: 'emotion',
      text: 'Emotions'
    },
    {
      dataField: 'today',
      text: "Today's count"
    },
    {
      dataField: 'popularGender',
      text: 'Popular Gender',
    },
    {
      dataField: 'popularAge',
      text: 'Popular Age'
    },
    {
      dataField: 'popularBeverage',
      text: 'Popular Beverage'
    },
]

type PageProps= {
    transactions: TransactionData[]
}

const Emotions: React.FC<PageProps> = ({transactions}) => {
    const intl = useIntl();

    const formerTable: { [key: string]: any } = {}
    emotions.forEach((emotion) => {
        formerTable[emotion] = {
            emotion,
            today: transactions.filter(t => {
              return (
                t.emotion===emotion
                && allBeverages.includes(t.choice)
                && moment.unix(t.timestamp).isSame(new Date(), "day")
                )
              }).length,
            popularGender: new Array(genders.length).fill(0),
            popularAge: new Array(ageIntervals.length).fill(0),
            popularBeverage: new Array(allBeverages.length).fill(0),
        }
    })

    for(let transaction of transactions) {
        if(emotions.includes(transaction.emotion)){
            formerTable[transaction.emotion].popularGender[genders.indexOf(GenderFromShortToLong[transaction.gender])] += 1
            formerTable[transaction.emotion].popularAge[Math.floor(transaction.age/10)%9] += 1
            formerTable[transaction.emotion].popularBeverage[allBeverages.indexOf(transaction.choice)] += 1
        }
    }

    let rowTable: TableRow;
    const rowsTable: TableRow[] = Object.values(formerTable).map( (former, index) => {
      rowTable = {
          emotion: former.emotion,
          today: former.today.toFixed(0),
          popularGender: former.today ? genders[former.popularGender.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
          popularAge: former.today ? ageIntervals[former.popularAge.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
          popularBeverage: former.today ? allBeverages[former.popularBeverage.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
      }
      return rowTable
    })

    return(
        <>
            <div className="row mb-3">
              { transactions.length ? <EmotionsChartCard data={transactions}/> : intl.formatMessage({ id: "loading" }) }
            </div>
            <div className="row">
              { transactions.length ? <TableCard tabs={tabsTable} rows={rowsTable} caption="Emotions Overview" /> : intl.formatMessage({ id: "loading" }) }
            </div>
        </>
    )

}

export default Emotions;