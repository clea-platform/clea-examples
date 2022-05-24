import React from "react";

import "./Card.css";

export type CardProps = {
  title?: string;
  className?: string;
  style?: React.CSSProperties;
};

const Card: React.FC<CardProps> = ({ title, className, style, children }) => {
  return (
    <div className={"card " + className} style={style}>
      <div className="card-body">
        {title && <h4 className="card-title title-text">{title}</h4>}
        {children}
      </div>
    </div>
  );
};

export default Card;
