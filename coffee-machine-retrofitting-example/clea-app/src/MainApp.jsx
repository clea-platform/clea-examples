
import "core-js/stable"
import "regenerator-runtime/runtime"
import React, { Fragment } from "react";
import { Table, Button, ButtonGroup, ButtonToolbar, Col, Container, Card, Row, InputGroup,
            FormControl,  ToggleButton} from "react-bootstrap";
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
    const [chart_desc, set_chart_desc]      = React.useState({data:[]/*, width:0, height:0*/})
    const [is_chart_ready, set_is_ready]    = React.useState(false);
    const [group_by, set_group_by]          = React.useState(0)     // Units, revenues
    const [filter_by, set_filter_by]        = React.useState(0)     // All, short, long
    const [time_period, set_time_period]    = React.useState(0)     // Day, week, month, year
    const control_buttons_descriptors       = {
        group_by : [
                {name:"Units", value:0, button_ref:React.useRef(null)},
                {name:"Revenue", value:1, button_ref:React.useRef(null)}
        ],
        filter_by : [
            {name:"All", value:0},
            {name:"Short", value:1},
            {name:"Long", value:2},
        ],
        time_period : [
            {name:"Day", value:0},
            {name:"Week", value:1},
            {name:"Month", value:2},
            {name:"Year", value:3},
        ]
    }

    const [trash_bin_status_card_style,
            set_trash_bin_status_card_style]    = React.useState ("Primary")    // Primary, Danger
    const [water_level_card_style,
            set_water_level_card_style]         = React.useState ("Primary")    // Primary, Danger
    const [total_beverages, set_beverages]      = React.useState (0)
    const [daily_revenue, set_revenue]          = React.useState (0)
    const [water_status, set_water]             = React.useState ("Normal")
    const [trash_bin_status, set_trash]         = React.useState ("Normal")
    const [overview_data, set_overview]         = React.useState ([])

    const trash_bin_error_messages          = {
        "CONTAINER_OFF_ALARM_EVENT"     : "Normal",
        "CONTAINER_OPEN_ALARM_EVENT"    : "Opened",
        "CONTAINER_FULL_ALARM_EVENT"    : "Full"
    }
    const water_level_error_messages        = {
        'WATER_OFF_ALARM_EVENT'         : 'Normal',
        'WATER_OPEN_ALARM_EVENT'        : 'Opened',
        'WATER_EMPTY_ALARM_EVENT'       : 'Empty'
    }
    
    let last_update_time                    = new Date();

    const INITIAL_BEVERAGES_HISTORY_HOURS   = 48;
    const UPDATE_INTERVAL_MS                = 4000;
    /* FIXME Consider to use two different times: the first one for normal queries,
                the second one for wueries when strange event appears */
    

    // Function that allows you to update counters and overviews 
    const update_counters_and_overviews = async (new_data) => {
        console.log (new_data)

        // Updating local counters
        if (new_data.short_coffee.length > 0 ) {
            let last_item                                   = new_data.short_coffee.at(-1)
            beverages_descriptors.short_coffee.counter      = last_item.value
            beverages_descriptors.short_coffee.last_update  = last_item.timestamp
        }
        if (new_data.long_coffee.length > 0 ) {
            let last_item                                   = new_data.long_coffee.at(-1)
            beverages_descriptors.long_coffee.counter       = last_item.value
            beverages_descriptors.long_coffee.last_update   = last_item.timestamp
        }

        let today       = new Date()
        let start_date  = new Date (today.getFullYear(), today.getMonth(), today.getDate (), 0, 0, 0, 0)
        let sc_data     = astarte_client.get_short_coffee_status ({device_id, since:start_date})
        let lc_data     = astarte_client.get_long_coffee_status ({device_id, since:start_date})
        sc_data         = await sc_data
        lc_data         = await lc_data

        // Summing all arrays length to update 'total_beverages'
        set_beverages (() => {
            return beverages_descriptors.short_coffee.counter + beverages_descriptors.long_coffee.counter
        })
        
        // Computing and updating daily_revenues
        set_revenue (() => {
            let revenue = (((sc_data.at(-1).value-sc_data.at(0).value+1) * beverages_descriptors.short_coffee.revenue))
                            + (((lc_data.at(-1).value-lc_data.at(0).value+1) * beverages_descriptors.long_coffee.revenue))
            return Number (revenue.toFixed(2))
        })
        
        
        // Updating trash bin container card
        set_trash (trash_bin_error_messages[new_data.container_status[0].value])
        console.log (trash_bin_status_card_style)
        if (new_data.container_status[0].value == "CONTAINER_OFF_ALARM_EVENT") {
            // No alarm
            set_trash_bin_status_card_style (() => {return "Primary"})
        }
        else {
            // Alarm from trash bin container
            set_trash_bin_status_card_style (() => {return "Danger"})
        }
        
        // Updatng water level card
        set_water (water_level_error_messages[new_data.water_status[0].value])
        if (new_data.water_status[0].value == "WATER_OFF_ALARM_EVENT") {
            // No alarm
            set_water_level_card_style (() => {return "Primary"})
        }
        else {
            // Alarm from trash bin container
            set_water_level_card_style (() => {return "Danger"})
        }

        
        // Updating overviews
        set_overview ((old_value) => {
            return [
                {
                    beverage_name   : 'Short Coffe',
                    total_count     : beverages_descriptors.short_coffee.counter,
                    total_revenue   : Number ((beverages_descriptors.short_coffee.counter *
                                        beverages_descriptors.short_coffee.revenue).toFixed(2)),
                    unit_price      : beverages_descriptors.short_coffee.revenue,
                    last_update     : beverages_descriptors.short_coffee.last_update
                },
                {
                    beverage_name   : 'Long Coffe',
                    total_count     : beverages_descriptors.long_coffee.counter,
                    total_revenue   : Number ((beverages_descriptors.long_coffee.counter *
                                        beverages_descriptors.long_coffee.revenue).toFixed(2)),
                    unit_price      : beverages_descriptors.long_coffee.revenue,
                    last_update     : beverages_descriptors.long_coffee.last_update
                }
            ]
        })
        
        // TODO Updating chart IF AND ONLY IF target time contains the current time
        console.log (`Updating chart (IFF)..`)

        return ;
    }
    

    /*  ==================================
                Chart size effects
        ================================== */

    const get_chart_width   = () => {
        let width   = 550;
        if (is_chart_ready)
            width       = chart_ref.current.getBoundingClientRect().width;
        return width
    }

    React.useEffect(() => {
        if (chart_ref.current) {
            set_is_ready(true);
        }
    }, [chart_ref]);

    React.useEffect(() => {
        if (is_chart_ready) {
            const resize_chart = () => {
                set_chart_desc (chart => {
                    return {...chart, width:get_chart_width()}
                })
            }
        
            // Registering listeners to resize chart
            window.addEventListener("resize", resize_chart);

            return () => {
                window.removeEventListener("resize", resize_chart, false);
            }
        }
    }, [is_chart_ready]);


    // Setting up handlers to fetch real-time data
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
                    clearInterval(status_interval);
                }
                mount = false;
            }
        }
        // else : object not initialized!
    }, [astarte_client]);


    // Setting up handlers to display chart data
    React.useEffect (async () => {
        if (!astarte_client) {
            console.error ("Astarte client not ready!")
            return ;
        }

        // Retrieving interesting data depending on 'filter_by' and 'time_period' values
        let [query_params, raw_data]    = await retrieve_data (astarte_client, device_id, filter_by,
                                                                time_period)

        // Parsing obtained data
        let new_data                    = parse_retrieved_data (raw_data, beverages_descriptors,
                                                                group_by, time_period, query_params)

        // Updating 'chart_desc'
        set_chart_desc ({...chart_desc, data:new_data})

    }, [group_by, filter_by, time_period])


    return (
        <div className="p-4">
            <Container fluid>

                {/* STATISTICS ROW */}
                {/*TODO Use a descriptor to build cards*/}
                <Row>
                    <Col sm={12} md={3}>
                        <Card bg="primary" className="stats-section rounded">
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
                        <Card bg="primary" className="stats-section rounded">
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
                        <Card bg={water_level_card_style[0].toLowerCase()} className="stats-section rounded">
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
                        <Card bg={trash_bin_status_card_style[0].toLowerCase()} className="stats-section rounded">
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
                                <ButtonToolbar>
                                    
                                    <ButtonGroup className="me-2">
                                        {control_buttons_descriptors.group_by.map((el, idx) => (
                                            <ToggleButton variant='outline-primary' key={idx} id={`gb-${idx}`}
                                                type='radio' name={el.name} value={el.value}
                                                checked={el.value === group_by}
                                                ref={el.button_ref}
                                                onChange={(e) => {set_group_by(el.value)}}
                                            >
                                                    {el.name}
                                            </ToggleButton>)
                                        )}
                                    </ButtonGroup>
                                    
                                    <ButtonGroup className="me-2">
                                        {control_buttons_descriptors.filter_by.map((el, idx) => (
                                            <ToggleButton variant='outline-primary' key={idx} id={`fb-${idx}`}
                                                type='radio' name={el.name} value={el.value}
                                                checked={el.value === filter_by}
                                                onChange={(e) => {set_filter_by(el.value)}}
                                            >
                                                    {el.name}
                                            </ToggleButton>)
                                        )}
                                    </ButtonGroup>
                                    
                                    <ButtonGroup className="me-2">
                                        {control_buttons_descriptors.time_period.map((el, idx) => (
                                            <ToggleButton variant='outline-primary' key={idx} id={`tp-${idx}`}
                                                type='radio' name={el.name} value={el.value}
                                                checked={el.value === time_period}
                                                onChange={(e) => {set_time_period(el.value)}}
                                            >
                                                    {el.name}
                                            </ToggleButton>)
                                        )}
                                    </ButtonGroup>
                                
                                </ButtonToolbar>
                                
                                <div className="chart-container" ref={chart_ref}>
                                    <Fragment>
                                        <ChartData chart_descriptor={chart_desc}/>
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
                                <OverviewData data={overview_data}/>
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




const retrieve_data     = async (astarte_client, device_id, filter_by, time_period) => {
    //console.log (`Retrieving data regarding ${filter_by} on period ${time_period}`)
    let result          = {
        short_coffee    : undefined,
        long_coffee     : undefined
    }
    let query_params    = {
        device_id   : device_id,
        since       : undefined,
        to          : undefined
    }

    let today_start_end_time    = () => {
        let curr_date   = new Date ()
        let start_date  = new Date (curr_date.getFullYear(), curr_date.getMonth(),
                                    curr_date.getDate (), 0, 0, 0, 0)
        let end_date    = new Date (curr_date.getFullYear(), curr_date.getMonth(),
                                    curr_date.getDate (), 23, 59, 59, 999)
        return [start_date, end_date]
    }
    let get_last_monday = (d) => {
        const result    = new Date(d);
        while (result.getDay() !== 1) {
            result.setDate(result.getDate()-1);
        }
        return new Date (result.getFullYear(), result.getMonth(),
                            result.getDate (), 0, 0, 0, 0);
    }
    let get_next_sunday = (d) => {
        const result    = new Date(d);
        while (result.getDay() !== 0) {
            result.setDate(result.getDate()+1);
        }
        return result;
    }

    [query_params.since, query_params.to]   = today_start_end_time()
    
    // Building since - to parameters
    if (time_period == 0) {
        // Requesting data only for today
        // Nothing to do
    }
    else if (time_period == 1) {
        // Requesting data for entire week
        query_params.since  = get_last_monday (query_params.since)
        query_params.to     = get_next_sunday (query_params.to)
    }
    else if (time_period == 2) {
        // Requesting data for entire month
        query_params.since.setDate (1)
        query_params.to.setDate (1)
        query_params.to.setMonth (query_params.to.getMonth()+1)
        query_params.to.setDate (query_params.to.getDate()-1)
    }
    else if (time_period == 3) {
        query_params.since.setMonth (0)
        query_params.since.setDate (1)
        query_params.to.setMonth (11)
        query_params.to.setDate (31)
    }
    else {
        // Bad time_period value: throwing an error!
        throw `Invalid time_period value: ${time_period}`
    }

    if (filter_by == 0) {
        // Requesting all data
        result.short_coffee = astarte_client.get_short_coffee_status (query_params)
        result.long_coffee  = astarte_client.get_long_coffee_status (query_params)
    }
    else if (filter_by == 1) {
        // Requesting data only for short coffe
        result.short_coffee = astarte_client.get_short_coffee_status (query_params)
    }
    else if (filter_by == 2) {
        // Requesting data only for long coffe
        result.long_coffee  = astarte_client.get_long_coffee_status (query_params)
    }
    else {
        // Bad filter_by value: throwing an error!
        throw `Invalid filter_by value: ${filter_by}`
    }

    
    if (result.short_coffee != undefined) {
        try {result.short_coffee = await result.short_coffee}
        catch (e) {result.short_coffee = []}
    }
    if (result.long_coffee != undefined) {
        try {result.long_coffee  = await result.long_coffee}
        catch (e) {result.long_coffee = []}
    }

    return [query_params, result]
}




const parse_retrieved_data  = (raw_data, beverages_descriptors, group_by, time_period, query_params) => {
    let y_data          = []
    let new_data        = []    // item: {x, y}
    let calculator      = undefined
    let days_of_week    = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
    let months_of_year  = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "jul", "Aug", "Sept", "Oct", "Nov", "Dec"]
    let days_in_month   = (date) => {
        return new Date(date.getFullYear(), date.getMonth()+1, 0).getDate();
    }
    
    
    if (group_by == 0) {
        // Grouping by units
        calculator  = (partial, revenue) => {
            return partial + 1
        }
    }
    else {
        // Grouping by revenues
        calculator  = (partial, revenue) => {
            return partial + revenue
        }
    }


    if (time_period == 0) {
        // Considering one day
        for (let i=0; i<24; i++) {
            y_data[i]   = 0
        }
        for (let d in raw_data.short_coffee) {
            let idx     = new Date(raw_data.short_coffee[d].timestamp).getHours()
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.short_coffee.revenue)
        }
        for (let d in raw_data.long_coffee) {
            let idx     = new Date(raw_data.long_coffee[d].timestamp).getHours()
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.long_coffee.revenue)
        }
        y_data.map ((item, idx) => {
            new_data.push ({x:idx, y:item})
        })
    }
    else if (time_period == 1) {
        // Considering a week
        for (let i=0; i<7; i++) {
            y_data[i]   = 0
        }
        for (let d in raw_data.short_coffee) {
            let idx     = (new Date(raw_data.short_coffee[d].timestamp).getDay() + 6 ) % 7
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.short_coffee.revenue)
        }
        for (let d in raw_data.long_coffee) {
            let idx     = (new Date(raw_data.long_coffee[d].timestamp).getDay() + 6 ) % 7
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.long_coffee.revenue)
        }
        y_data.map ((item, idx) => {
            new_data.push ({x:days_of_week[idx], y:item})
        })
    }
    else if (time_period == 2) {
        // Considering a month
        let days_number = days_in_month(query_params.since)
        for (let i=0; i<days_number; i++) {
            y_data[i]   = 0
        }

        for (let d in raw_data.short_coffee) {
            let idx     = (new Date(raw_data.short_coffee[d].timestamp).getDate())
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.short_coffee.revenue)
        }
        for (let d in raw_data.long_coffee) {
            let idx     = (new Date(raw_data.long_coffee[d].timestamp).getDate())
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.long_coffee.revenue)
        }
        y_data.map ((item, idx) => {
            new_data.push ({x:idx+1, y:item})
        })
    }
    else if (time_period == 3) {
        // Considering an year
        for (let i=0; i<12; i++) {
            y_data[i]   = 0
        }
        for (let d in raw_data.short_coffee) {
            let idx     = new Date(raw_data.short_coffee[d].timestamp).getMonth()
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.short_coffee.revenue)
        }
        for (let d in raw_data.long_coffee) {
            let idx     = new Date(raw_data.long_coffee[d].timestamp).getMonth()
            y_data[idx] = calculator (y_data[idx], beverages_descriptors.long_coffee.revenue)
        }
        y_data.map ((item, idx) => {
            new_data.push ({x:idx+1, y:item})
            /* FIXME Not working
            console.log (`${months_of_year[idx]} -> ${item}`)
            new_data.push ({
                x:months_of_year[idx], y:item})//*/
        })
    }

    console.log (`new_data`)
    console.log (new_data)

    return new_data
}




