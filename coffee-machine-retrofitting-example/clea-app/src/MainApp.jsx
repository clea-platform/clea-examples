
import "core-js/stable"
import "regenerator-runtime/runtime"
import React, { Fragment } from "react";
import { Table, Button, Col, Container, Card, Row, InputGroup, FormControl } from "react-bootstrap";
import { FormattedMessage } from "react-intl";
import Chart from "react-apexcharts";
import _, { now } from 'lodash';




export const MainApp = ({ astarte_client, device_id }) => {
    const beverages_descriptors             = {
        short_coffee    : {
            revenue     : .60,
            counter     : 0,
            last_update : new Date()
        },
        long_coffee     : {
            revenue     : .40,
            counter     : 0,
            last_update : new Date()
        }
    }

    const chart_ref                         = React.useRef(null);
    const [is_ready, set_is_ready]          = React.useState(false);
    const [total_beverages, set_beverages]  = React.useState (0)
    const [daily_revenue, set_revenue]      = React.useState (0)
    const [water_status, set_water]         = React.useState ("Normal")
    const [trash_bin_status, set_trash]     = React.useState ("Normal")
    const [overview_data, set_overview]     = React.useState ([])
    const overview_ref                      = React.useRef(null);
    
    let last_update_time                    = new Date();

    const INITIAL_BEVERAGES_HISTORY_HOURS   = 48;
    const UPDATE_INTERVAL_MS                = 4000;
    /* FIXME Consider to use two different times: the first one for normal queries,
                the second one for wueries when strange event appears */
    

    // Function that allows you to update counters and overviews 
    const update_counters_and_overviews = (new_data) => {
        console.log (new_data)

        // Updating local counters
        if (new_data.short_coffee.length > 0 ) {
            let last_item                                   = new_data.short_coffee[new_data.short_coffee.length-1]
            beverages_descriptors.short_coffee.counter      = last_item.value
            beverages_descriptors.short_coffee.last_update  = last_item.timestamp
        }
        if (new_data.long_coffee.length > 0 ) {
            let last_item                                   = new_data.long_coffee[new_data.long_coffee.length-1]
            beverages_descriptors.long_coffee.counter       = last_item.value
            beverages_descriptors.long_coffee.last_update   = last_item.timestamp
        }

        // Summing all arrays length to update 'total_beverages'
        console.log (`Updating total beverages..`)
        set_beverages (() => {
            return beverages_descriptors.short_coffee.counter + beverages_descriptors.long_coffee.counter
        })
        
        // Computing and updating daily_revenues
        console.log (`Computing and updating revenues..`)
        set_revenue (() => {
            return (beverages_descriptors.short_coffee.counter * beverages_descriptors.short_coffee.revenue)
                    + (beverages_descriptors.long_coffee.counter * beverages_descriptors.long_coffee.revenue)
        })
        
        // TODO Updating water and bin levels
        console.log (`Updating water and bin levels..`)
        
        // Updating overviews
        console.log (`Updating overviews..`)
        set_overview ((old_value) => {
            return [
                {
                    beverage_name   : 'Short Coffe',
                    total_count     : beverages_descriptors.short_coffee.counter,
                    total_revenue   : beverages_descriptors.short_coffee.counter *
                                        beverages_descriptors.short_coffee.revenue,
                    unit_price      : beverages_descriptors.short_coffee.revenue,
                    last_update     : beverages_descriptors.short_coffee.last_update
                },
                {
                    beverage_name   : 'Long Coffe',
                    total_count     : beverages_descriptors.long_coffee.counter,
                    total_revenue   : beverages_descriptors.long_coffee.counter *
                                        beverages_descriptors.long_coffee.revenue,
                    unit_price      : beverages_descriptors.long_coffee.revenue,
                    last_update     : beverages_descriptors.long_coffee.last_update
                }
            ]
        })
        
        // TODO Updating chart IF AND ONLY IF target time contains the current time
        console.log (`Updating chart (IFF)..`)

        return ;
    }
    

    const getChartWidth = () => {
        if (is_ready) {
            const domRect = chart_ref.current.getBoundingClientRect();
            return domRect.width;
        }
        return 550;
    }


    React.useEffect(() => {
        if (chart_ref.current) {
            set_is_ready(true);
        }
    }, [chart_ref]);


    React.useEffect(() => {
        if (is_ready) {
            const resizeChart = () => {
                const domRect = chart_ref.current.getBoundingClientRect();
                /* FIXME
                setViz(viz => {
                    return { ...viz, width: domRect.width }
                });*/
            }
            window.addEventListener("resize", resizeChart);

            return () => {
                window.removeEventListener("resize", resizeChart, false);
            }
        }
    }, [is_ready]);


    React.useEffect (async () => {
        if (astarte_client) {
            try {
                let since   = new Date ()
                since.setHours (since.getHours() - INITIAL_BEVERAGES_HISTORY_HOURS)
                let historical_data = await get_all_data_since (astarte_client, device_id, since)
                update_counters_and_overviews (historical_data)
            } catch (err) {
                console.error (`Cannot retrieve historical data: ${err}`)
            }
            
            // Registering beverages count polling interval
            let status_interval = setInterval (async () => {
                try {
                    let since           = last_update_time
                    last_update_time    = new Date ()
                    let data            = await get_all_data_since (astarte_client, device_id, since)
                    if (Object.keys(data).length != 0)
                        update_counters_and_overviews (data)
                } catch (err) {
                    console.log (`Catched an error:`)
                }
            }, UPDATE_INTERVAL_MS)

            return () => {
                if (status_interval) {
                    console.log ("Clearing interval...")
                    clearInterval(status_interval);
                }
                mount = false;
            }
        }
        // else : object not initialized!
    }, [astarte_client]);


    return (
        <div className="p-4">
            <Container fluid>


                {/* STATISTICS ROW */}
                <Row>
                    <Col sm={12} md={3}>
                        <Card bg="info" className="stats-section rounded">
                            <Card.Body>
                                <div className="stats-container">
                                    <div className="stats-title">
                                        <div className="stats-number">{total_beverages}</div>
                                        <h3>Served Beverages</h3>
                                        <small>Real time</small>
                                    </div>
                                </div>
                            </Card.Body>
                        </Card>
                    </Col>

                    <Col sm={12} md={3}>
                        <Card bg="info" className="stats-section rounded">
                            <Card.Body>
                                <div className="stats-container">
                                    <div className="stats-title">
                                        <div className="stats-number">{daily_revenue} €</div>
                                        <h3>Daily Revenue</h3>
                                        <small>Real time</small>
                                    </div>
                                </div>
                            </Card.Body>
                        </Card>
                    </Col>

                    <Col sm={12} md={3}>
                        <Card bg="info" className="stats-section rounded">
                            <Card.Body>
                                <div className="stats-container">
                                    <div className="stats-title">
                                        <div className="stats-number">{water_status}</div>
                                        <h3>Water Level</h3>
                                        <small>Real time</small>
                                    </div>
                                </div>
                            </Card.Body>
                        </Card>
                    </Col>

                    <Col sm={12} md={3}>
                        <Card bg="info" className="stats-section rounded">
                            <Card.Body>
                                <div className="stats-container">
                                    <div className="stats-title">
                                        <div className="stats-number">{trash_bin_status}</div>
                                        <h3>Trash Bin Status</h3>
                                        <small>Real time</small>
                                    </div>
                                </div>
                            </Card.Body>
                        </Card>
                    </Col>
                </Row>


                {/* CHART ROW */}
                <Row>
                    <Col sm={12} md={12}>
                        <Card className="chart-section rounded">
                            <Card.Body>
                                {/*https://codesandbox.io/s/i8b44?file=/src/app/app.component.ts:952-969
                                    customIcons @ https://apexcharts.com/docs/options/chart/toolbar/
                                    https://codepen.io/junedchhipa/pen/YJQKOy*/}
                                
                                <div className="chart-container" ref={chart_ref}>
                                    <Fragment>
                                        <DataChart height={10} data={[]} />
                                    </Fragment>
                                </div>
                            </Card.Body>
                        </Card>
                    </Col>
                </Row>


                {/* PRODUCTS OVERVIEW ROW */}
                <Row>
                    <Col sm={12} md={12}>
                        <Card bg="info" className="rounded">
                            <Card.Body>
                                <div className="stats-title">
                                    Products Overview
                                </div>
                                
                                <Table striped bordered hover>
                                    <thead>
                                        <tr>
                                        <th>Beverage</th>
                                        <th>Total Count</th>
                                        <th>Total Revenue</th>
                                        <th>Unit Price</th>
                                        <th>Last Update</th>
                                        </tr>
                                    </thead>
                                    <tbody ref={overview_ref}>
                                        <Fragment>
                                            <OverviewData data={overview_data}/>
                                        </Fragment>
                                    </tbody>
                                    </Table>
                            </Card.Body>
                        </Card>
                    </Col>
                </Row>
            </Container>
        </div>
    );
}




