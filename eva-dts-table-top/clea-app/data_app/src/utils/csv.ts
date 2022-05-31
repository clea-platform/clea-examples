export type CsvData = {
  beverage_name: string;
  datetime: string;
  value: number;
};

export type CsvFile = {
  filename: string;
  data: CsvData[];
};

/**
 * Create a CSV string starting from a generic array of objects.
 *
 * --------------------------------------------------
 * | beverage_name   | datetime |   units|revenues  |
 * --------------------------------------------------
 *
 * @param items array of object to convert into CSV
 * @returns the CSV string generated
 */
export function getCsvString(items: any[]): string {
  // console.log("[CreateCsv] Creating new CSV file.");

  let csv = "";

  // Loop the array of objects
  for (let row = 0; row < items.length; row++) {
    let keysAmount = Object.keys(items[row]).length;
    let keysCounter = 0;

    // If this is the first row, generate the headings
    if (row === 0) {
      // Loop each property of the object
      for (let key in items[row]) {
        // This is to not add a comma at the last cell
        // The '\r\n' adds a new line
        csv += key + (keysCounter + 1 < keysAmount ? "," : "\r\n");
        keysCounter++;
      }
    } else {
      for (let key in items[row]) {
        csv += items[row][key] + (keysCounter + 1 < keysAmount ? "," : "\r\n");
        keysCounter++;
      }
    }

    keysCounter = 0;
  }

  return csv;
}
