import React from "react";

import "./Card.css";

export type CardProps = {
  title?: string;
  className?: string;
};

const Card: React.FC<CardProps> = ({ title, className, children }) => {
  return (
    <div className={"card " + className} style={{ minHeight: "100%" }}>
      <div className="card-body">
        {title && <h4 className="card-title title-text">{title}</h4>}
        {children}
      </div>
    </div>
  );
};

export default Card;
