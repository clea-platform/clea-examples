// SideMenu.js
import React from 'react'

import MenuLink from "./routers/MenuLink";

export type SideMenuProps = {
  titles:  Array<string>;
  paths: Array<string>;
};

const SideMenu: React.FC<SideMenuProps> = ({ titles, paths }) => {
  const cardStyle = {
    "borderRadius": "15px",
  }

  return (
    <div className='card card-custom' style={cardStyle}>
          {titles.map((title: string, index) => {
            return (
              <MenuLink key={title} title={title} path={paths[index]} />
            )
          })}
    </div>
  );
};

export default SideMenu;