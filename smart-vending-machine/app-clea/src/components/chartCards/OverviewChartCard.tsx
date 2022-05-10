import React, { useState, useEffect, useRef, forwardRef } from "react"
import { FormattedMessage, useIntl } from "react-intl";
import CSS from 'csstype';

import moment from "moment"
import { TransactionData, BeverageFromLongToShort } from "../../types";
// @ts-ignore
import DateRangePicker from "@wojtekmaj/react-daterange-picker";

import BarChart, { Dataset, DataPoint } from "../charts/BarChart"


// --------- STYLE ---------
const subCardStyle: CSS.Properties = {
    backgroundColor: "#f8f9fa",
    borderRadius: "12px",
}

// -------- Chart var -------
const labels = Object.keys(BeverageFromLongToShort);;
const colors = ["red", "blue", "yellow", "green", "pink", "cyan", "orange", "lime", "grape", "indigo", "teal", "violet"];

// -------- Date ----------
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


enum DataType {
    UNITS,
    REVENUES,
}

// ------- Chart Card ---------
type ChartCardProps = {
    data: TransactionData[],
}

const OverviewChartCard: React.FC<ChartCardProps> = ( { data } ) => {

    // --------- DatePicker ----------
    const [dateRange, setDateRange] = useState<DateRange>(getDateRange(DateGranularity.DAY));

    const [dateGranularity, setDateGranularity] = useState<DateGranularity>(DateGranularity.YEAR);
    const customDateRange = useRef<boolean>(false);

    // --------- Dataset -------------
    const [dataset, setDataset] = useState<Dataset[]>([]);
    const [currentDataType, setCurrentDataType] = useState<DataType>(DataType.UNITS);

    // Trigger filtering on new Beverage or DateRange change
    useEffect(() => {
        updateDataset(data);
    }, [currentDataType, dateRange, dateGranularity]);

    const units: { [key: string]: number } = {}
    const revenenues: { [key: string]: number } = {}
    const choicesPoints: DataPoint[] = []

    labels.forEach((label) => {
        units[label]=0
        revenenues[label]=0
    })

    const updateDataset = (data: TransactionData[]) => {
        const newDataset: Dataset[] = [];
        const range: DateRange = customDateRange.current ? dateRange : getDateRange(dateGranularity)
        data.forEach( (transaction, index) => {
            if (transaction && labels.includes(transaction.choice)
             && moment(transaction.timestamp).isBetween(range.start, range.end)) {
                units[transaction.choice] += 1
                revenenues[transaction.choice] += transaction.price || 0
            }
        })
        if (currentDataType === DataType.UNITS) {
            Object.entries(units).forEach(([key, value], index) => {            
                choicesPoints.push({label: key, value})
            })
        } else {
            Object.entries(revenenues).forEach(([key, value], index) => {            
                choicesPoints.push({label: key, value: Math.round(value*100) / 100})
            })
        }
         
        newDataset.push({name: "Vending Machine", color: colors[6], points: choicesPoints});
        setDataset(newDataset);
    }


    return (
        <div className="card-custom">
            <div className="row pt-2">
                {/* Revenue - Quantity */}
                <div className="col-auto">
                    <div className="py-1" style={subCardStyle}>
                        <div className="row justify-content-center">
                            <div className="col pe-0">
                                <button
                                className= {"btn text-center py-1 px-2 ms-1 " + ((currentDataType === DataType.UNITS) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                onClick={() => setCurrentDataType(DataType.UNITS)}
                                >
                                    <FormattedMessage id="quantity" />                                                                
                                </button>
                            </div>
                            <div className="col">
                                <button 
                                className= {"btn text-center py-1 px-2 ms-1 " + ((currentDataType === DataType.REVENUES) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                onClick={() => setCurrentDataType(DataType.REVENUES)}
                                >
                                    <FormattedMessage id="revenues" />
                                </button>
                            </div>
                        </div>
                    </div>
                </div>
                {/* Day - Week - Month - Year */}
                <div className="col-auto ms-auto">
                    <div className="py-1 px-1" style={subCardStyle}>
                        <div className="row">
                            {/* Day */}
                            <div className="col pe-0">
                                <button 
                                className={"card btn ms-1 text-center " + ((dateGranularity === DateGranularity.DAY && !customDateRange.current) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                onClick={() => {
                                    customDateRange.current = false;
                                    setDateRange(getDateRange(DateGranularity.DAY));
                                    setDateGranularity(DateGranularity.DAY);
                                  }}
                                >
                                    Day
                                </button>
                            </div>
                            {/* Week */}
                            <div className="col px-0">
                                <button
                                className={"card btn ms-1 text-center " + ((dateGranularity === DateGranularity.WEEK && !customDateRange.current) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                onClick={() => {
                                    customDateRange.current = false;
                                    setDateRange(getDateRange(DateGranularity.WEEK));
                                    setDateGranularity(DateGranularity.WEEK);
                                  }}
                                >
                                    Week
                                </button>
                            </div>
                            {/* Month */}
                            <div className="col px-0">
                                <button
                                className={"card btn ms-1 text-center " + ((dateGranularity === DateGranularity.MONTH && !customDateRange.current) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                onClick={() => {
                                    customDateRange.current = false;
                                    setDateRange(getDateRange(DateGranularity.MONTH));
                                    setDateGranularity(DateGranularity.MONTH);
                                  }}
                                  >
                                    Month
                                </button>
                            </div>
                            {/* Year */}
                            <div className="col ps-0">
                                <button
                                className={"card btn ms-1 text-center " + ((dateGranularity === DateGranularity.YEAR && !customDateRange.current) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                onClick={() => {
                                    customDateRange.current = false;
                                    setDateRange(getDateRange(DateGranularity.YEAR));
                                    setDateGranularity(DateGranularity.YEAR);
                                  }}
                                >
                                    Year
                                </button>
                            </div>
                        </div>
                    </div>
                </div>
                {/* Calendar */}
                <div className="col-auto">
                    <div className="bg-light p-1 mt-1" style={subCardStyle}>
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
            </div>
            <div className="row">
                {dataset.length && <BarChart datasets={dataset} measure={currentDataType === DataType.REVENUES ? "€" : ""} legend={false} />}
            </div>
        </div>
    );
};

export default OverviewChartCard;