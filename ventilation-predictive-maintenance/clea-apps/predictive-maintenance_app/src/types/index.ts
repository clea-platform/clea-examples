export type UserPreferences = {
  language: "en" | "it";
};

export type Settings = {
  themeUrl: string;
  userPreferences: UserPreferences;
};

export type AirflowData = {
  value: number; // perc values from 0 to 1
  timestamp: number; // UTC time
};
export type AirPollutionData = {
  value: number; // pollution values in ug/m3
  timestamp: number; // UTC time
};
export type AirVelocityData = {
  value: number; // velocity values in m/s
  timestamp: number; // UTC time
};
export type AirFluxEvent = {
  detection: string; // detection event name
  measure: number; // detection event value
  noteCode: number; // code identifier explaining the issue
  timestamp: number; // UTC time
};

export enum EventType {
  AIR_FLOW,
  AIR_POLLUTION,
  AIR_VELOCITY,
  UNKNOWN,
}

export enum State {
  SUCCESS,
  WARNING,
  DANGER,
  UNKNOWN,
}
