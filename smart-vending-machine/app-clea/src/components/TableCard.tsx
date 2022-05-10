import React from "react"
import CSS from 'csstype';

import BootstrapTable, { PaginationOptions } from 'react-bootstrap-table-next';
// @ts-ignore
import paginationFactory from 'react-bootstrap-table2-paginator';


// -------- STYLE --------
const cartTitle = {
    color: "#0631cc",
    fontSize: "20px",
    fontWeight: "700",
    marginTop: "0.5em",
    // marginLeft: "0.5em",
};
/* const spanPagination = {
    backgroundColor: '#11b0ef',
} */

type TableProps = {
    tabs: TableTab[];
    rows: TableRow[];
    caption?: string;
    pagination?: boolean;
};
export type TableTab = {
    dataField: string,
    text: string,
    sort?: boolean,
};
export type TableRow = {
    timedate: number,
    gender: string,
    emotion: string,
    age: string,
    suggestion: string,
    choice: string,
} | {
    beverage: string,
    ordered: string | number,
    revenue: string | number,
    price: string | number,
    popularGender: string,
    popularAge: string,
} | {}

/* const customTotal = (from: number, to: number, size: number) => (
    <span className="react-bootstrap-table-pagination-total ms-1">
      Showing { from } to { to } of { size } Results
    </span>
  ); */
const pageButtonRenderer: any = ( (page: string|number, active:boolean, onPageChange: any) => {
    const handleClick = (e: { preventDefault: () => void; }) => {
        e.preventDefault();
        onPageChange(page);
    };
    const activeStyle: CSS.Properties = {
        borderRadius: "50px",
        width: "25px",
        textAlignLast: "center",
        cursor: "pointer",
        borderWidth: "1px",
        borderStyle: "solid",
    };
    if (active) {
        activeStyle.backgroundColor = "#0631cc";
        activeStyle.color = 'white';
        activeStyle.borderColor = "#0631cc";
    } else {
        activeStyle.color = 'black';
        activeStyle.borderColor = "#dadada";

    }
    if (typeof page === 'string') {
        activeStyle.color = 'black';
    }
    return (
        <div className="col px-1 page-item" key={page}>
            <div style={ activeStyle } onClick={ handleClick }>
                { typeof page === 'string' ? page : page + 1}
            </div>
        </div>
    );
});
const pageListRenderer: any = ({pages, onPageChange}: any) => {
    const pageWithoutDoubleInd = pages.filter((p: { page: string; }) => (p.page !== '<<' && p.page !== '>>'));
    return (
      <div className="col-auto ms-auto row me-5" style={{textAlign:"justify"}}>
        {
          pageWithoutDoubleInd.map((p: any) => {
                return (pageButtonRenderer(p.page, p.active, onPageChange));
          })
        }
      </div>
    );
};

const TableCard: React.FC<TableProps> = ( { tabs, rows, caption="Latest detections", pagination=false }) => {
      
    const CaptionElement = () => <div className="card-title" style={cartTitle}> {caption}</div>;

    const options: PaginationOptions = {
        pageListRenderer,
        paginationSize: 4,
        pageStartIndex: 0,
        prePageText: '<',
        nextPageText: '>',
        hideSizePerPage: true,
        hidePageListOnlyOnePage: true,
        nextPageTitle: 'First page',
        prePageTitle: 'Pre page',
        firstPageTitle: 'Next page',
        lastPageTitle: 'Last page',
    };

    if(pagination){
        // options.showTotal = true;
        // options.paginationTotalRenderer = customTotal
        options.sizePerPageList = [{
            text: '5', value: 5
        }, {
            text: '10', value: 10
        }, {
            text: 'All', value: rows.length
        }] 
    }


    return (
        <div className="card-custom mb-3 pb-2">
            {pagination ? 
            <BootstrapTable
                bootstrap4
                keyField={Object.keys(rows[0])[0]}
                data={rows}
                columns={tabs}
                caption={<CaptionElement />}
                pagination={ paginationFactory(options) }
            />
            :
            <BootstrapTable
                bootstrap4
                keyField={Object.keys(rows[0])[0]}
                data={rows}
                columns={tabs}
                caption={<CaptionElement />}
            />
            }
        </div>
    );
};

export default TableCard;