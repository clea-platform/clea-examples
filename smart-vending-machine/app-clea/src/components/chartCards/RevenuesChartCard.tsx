import React, { useState, useEffect, useRef } from "react"
import CSS from 'csstype';

import moment from "moment"
import { TransactionData, BeverageShort, BeverageFromLongToShort } from "../../types";
import { groupByHour, groupByDay, groupByMonth } from "../../utils/aggregate";
// @ts-ignore
import DateRangePicker from "@wojtekmaj/react-daterange-picker";

import BarChart, { Dataset, DataPoint } from "../charts/BarChart"


// --------- STYLE ---------
const subCardStyle: CSS.Properties = {
    backgroundColor: "#f8f9fa",
    borderRadius: "12px",
}

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

// -------- Chart var -------
const colors = ["red", "blue", "yellow", "green", "pink", "cyan", "orange", "lime", "grape", "indigo", "teal", "violet"];


// ------- Chart Card --------
type ChartCardProps = {
    data: TransactionData[],
}

const RevenuesChartCard: React.FC<ChartCardProps> = ( { data } ) => {

    // --------- DatePicker ----------
    const [dateRange, setDateRange] = useState<DateRange>(getDateRange(DateGranularity.DAY));

    const [dateGranularity, setDateGranularity] = useState<DateGranularity>(DateGranularity.YEAR);
    const customDateRange = useRef<boolean>(false);

    // --------- Dataset -------------
    const [dataset, setDataset] = useState<Dataset[]>([]);
    const [currentDataType, setCurrentDataType] = useState<BeverageShort>(BeverageShort.ALL);

    // Trigger filtering on new Beverage or DateRange change
    useEffect(() => {
        updateDataset(data);
    }, [currentDataType, dateRange, dateGranularity]);

    const updateDataset = (data: TransactionData[]) => {
        const newDataset: Dataset[] = [];
        let choicesPoints: DataPoint[] = [];
        const range: DateRange = customDateRange.current ? dateRange : getDateRange(dateGranularity)
        const filteredData = data.filter((transaction) => {
            return (
                transaction && Object.keys(BeverageFromLongToShort).includes(transaction.choice)
                && moment(transaction.timestamp).isBetween(range.start, range.end)
                && ( currentDataType===BeverageShort.ALL || currentDataType===BeverageFromLongToShort[transaction.choice])
            )
        });

        const aggregateHour = groupByHour(filteredData, "timestamp", "price");
        const aggregateDay = groupByDay(filteredData, "timestamp", "price");
        const aggregateMonth = groupByMonth(filteredData, "timestamp", "price");

        if (customDateRange.current) {
            choicesPoints = Object.entries(aggregateDay).map(([key, value]) => {
                return {
                label: moment.unix(parseInt(key)).format("DD/MM - dd"),
                value: parseFloat(value.toFixed(2)),
                };
            });
        } else {
            switch (dateGranularity) {
                case DateGranularity.DAY:
                    choicesPoints = Object.entries(aggregateHour).map(([key, value]) => {
                    return {
                    label: moment.unix(parseInt(key)).format("dd hh") + ":00",
                    value: parseFloat(value.toFixed(2)),
                    };
                });
                break;
                case DateGranularity.WEEK:
                    choicesPoints = Object.entries(aggregateDay).map(([key, value]) => {
                    return {
                    label: moment.unix(parseInt(key)).format("DD/MM - dddd"),
                    value: parseFloat(value.toFixed(2)),
                    };
                });
                break;
                case DateGranularity.MONTH:
                    choicesPoints = Object.entries(aggregateDay).map(([key, value]) => {
                    return {
                    label: moment.unix(parseInt(key)).format("DD - MMMM"),
                    value: parseFloat(value.toFixed(2)),
                    };
                });
                break;
                case DateGranularity.YEAR:
                    choicesPoints = Object.entries(aggregateMonth).map(([key, value]) => {
                    return {
                    label: moment.unix(parseInt(key)).format("MMMM - YYYY"),
                    value: parseFloat(value.toFixed(2)),
                    };
                });
                break;
            }
        }
            
        newDataset.push({name: "Vending Machine", color: colors[6], points: choicesPoints});
        setDataset(newDataset);
    }


    return (
        <div className="card-radius card-custom">
            <div className="row pt-2">
                {/* All Beverages */}
                <div className="col-auto">
                    <div className="py-1 ps-2 pe-3" style={subCardStyle}>
                        <div className="row justify-content-center">
                            {
                                Object.values(BeverageShort).map(( value, index ) => {
                                    return (
                                        <div className="col pe-0 ps-1" key={value}>
                                            <button
                                            className= {"btn text-center py-1 px-2 ms-1 " + ((currentDataType === value) ? "button-style-active shadow-sm" : "button-style-disabled")}
                                            onClick={() => setCurrentDataType(value)}
                                            >
                                                {value}
                                            </button>

                                        </div>
                                    )
                                })
                            }
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
                {dataset.length && <BarChart datasets={dataset} measure="€" legend={false}/>}
            </div>
        </div>
    )
}

export default RevenuesChartCard