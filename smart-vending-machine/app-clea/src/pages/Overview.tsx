import React, { useEffect, useState } from "react";
import { useIntl } from "react-intl";

import moment from "moment";
import TopBar from "../components/TopBar";
import OverviewChartCard from "../components/chartCards/OverviewChartCard";
import TableCard, {TableRow, TableTab} from "../components/TableCard";

import { TransactionData } from "../types";


const tabsTable: TableTab[] = [
  {
    dataField: 'timedate',
    text: 'Timedate',
    sort: true,
  },
  {
    dataField: 'gender',
    text: 'Gender',
    sort: true,
  },
  {
    dataField: 'emotion',
    text: 'Emotion',
    sort: true,
  },
  {
    dataField: 'age',
    text: 'Age',
    sort: true,
  },
  {
    dataField: 'suggestion',
    text: 'Suggestion',
    sort: true,
  },
  {
    dataField: 'choice',
    text: 'Choice',
    sort: true,
  },
]

type OverviewProps = {
  transactions: TransactionData[]
}

const Overview: React.FC<OverviewProps> = ({transactions}) => {

    const intl = useIntl()

    let rowTable: TableRow;
    const rowsTable: TableRow[] = [];
    transactions = transactions.reverse();
    transactions.forEach( (transaction, index) => {
      // Showing only the last 50
      if (index < 50) {
        rowTable = {
          timedate: transaction.timestamp ? moment.unix(transaction.timestamp).format("YYYY/MM/DD HH:mm:ss") : "",
          gender: !transaction.gender ? "-" : transaction.gender==="None" ? "-" : transaction.gender,
          emotion: !transaction.emotion ? "-" : transaction.emotion==="None" ? "-" : transaction.emotion,
          age: transaction.age ? transaction.age.toString() : "-",
          suggestion: !transaction.suggestion ? "-" : transaction.suggestion==="None" ? "-" : transaction.suggestion,
          choice: transaction.choice ? transaction.choice : "-",
        }
        rowsTable.push(rowTable);
      }
    })

    return (
      <div>
        <div className="row mb-3">
            <TopBar transactions={transactions}/>
        </div>
        <div className="row mb-3">
            {transactions.length ? <OverviewChartCard data={transactions}/> : intl.formatMessage({ id: "loading" })}
        </div>
        <div className="row">
            {transactions.length ? <TableCard tabs={tabsTable} rows={rowsTable} pagination={true} /> : intl.formatMessage({ id: "loading" })}
        </div>
      </div>
    );
  };

export default Overview;