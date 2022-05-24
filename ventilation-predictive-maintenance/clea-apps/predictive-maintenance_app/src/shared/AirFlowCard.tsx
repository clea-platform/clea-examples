import React from "react";

import Card from "./common/Card";

import "./AirFlowCard.css";

type DynamicBarProps = {
  value: number;
  thresholds: {
    warning: number;
    danger: number;
  };
};

const DynamicBar: React.FC<DynamicBarProps> = ({ value, thresholds }) => {
  const percentages = new Array(50).fill(null).map((_, i) => Number((i / 50 + 0.02).toFixed(2)));
  // console.log(percentages);
  return (
    <div className="d-flex flex-row">
      {percentages.map((i) => {
        if (i === thresholds.danger) {
          return <div id="vertical-box-danger" key={i} className="me-1"></div>;
        } else if (i === thresholds.warning) {
          return <div id="vertical-box-warning" key={i} className="me-1"></div>;
        } else if (i <= value) {
          return <div id="vertical-box-default" key={i} className="me-1"></div>;
        } else {
          return <div id="vertical-box-empty" key={i} className="me-1"></div>;
        }
      })}
    </div>
  );
};

type AirFlowCardProps = {
  title: string;
  value: number;
  thresholds: {
    warning: number;
    danger: number;
  };
};

const AirFlowCard: React.FC<AirFlowCardProps> = ({ title, value, thresholds }) => {
  return (
    <Card>
      <h6>{title}</h6>

      <div className="d-flex align-items-center">
        <DynamicBar value={value} thresholds={thresholds}></DynamicBar>
        <div className="ms-auto">
          <p id="value-perc" className="my-auto">
            {(value * 100).toFixed(2)}%
          </p>
        </div>
      </div>
    </Card>
  );
};

export default AirFlowCard;
