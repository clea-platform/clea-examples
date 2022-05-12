import React, { useEffect, useRef, useState } from "react";

import Banner from "../shared/Banner";
import Card from "../shared/common/Card";
import Table from "../shared/common/Table";

import { AirflowData, AirPollutionData, EventType, AirVelocityData, AirFluxEvent } from "../types";

import AstarteClient from "../AstarteClient";
import AlertBanner from "../shared/AlertBanner";
import moment from "moment";
import AirMeasurementCard from "../shared/AirMeasurementCard";
import AirFlowCard from "../shared/AirFlowCard";

function getEventType(event: AirFluxEvent): EventType {
  switch (event.detection) {
    case "Flow":
      return EventType.AIR_FLOW;
    case "Pollution":
      return EventType.AIR_POLLUTION;
    case "Air Velocity":
      return EventType.AIR_VELOCITY;
    default:
      return EventType.UNKNOWN;
  }
}
function getEventMeasure(event: AirFluxEvent, eventType: EventType): string {
  switch (eventType) {
    case EventType.AIR_FLOW:
      return `${(event.measure * 100).toFixed(2)} %`;
    case EventType.AIR_POLLUTION:
      return `${event.measure.toFixed(2)} μg/m3`;
    case EventType.AIR_VELOCITY:
      return `${event.measure.toFixed(2)} m/s`;
    default:
      return "";
  }
}
function getEventNote(event: AirFluxEvent, eventType: EventType): string {
  switch (event.noteCode) {
    // Warning Note
    case 1:
      switch (eventType) {
        case EventType.AIR_FLOW:
          return "The air flow is near the safe threshold.";
        case EventType.AIR_POLLUTION:
          return "The air pollution is getting below the safe quality threshold.";
      }
    // Danger Note
    case 0x002:
      switch (eventType) {
        case EventType.AIR_FLOW:
          return "The air flow has exceeded the minimum acceptable threshold.";
        case EventType.AIR_POLLUTION:
          return "The air pollution has exceeded the minimum acceptable threshold.";
      }
    default:
      return "Unknown event detection.";
  }
}

type ActionDetectorProps = {
  thresholds: {
    warning: number;
    danger: number;
  };
  deviceId: string;
  astarteClient: AstarteClient;
};

