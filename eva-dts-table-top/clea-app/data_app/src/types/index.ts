export type UserPreferences = {
  language: "en" | "it";
};

export type Settings = {
  themeUrl: string;
  userPreferences: UserPreferences;
};

export enum DataType {
  UNITS,
  REVENUES,
}

export type UnitsData = {
  value: number; // number of units sold
  timestamp: number; // UTC time
};

export type RevenuesData = {
  value: number; // earnings in euros
  timestamp: number; // UTC time
};
