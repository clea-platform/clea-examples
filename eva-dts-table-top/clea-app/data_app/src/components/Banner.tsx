import React from "react";

import Card from "./common/Card";

import "./Banner.css";

type BannerProps = {
  title: String;
  subtitle: String;
  value?: String;
};

const Banner: React.FC<BannerProps> = ({ title, subtitle, value }) => {
  return (
    <Card className="banner-card">
      <div className="row row-cols-1 row-cols-lg-2 row-cols-auto align-items-center">
        <div className="col col-lg-5">
          <p className="card-subtitle banner-subtitle">{subtitle}</p>
          <h6 className="card-title banner-title">{title}</h6>
        </div>
        <div className="col col-lg-7 text-end">
          <h4 className="card-text banner-text">{value}</h4>
        </div>
      </div>
    </Card>
  );
};

export default Banner;