const PredictiveMaintenance: React.FC<ActionDetectorProps> = ({ thresholds, deviceId, astarteClient }) => {
  const [airFlow, setAirFlow] = useState<AirflowData>();
  const [airPollution, setAirPollution] = useState<AirPollutionData>();
  const [airVelocity, setAirVelocity] = useState<AirVelocityData>();

  const tableHeader = ["Detection", "Measure", "Note", "Date"];
  const [tableRows, setTableRows] = useState<string[][]>([]);

  const lastTimestamp = useRef<number>(0);

  useEffect(() => {
    const fetchIntervalTime = 3000; // ms
    // Fetch the airflux values of the current day
    getLatestAirFluxValues();
    // Then start fetching the last values every N milliseconds
    const airFluxInterval = setInterval(() => getLatestAirFluxValues(), fetchIntervalTime);

    const maxEventsToShow = 25;
    getLatestFluxEventsHistoryValues({ numEvents: maxEventsToShow, maxEventsToShow });
    const fluxEventsInterval = setInterval(() => getLatestFluxEventsHistoryValues({ numEvents: 1, maxEventsToShow }), fetchIntervalTime);

    // Clear interval
    return () => {
      clearInterval(airFluxInterval);
      clearInterval(fluxEventsInterval);
    };
    // eslint-disable-next-line
  }, []);

  /**
   * Return the latest airflux data available.
   */
  const getLatestAirFluxValues = async () => {
    try {
      const [latestAirFlowValues, latestAirPollutionValues, latestAirVelocityValues] = await Promise.all([
        astarteClient.getAirFlowValues({
          deviceId,
          limit: 1,
        }),
        astarteClient.getPollutionValues({
          deviceId,
          limit: 1,
        }),
        astarteClient.getVelocityValues({
          deviceId,
          limit: 1,
        }),
      ]);
      const [latestAirFlow, latestAirPollution, latestAirVelocity] = [
        latestAirFlowValues.at(0),
        latestAirPollutionValues.at(0),
        latestAirVelocityValues.at(0),
      ];
      console.log(latestAirFlow, latestAirPollution, latestAirVelocity);
      if (latestAirFlow) {
        setAirFlow(latestAirFlow);
      }
      if (latestAirPollution) {
        setAirPollution(latestAirPollution);
      }
      if (latestAirVelocity) {
        setAirVelocity(latestAirVelocity);
      }
    } catch (error) {
      console.error(error);
    }
  };

  /**
   * Return the latest airflux events available.
   * @param numEvents: number of events to return
   * @param maxEventsToShow: maximum number of events to show inside the table
   */
  const getLatestFluxEventsHistoryValues = async ({ numEvents, maxEventsToShow = 50 }: { numEvents: number; maxEventsToShow: number }) => {
    const latestFluxEventsHistoryValues = await astarteClient.getFluxEventsHistoryValues({ deviceId, endpoint: "event", limit: numEvents });

    const newTableRows: string[][] = [];
    function pushRow(event: AirFluxEvent) {
      const eventType = getEventType(event);
      const eventMeasureValue = getEventMeasure(event, eventType);
      const eventNote = getEventNote(event, eventType);
      const eventDate = moment.utc(event.timestamp).format("DD/MM/YYYY HH:mm:ss");

      newTableRows.push([event.detection, eventMeasureValue, eventNote, eventDate]);
    }

    console.log(`Last timestamp recorded: ${lastTimestamp.current}`);
    latestFluxEventsHistoryValues.map((airFluxEvent) => {
      if (lastTimestamp.current !== airFluxEvent.timestamp) pushRow(airFluxEvent);
    });

    // Sort in reverse order from the most recent to the oldest
    setTableRows((tableRows) => (tableRows = [...newTableRows, ...tableRows].slice(0, maxEventsToShow)));
    lastTimestamp.current = latestFluxEventsHistoryValues.at(0)!! ? latestFluxEventsHistoryValues.at(0)!.timestamp : 0;
  };

  return (
    <div className="container-fluid">
      <div className="row mt-3">
        {/* Banners */}
        <div className="col-4">
          <AlertBanner
            title={"Predictive Maintenance"}
            thresholds={thresholds}
            // value={alerts.length ? alerts[alerts.length - 1].value : 1}
            value={airFlow?.value}
          />
        </div>

        {/* Air Measurement Card */}
        <div className="col">
          {/* <Filter label="Air Flow" color="orange" alerts={alerts} thresholds={thresholds} fetcher={getAlerts} /> */}
          <Card title="Air Measurement">
            <div className="container-fluid">
              <div className="row">
                <div className="col">
                  {airFlow ? (
                    <AirFlowCard title="Air Flow" thresholds={thresholds} value={airFlow.value}></AirFlowCard>
                  ) : (
                    <Card>
                      <h3>Loading</h3>
                    </Card>
                  )}
                </div>
              </div>
              <div className="row mt-3">
                <div className="col">
                  {airVelocity ? (
                    <AirMeasurementCard title="Air Velocity" unit="m/s" value={airVelocity.value}></AirMeasurementCard>
                  ) : (
                    <Card>
                      <h3>Loading</h3>
                    </Card>
                  )}
                </div>
                <div className="col">
                  {airPollution ? (
                    <AirMeasurementCard title="Air Pollution" unit="μg/m3" value={airPollution.value}></AirMeasurementCard>
                  ) : (
                    <Card>
                      <h3>Loading</h3>
                    </Card>
                  )}
                </div>
              </div>
            </div>
          </Card>
        </div>
      </div>

      {/* Alerts Table */}
      <div className="row mt-3">
        <div className="col">
          <Card title="Events History" className="products-table-card">
            <Table tabs={tableHeader} rows={tableRows} />
          </Card>
        </div>
      </div>
    </div>
  );
};

export default PredictiveMaintenance;
