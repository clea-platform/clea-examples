import React from "react";

import Card from "./common/Card";
import { State } from "../types";

import "./AlertBanner.css";

import warning_icon from "../assets/svg/warning-icon.svg";
import danger_icon from "../assets/svg/danger-icon.svg";

const getDescription = (state: State): string => {
  switch (state) {
    case State.SUCCESS:
      return "The predictive maintenance system does not detect any problems in the operation of the system. The system will warn you in time if one of the values approaches the danger threshold so that you can act in advance in case you need to intervene, in order to reduce downtime situations.";
    case State.WARNING:
      return "The airflow through the engine filter is dropping toward the minimum safe threshold. The system is not yet in danger but immediate action is recommended to discover the cause of the problem.";
    case State.DANGER:
      return "The airflow through the engine filter has exceeded the minimum acceptable threshold. The machine may have already sustained medium or large amounts of damage. In a short period of time the system may faut. You should contact assistance service immediately to resolve the problem.";
    case State.UNKNOWN:
      return "No airflow data has been detected yet.";
  }
};
const getBackground = (state: State): string => {
  switch (state) {
    case State.SUCCESS:
      return "linear-gradient(to bottom right, #11b0ef, #11b0ef)";
    case State.WARNING:
      return "linear-gradient(to bottom right, #ffcf0f, #ff1c02)";
    case State.DANGER:
      return "linear-gradient(to bottom right, #ce4302, #ce4302)";
    case State.UNKNOWN:
      return "linear-gradient(to bottom right, grey, grey)";
  }
};
const getIcon = (state: State): string => {
  switch (state) {
    case State.SUCCESS:
      return "";
    case State.WARNING:
      return warning_icon;
    case State.DANGER:
      return warning_icon;
    case State.UNKNOWN:
      return "";
  }
};
const getValueString = (value: number | undefined, state: State): string => {
  switch (state) {
    case State.SUCCESS:
      return "No warning";
    case State.WARNING:
      return "Warning";
    case State.DANGER:
      return (value! * 100).toFixed(0) + "%";
    case State.UNKNOWN:
      return "Loading...";
  }
};

type AlertBannerProps = {
  title: string;
  value?: number;
  thresholds: {
    warning: number;
    danger: number;
  };
};

const AlertBanner: React.FC<AlertBannerProps> = ({ title, value, thresholds }) => {
  let state = State.SUCCESS;
  if (!value) {
    state = State.UNKNOWN;
  } else if (value <= thresholds.danger) {
    state = State.DANGER;
  } else if (value <= thresholds.warning) {
    state = State.WARNING;
  }

  return (
    <Card className="alert-banner-card h-100" style={{ backgroundImage: getBackground(state) }}>
      <h6 className="card-title alert-banner-title">{title}</h6>
      <p className="card-text alert-banner-value">{getValueString(value, state)}</p>
      <p className="card-subtitle alert-banner-subtitle">{getDescription(state)}</p>
      <img className="alert-banner-icon" src={getIcon(state)} />
    </Card>
  );
};

export default AlertBanner;
