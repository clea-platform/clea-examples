import Gender from "../pages/Gender";

export type UserPreferences = {
  language: "en" | "it";
};

export type Settings = {
  themeUrl: string;
  userPreferences: UserPreferences;
};

export type TransactionData = {
  age: number,
  choice: string,
  emotion: string,
  gender: string,
  price: number,
  suggestion: string,
  timedate?: string | null | undefined,
  timestamp: number
}

export enum BeverageShort {
  ALL = "All",
  ESP = "Esp",
  AME = "Ame",
  MIC = "MiC",
  GIN = "Gin",
  CAP = "Cap",
  CHO = "Cho",
  LAT = "Lat",
  TEA = "Tea",
  GRT = "GrT",
  MIT = "MiT",
}
export const  BeverageFromLongToShort: {[key: string]: BeverageShort} = {
  "Espresso": BeverageShort.ESP,
  "Americano": BeverageShort.AME,
  "Milky Coffee": BeverageShort.MIC,
  "Ginseng": BeverageShort.GIN,
  "Cappuccino": BeverageShort.CAP,
  "Hot Chocolate": BeverageShort.CHO,
  "Latte": BeverageShort.LAT,
  "Tea": BeverageShort.TEA,
  "Green Tea": BeverageShort.GRT,
  "Milk Tea": BeverageShort.MIT,
}

export enum GenderShort {
  W = "W",
  M = "M",
}

export const GenderFromLongToShort: {[key: string]: GenderShort} = {
  "Female": GenderShort.W,
  "Male": GenderShort.M,
}

export const GenderFromShortToLong: {[key: string]: string} = {
  "W": "Female",
  "M": "Male",
}

export const EmotionFromLongToShort: {[key: string]: string} = {
  'Neutral': "Neu",
  'Happy': "Hap",
  'Sad': "Sad",
  'Surprise': "Sup",
  'Anger': "Ang",
}

export const ageIntervals = ["10-19", "20-29", "30-39", "40-49", "50-59", "60-69", "70-79", "80-89", "90-99"];

export type UnitsData = {
  value: number; // number of units sold
  timestamp: number; // UTC time
};

export type RevenuesData = {
  value: number; // earnings in euros
  timestamp: number; // UTC time
};
