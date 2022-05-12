import React from "react";

import Card from "./common/Card";

import "./AirMeasurementCard.css";

type AirMeasurementCardProps = {
  title: string;
  unit: string;
  value: number;
  className?: string;
};

const AirMeasurementCard: React.FC<AirMeasurementCardProps> = ({ title, unit, value, className }) => {
  return (
    <Card className={className}>
      <div className="d-flex">
        <p className="title">{title}</p>
        <p className="unit ms-auto">{unit}</p>
      </div>
      <p className="value mb-0 mt-3">{value.toFixed(2)}</p>
    </Card>
  );
};

export default AirMeasurementCard;
