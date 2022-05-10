import moment from "moment";
import { groupBy, mapValues } from "lodash";

// Order data by key (timestamp)
const orderByKey = (data: any): Object => {
  return Object.keys(data)
    .sort()
    .reduce((obj: any, key) => {
      obj[key] = data[key];
      return obj;
    }, {});
};

/**
 * Group data by each hour and aggregate the values.
 * @param data A generic array of data objects.
 * @param groupKey The timestamp key name on which to perform the aggregation.
 * @param aggregateKey The value key name used on the reduce operation.
 * @returns An array containing the groups.
 */
export function groupByHour(data: any[], groupKey: string, aggregateKey: string) {
  const hour = (obj: any) => {
    return moment.unix(obj[groupKey]).startOf("hour").unix();
  };

  return orderByKey(mapValues(groupBy(data, hour), (item) => item.reduce((acc, obj) => acc + obj[aggregateKey], 0)));
}

/**
 * Group data by each day and aggregate the values.
 * @param data A generic array of data objects.
 * @param groupKey The timestamp key name on which to perform the aggregation.
 * @param aggregateKey The value key name used on the reduce operation.
 * @returns An array containing the groups.
 */
export function groupByDay(data: any[], groupKey: string, aggregateKey: string) {
  const day = (obj: any) => {
    return moment.unix(obj[groupKey]).startOf("day").unix();
  };

  return orderByKey(mapValues(groupBy(data, day), (item) => item.reduce((acc, obj) => acc + obj[aggregateKey], 0)));
}

/**
 * Group data by each week and aggregate the values.
 * @param data A generic array of data objects.
 * @param groupKey The timestamp key name on which to perform the aggregation.
 * @param aggregateKey The value key name used on the reduce operation.
 * @returns An array containing the groups.
 */
export function groupByWeek(data: any[], groupKey: string, aggregateKey: string) {
  const week = (obj: any) => {
    return moment.unix(obj[groupKey]).startOf("week").unix();
  };

  return orderByKey(mapValues(groupBy(data, week), (item) => item.reduce((acc, obj) => acc + obj[aggregateKey], 0)));
}

/**
 * Group data by each month and aggregate the values.
 * @param data A generic array of data objects.
 * @param groupKey The timestamp key name on which to perform the aggregation.
 * @param aggregateKey The value key name used on the reduce operation.
 * @returns An array containing the groups.
 */
export function groupByMonth(data: any[], groupKey: string, aggregateKey: string) {
  const month = (obj: any) => {
    return moment.unix(obj[groupKey]).startOf("month").unix();
  };

  return orderByKey(mapValues(groupBy(data, month), (item) => item.reduce((acc, obj) => acc + obj[aggregateKey], 0)));
}

/**
 * Group data by each year and aggregate the values.
 * @param data A generic array of data objects.
 * @param groupKey The timestamp key name on which to perform the aggregation.
 * @param aggregateKey The value key name used on the reduce operation.
 * @returns An array containing the groups.
 */
export function groupByYear(data: any[], groupKey: string, aggregateKey: string) {
  const year = (obj: any) => {
    return moment.unix(obj[groupKey]).startOf("year").unix();
  };

  return orderByKey(mapValues(groupBy(data, year), (item) => item.reduce((acc, obj) => acc + obj[aggregateKey], 0)));
}
