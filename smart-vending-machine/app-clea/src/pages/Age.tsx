import React from "react";
import { useIntl } from "react-intl";

import TableCard, {TableRow, TableTab} from "../components/TableCard";

import { TransactionData, BeverageFromLongToShort, GenderFromLongToShort, GenderFromShortToLong, EmotionFromLongToShort, ageIntervals } from "../types";
import AgeChartCard from "../components/chartCards/AgeChartCard";
import moment from "moment";


const allBeverages = Object.keys(BeverageFromLongToShort);
const emotions = Object.keys(EmotionFromLongToShort);
const genders = Object.keys(GenderFromLongToShort);

const tabsTable: TableTab[] = [
    {
      dataField: 'age',
      text: 'Age',
      sort: true,
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
      dataField: 'popularEmotion',
      text: 'Popular Emotion'
    },
    {
      dataField: 'popularBeverage',
      text: 'Popular Beverage'
    },
]

type PageProps= {
    transactions: TransactionData[]
}

const Age: React.FC<PageProps> = ({transactions}) => {
    const intl = useIntl();

    const formerTable: { [key: string]: any } = {}

    for(let transaction of transactions) {
      if (transaction.age) {
        if (!formerTable[ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length]]){
            formerTable[ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length]] = {
                age: ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length],
                today: transactions.filter(t => {
                  return (
                    ageIntervals[Math.floor(t.age/10)%ageIntervals.length]===ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length]
                    && allBeverages.includes(t.choice)
                    && moment.unix(t.timestamp).isSame(new Date(), "day")
                  )}).length,
                popularGender: new Array(genders.length).fill(0),
                popularEmotion: new Array(emotions.length).fill(0),
                popularBeverage: new Array(allBeverages.length).fill(0),
            }
        }
        formerTable[ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length]].popularGender[genders.indexOf(GenderFromShortToLong[transaction.gender])] += 1
        formerTable[ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length]].popularEmotion[emotions.indexOf(transaction.emotion)] += 1
        formerTable[ageIntervals[Math.floor(transaction.age/10)%ageIntervals.length]].popularBeverage[allBeverages.indexOf(transaction.choice)] += 1
      }
    }

    let rowTable: TableRow;
    const rowsTable: TableRow[] = Object.values(formerTable).map( (former, index) => {
      rowTable = {
          age: former.age,
          today: former.today.toFixed(0),
          popularGender: former.today ? genders[former.popularGender.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
          popularEmotion: former.today ? emotions[former.popularEmotion.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
          popularBeverage: former.today ? allBeverages[former.popularBeverage.reduce((iMax: string | number, x: number, i: any, arr: { [x: string]: number; }) => x > arr[iMax] ? i : iMax, 0)] : "-",
      }
      return rowTable
    })

    return(
        <>
            <div className="row mb-3">
                { transactions.length ? <AgeChartCard data={transactions}/> : intl.formatMessage({ id: "loading" }) }
            </div>
            <div className="row">
                { transactions.length ? <TableCard tabs={tabsTable} rows={rowsTable} caption="Age Overview" /> : intl.formatMessage({ id: "loading" }) }
            </div>
        </>
    )

}

export default Age;