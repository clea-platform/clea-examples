import React from "react";
import { useIntl } from "react-intl";

import TableCard, {TableRow, TableTab} from "../components/TableCard";

import { TransactionData, BeverageFromLongToShort, GenderFromLongToShort, EmotionFromLongToShort } from "../types";
import GenderChartCard from "../components/chartCards/GenderChartCard";
import moment from "moment";


const allBeverages = Object.keys(BeverageFromLongToShort);
const ageIntervals = ["10-19", "20-29", "30-39", "40-49", "50-59", "60-69", "70-79", "80-89", "90-99"];
const emotions = Object.keys(EmotionFromLongToShort);
const genders = Object.keys(GenderFromLongToShort);

const tabsTable: TableTab[] = [
    {
      dataField: 'gender',
      text: 'Gender'
    },
    {
      dataField: 'today',
      text: "Today's count"
    },
    {
      dataField: 'popularEmotion',
      text: 'Popular Emotion',
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

const Gender: React.FC<PageProps> = ({transactions}) => {
    const intl = useIntl();


    const formerTable: { [key: string]: any } = {}
    genders.forEach((gender) => {
        formerTable[GenderFromLongToShort[gender]] = {
            gender,
            today: transactions.filter(t => {
              return (
                t.gender===GenderFromLongToShort[gender]
                && allBeverages.includes(t.choice)
                && moment.unix(t.timestamp).isSame(new Date(), "day")
                )
              }).length,
            popularEmotion: new Array(emotions.length).fill(0),
            popularAge: new Array(ageIntervals.length).fill(0),
            popularBeverage: new Array(allBeverages.length).fill(0),
        }
    })
    for(let transaction of transactions) {
      if (Object.keys(formerTable).includes(transaction.gender) && allBeverages.includes(transaction.choice)){
        formerTable[transaction.gender].popularEmotion[emotions.indexOf(transaction.emotion)] += 1
        formerTable[transaction.gender].popularAge[Math.floor(transaction.age/10)%9] += 1
        formerTable[transaction.gender].popularBeverage[allBeverages.indexOf(transaction.choice)] += 1
      }
    }


    let rowTable: TableRow;
    const rowsTable: TableRow[] = Object.values(formerTable).map( (former, index) => {
      rowTable = {
          gender: former.gender,
          today: former.today.toFixed(0),
          popularEmotion: former.today ? emotions[former.popularEmotion.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
          popularAge: former.today ? ageIntervals[former.popularAge.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
          popularBeverage: former.today ? allBeverages[former.popularBeverage.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
      }
      return rowTable
    })
    

    return (
        <>
          <div className="row mb-3">
            { transactions.length ? <GenderChartCard data={transactions}/> : intl.formatMessage({ id: "loading" }) }
          </div>
          <div className="row">
            { transactions.length ? <TableCard tabs={tabsTable} rows={rowsTable} caption="Gender Overview" /> : intl.formatMessage({ id: "loading" })}
          </div>
        </>
    )

}

export default Gender;