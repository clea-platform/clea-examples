import "core-js/stable"
import "regenerator-runtime/runtime"
import React, { Fragment } from "react";
import { Col, Container, Card, Row } from "react-bootstrap";
import { FormattedMessage } from "react-intl";
import Chart from "react-apexcharts";
import _ from 'lodash';

const default_areas = [
  {count: 0, name: "Desk Area", id: 1},
  {count: 0, name: "Coffee Area", id: 2},
  {count: 0, name: "Blackboard Area", id: 3},
  {count: 0, name: "Window Area", id: 4},
];


export const MainApp = ({ cameras, astarteClient, deviceId }) => {
  const chartRef = React.useRef(null);
  const [isReady, setIsReady] = React.useState(false);
  const [viz, setViz] = React.useState({width: 550, height: 350, data: []});
  const [counter, setCounter] = React.useState({total: 0, areas: default_areas });

  const getChartWidth = () => {
    if( isReady ){
      const domRect = chartRef.current.getBoundingClientRect();
      return domRect.width;
    }
    return 550;
  }
  
  React.useEffect(() => {
    if (cameras.length > 0){
      let mount = true;
      const intervalID = setInterval(() => {
        if(mount){
          getData(deviceId, cameras[0], astarteClient).then((data) => {
            if(data.length > 0){
              // modify counter for realtime if most recent data is within 1 minute from now
              const nowDate = new Date();
              const lastDataDate = new Date(data[0].timestamp);
              const seconds = (nowDate.getTime() - lastDataDate.getTime()) / 1000;
              if (seconds <= 60){
                setCounter(counter => {
                  const temp = parseCounterData(data[0]);
                  return {
                    total: temp.total,
                    areas: _.sortBy(_.unionBy(temp.areas, default_areas, 'id'), ['id'])
                  }
                });
              }
              else {
                setCounter({total: 0, areas: default_areas});
              }
              // update chart
              setViz( viz => {
                return {...viz, width: getChartWidth(), data: _.map(data, (d) => {
                  return { timestamp: d.reading_timestamp, value: d.people_count}
                })}
              });
            }
          })
        }
      }, 3000);

      return () => {
        clearInterval(intervalID);
        mount = false;
      }
    }
  }, [deviceId, cameras, astarteClient]);
  
  React.useEffect(()=>{
    if (chartRef.current){
      setIsReady(true);
    }
  }, [chartRef]);

  React.useEffect(()=>{
    if (isReady){
      const resizeChart = () => {
        const domRect = chartRef.current.getBoundingClientRect();
        setViz(viz => {
          return {...viz, width: domRect.width}
        });
      }
      window.addEventListener("resize", resizeChart);

      return () => {
        window.removeEventListener("resize", resizeChart, false);
      }
    }
  }, [isReady]);

  return (
    <div className="p-4">
      <Container fluid>
        <Row>
          <Col sm={12} md={6}>
            <Card bg="info" className="counter-section rounded">
              <Card.Body>
                <div className="counter-container">
                  <div className="counter-title">
                    <small>Real time</small>
                    <h3>People Counter</h3>
                  </div>
                  <div className="counter-number">
                    {counter.total}
                  </div>
                </div>
              </Card.Body>
            </Card>
            <Row>
              {counter.areas.map((area, index) => {
                return (
                <Col sm={12} md={6} key={index}>
                  <Card className="area-section rounded">
                    <Card.Body>
                      <div className="area-container">
                        <div className="area-title text-secondary">
                          <h4>{area.name}</h4>
                          <small>People Counter</small>
                        </div>
                        <div className="area-number text-primary">
                          {area.count}
                        </div>
                      </div>
                    </Card.Body>
                  </Card>
                </Col>);
              })}
            </Row>
          </Col>
          <Col sm={12} md={6}> 
            <Card className="chart-section rounded">
              <Card.Body>
                <div className="chart-container" ref={chartRef}>
                  <DataChart width={viz.width} height={viz.height} data={viz.data}/>
                </div>
              </Card.Body>
            </Card>
          </Col>
        </Row>
      </Container>
    </div>
  );
}

async function getData(deviceId, cameraId, astarteClient, limit = 200){
  return astarteClient
    .getCameraData({ deviceId, cameraId, limit})
    .then((data) => {
      return data;
    })
    .catch(() => {
      return [];
  });
}

function parseCounterData(data){

  const people = data.people ? data.people : [];

  let areas = _.map(people, (person) =>{
    return JSON.parse(person);
  });

  areas = _.groupBy(areas, (person) => {
    return person.pos_zone.id;
  })

  areas = _.map(areas, (areaZone) => {
    return {
      count: areaZone.length, 
      id: areaZone[0].pos_zone.id, 
      name: areaZone[0].pos_zone.name.split(" ").map((v) => _.capitalize(v)).join(" ")
    }
  })

  const counter = {
    total: data.people_count,
    areas
  }

  return counter;
}

const chartOptions = {
  chart: {
    id: 'people',
    type: 'line',
    stacked: false,
    zoom: {
      type: 'x',
      enabled: true,
      autoScaleYaxis: true
    },
    toolbar: {
      autoSelected: 'zoom'
    }
  },
  stroke: {
    width: [2, 2],
    curve: 'smooth'
  },
  colors: ['#FF8300'],
  dataLabels: {
    enabled: false
  },
  markers: {
    size: 0,
  },
  title: {
    text: 'People',
    align: 'left'
  },
  xaxis: {
    type: 'datetime',
  },
  tooltip: {
    shared: false,
    y: {
      formatter: function (val) {
        return (val).toFixed(0)
      }
    }
  },
  yaxis: {
    labels: {
      formatter: function (val) {
        return (val).toFixed(0);
      },
    },
  }
};

const DataChart = ({ data, width, height, isMount = false }) => {
  if (data.length === 0) {
    return (
      <div>
        <FormattedMessage id="no_data" defaultMessage="No recent data" />
      </div>
    );
  }

  const series = React.useMemo(
    () => [
      {
        name: "People",
        data: data.map((d) => [new Date(d.timestamp * 1000), d.value]),
      },
    ],
    [data]
  );
  return (
    <Fragment>
      <Chart type="line" width={width} options={chartOptions} series={series} />
    </Fragment>
  );
};