/*  =====================================
            ASTARTE DATA HANDLERS
    ===================================== */


let get_all_data_since  = async (astarte_client, device_id, since) => {
    let grouped_data        = {
        container_status    : [],
        water_status        : [],
        short_coffee        : [],
        long_coffee         : [],
    }
    
    let container_status    = astarte_client.get_container_status ({device_id, limit:1})
    let water_status        = astarte_client.get_water_status ({device_id, limit:1})
    // Short coffee and long coffee requests are not prepared here because they may throw errors
    
    try {
        let s_data  = await astarte_client.get_short_coffee_status ({device_id, since})
        s_data.map (e => grouped_data.short_coffee.push (e))
    } catch (err) {
        // No new data
    }
    
    try {
        let l_data  = await astarte_client.get_long_coffee_status ({device_id, since})
        l_data.map (e => grouped_data.long_coffee.push (e))
    } catch (err) {
        // No new data
    }

    try {
        grouped_data.container_status   = await container_status
    } catch (err) {
        // No new data
    }
    try {
        grouped_data.water_status       = await water_status
    } catch (err) {
        // No new data
    }

    return grouped_data
}




/*  ==============================
            CHART SETTINGS
    ============================== */

const chartOptions = {
    chart: {
        id: 'people',
        type: 'line',
        stacked: false,
        zoom: {
            type: 'x',
            enabled: false,
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


const DataChart = ({ data, height, isMount = false }) => {

    if (data.length === 0) {
        return (
            <div>
                <FormattedMessage id="no_data" defaultMessage="No recent data" />
            </div>
        );
    }

    const series = React.useMemo(
        () => {
            
            return [
                {
                    name: "People",
                    data: data.map((d) => [new Date(d.timestamp), d.value]),
                },
            ]
        },
        [data]
    );

    return (
        <Chart type="line" options={chartOptions} series={series} />
    );
};




/*  =================================
            OVERVIEW SETTINGS
    ================================= */

const OverviewData  = ({data}) => {
    return (
        data.map ((item) => {
            return (
                <tr key={item.beverage_name}>
                    <td>{item.beverage_name}</td>
                    <td>{item.total_count}</td>
                    <td>{item.total_revenue} €</td>
                    <td>{item.unit_price} €</td>
                    <td>{new Date (item.last_update).toLocaleString()}</td>
                </tr>)
        })
    );
}