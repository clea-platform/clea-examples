import React from "react";

import Card from "./common/Card";

import "./Banner.css";

type BannerProps = {
  title: String;
  subtitle: String;
};

const Banner: React.FC<BannerProps> = ({ title, subtitle }) => {
  return (
    <Card className="banner-card">
      <p className="card-subtitle banner-subtitle">{subtitle}</p>
      <h6 className="card-title banner-title">{title}</h6>
    </Card>
  );
};

export default Banner;