/*  ==============================
            CHART SETTINGS
    ============================== */

const chart_options = {
    chart: {
        id: 'beverages-chart',
        stacked: false,
        zoom: {
            type: 'x',
            enabled: true,
            autoScaleYaxis: true
        },
        toolbar: {
            show : false,
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
        text: 'Beverages',
        align: 'left'
    },
    xaxis: {
        type : "category"
    },
    tooltip: {
        shared: false,
        y: {
            formatter: function (val) {
                return val.toString().indexOf(".") == -1 ? val : val.toFixed(2)
            }
        }
    },
    yaxis: {
        labels: {
            formatter: function (val) {
                return val.toFixed(0)
            },
        },
    }
};


const ChartData = ({ chart_descriptor, isMount = false }) => {
    const series    = React.useMemo(
        () => {
            return [{
                name : "Beverages",
                data : chart_descriptor.data
            }]
        }
    )
    //chart_options.xaxis.categories  = chart_descriptor.categories

    /*if (!width)
        console.log (`w: ${MainApp.get_chart_width()}`)*/
    //console.log (`New width is ${width}`)


    return (<div>
        <Chart width={chart_descriptor.width} options={chart_options} series={series} type='bar'/>
    </div>)
};




/*  =================================
            OVERVIEW SETTINGS
    ================================= */

const OverviewData  = ({data}) => {
    return (
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
            <tbody>
                <Fragment>
                    {data.map ((item) => {
                        return (
                            <tr key={item.beverage_name}>
                                <td>{item.beverage_name}</td>
                                <td>{item.total_count}</td>
                                <td>{item.total_revenue} €</td>
                                <td>{item.unit_price} €</td>
                                <td>{new Date (item.last_update).toLocaleString()}</td>
                            </tr>
                        )
                    })}
                </Fragment>
            </tbody>
        </Table>
    );
}