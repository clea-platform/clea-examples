import React from "react";

import "./Table.css";

type TableProps = {
  tabs: string[];
  rows: string[][];
};

const Table: React.FC<TableProps> = ({ tabs, rows }) => {
  return (
    <div className="table-wrapper-scroll-y table-scrollbar">
      <table className="table">
        <thead>
          <tr>
            {tabs.map((tab) => (
              <th scope="col" key={tab}>
                {tab}
              </th>
            ))}
          </tr>
        </thead>
        <tbody>
          {rows.sort().map((row, idx) => {
            return (
              <tr key={idx}>
                {row.map((value, idx) => (
                  <td key={idx}>{value}</td>
                ))}
              </tr>
            );
          })}
        </tbody>
      </table>
    </div>
  );
};

export default Table;
