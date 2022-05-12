#include <Arduino.h>
#include <SerialCommands.h>
#include <Wire.h>
#include "SparkFunCCS811.h"
// ------------------------------ GLOBALS ------------------------------

// PINS
#define T1H_PIN A0
#define T1C_PIN A2
#define T2H_PIN A1
#define T2C_PIN A3

// Sensors
#define CCS811_ADDR 0x5A // Default I2C Address
CCS811 mySensor(CCS811_ADDR);

/*
 * thermistor parameters:
 * RT0: 10 000 Ω
 * B: 3977 K +- 0.75%
 * T0:  25 C
 * +- 5%
 */

#define RT0 10000 // Ω
#define B 3977    // K
#define VCC 5     // Supply voltage
#define R 10000   // R=10KΩ
#define ROOM_TEMP_C 25
#define KELVIN_FACTOR 273.15
// AIR FLOW PARAMETERS
#define MIN_TEMP_DIFF 15
#define MAX_TEMP_DIFF 48
#define MAX_RES_TEMP 90
#define MAX_FLOW_VALUE 1.0
#define MIN_FLOW_VALUE 0.0
#define MIN_ALLOWED_FLOW 0.1
#define E_NEP 2.71828
#define ALPHA 0.05
#define ALPHA_RIGHT 0.07
#define ALPHA_LEFT 0.25
#define PIVOT_POINT 3.47

// Sensors temperatures
float T1C, T1H, T2C, T2H;
// Room temperature
const float T0 = ROOM_TEMP_C + KELVIN_FACTOR;

// Global values
float GLOBAL_FLOW_RATE = 0.0;
uint16_t GLOBAL_CO2 = 0;
uint16_t GLOBAL_TVOC = 0;

// Init serial commands
char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

// ------------------------------ FUNCTIONS ------------------------------

void cmd_unrecognized(SerialCommands *sender, const char *cmd)
{
  sender->GetSerial()->println("ERROR: Unrecognized command [" + String(cmd) + "]");
  // sender->GetSerial()->print(cmd);
  // sender->GetSerial()->println("]");
}

// HELLO
void cmd_hello(SerialCommands *sender)
{
  sender->GetSerial()->println("ready");
}

// READ AIR FLOW
void cmd_read_air_flow(SerialCommands *sender)
{
  sender->GetSerial()->println("flow: " + String(GLOBAL_FLOW_RATE));
}

// READ AIR QUALITY
void cmd_read_air_quality(SerialCommands *sender)
{
  // sender->GetSerial()->println("CO2: " + String(GLOBAL_CO2));
  // sender->GetSerial()->println("TVOC: " + String(GLOBAL_TVOC / 1000));

  // Converted in mg/m3
  sender->GetSerial()->println("pollution: " + String((GLOBAL_TVOC / 1000) * 0.0409 * 78.9516));
}

SerialCommand cmd_hello_("hello", cmd_hello);
SerialCommand cmd_read_air_flow_("f", cmd_read_air_flow);
SerialCommand cmd_read_air_quality_("q", cmd_read_air_quality);

// READ TEMP SENSORS
float leanReadTemperature(int pin)
{
  float vrt = analogRead(pin);                           // Acquisition analog value of VRT
  float voltage = (5.00 / 1023.00) * vrt;                // Conversion to voltage
  float VR = VCC - voltage;                              // Voltage difference
  float rt = (voltage * R) / VR;                         // Resistance of RT
  float ln = log(rt / RT0);                              // Logarithm of resistance
  float temperatureKelvin = (1 / ((ln / B) + (1 / T0))); // Temperature from thermistor
  return temperatureKelvin - KELVIN_FACTOR;              // Conversion from kelvin to Celsius
}

// EVENTS LISTENER FOR LOOPING
void eventLoop(SerialCommands serial_commands_)
{
  T1H = leanReadTemperature(T1H_PIN);
  T1C = leanReadTemperature(T1C_PIN);
  T2H = leanReadTemperature(T2H_PIN);
  T2C = leanReadTemperature(T2C_PIN);

  // Tst 1
  // float sens1Map = map(T1H - T1C, 0.0, MAX_TEMP_DIFF, MAX_FLOW_VALUE, MIN_FLOW_VALUE);
  // float sens2Map = map(T2H - T2C, 0.0, MAX_TEMP_DIFF, MAX_FLOW_VALUE, MIN_FLOW_VALUE);

  // float sens1Map = MAX_FLOW_VALUE - ((T1H - T1C - MIN_TEMP_DIFF) / MAX_TEMP_DIFF);
  // float sens2Map = MAX_FLOW_VALUE - ((T2H - T2C - MIN_TEMP_DIFF) / MAX_TEMP_DIFF);

  // Test 3
  // float sens1Map = pow(E_NEP, -ALPHA * ((T1H - T1C - MIN_TEMP_DIFF) > 0 ? (T1H - T1C - MIN_TEMP_DIFF) : 0));
  // float sens2Map = pow(E_NEP, -ALPHA * ((T2H - T2C - MIN_TEMP_DIFF) > 0 ? (T2H - T2C - MIN_TEMP_DIFF) : 0));

  // Sens 1
  float range = MAX_RES_TEMP - T1C;
  float pivot = range / PIVOT_POINT;
  float tmp = T1H - T1C;

  float sens1Map;
  float sens2Map;
  if (tmp >= pivot)
  {
    sens1Map = pow(E_NEP, -ALPHA_RIGHT * (tmp - pivot));
    sens1Map = (sens1Map > 0.1) ? sens1Map : 0;
  }
  else
  {
    sens1Map = pow(E_NEP, -ALPHA_LEFT * (pivot - tmp));
  }

  // sens 2
  range = MAX_RES_TEMP - T2C;
  pivot = range / PIVOT_POINT;
  tmp = T2H - T2C;

  if (tmp >= pivot)
  {
    sens2Map = pow(E_NEP, -ALPHA_RIGHT * (tmp - pivot));
    sens2Map = (sens2Map > 0.1) ? sens2Map : 0;
  }
  else
  {
    sens2Map = pow(E_NEP, -ALPHA_LEFT * (pivot - tmp));
  }

  // serial_commands_.GetSerial()->println("Sensor 1: Hot->" + String(T1H) + " | Cold ->" + String(T1C));
  // serial_commands_.GetSerial()->println("Sensor 2: Hot->" + String(T2H) + " | Cold ->" + String(T2C));
  // serial_commands_.GetSerial()->println("----------------------MAPPED---------------------------");
  // serial_commands_.GetSerial()->println("Mapped sensor 1: " + String(sens1Map));
  // serial_commands_.GetSerial()->println("Mapped sensor 2: " + String(sens2Map));
  // serial_commands_.GetSerial()->println("-----------------------------------------------------");

  // delay(1000);

  GLOBAL_FLOW_RATE = (sens1Map + sens2Map) / 2;

  // serial_commands_.GetSerial()->println("Global flow: " + String(GLOBAL_FLOW_RATE));
  // serial_commands_.GetSerial()->println("-----------------------------------------------------");
  // delay(1000);

  // Remove out of bound values
  if (GLOBAL_FLOW_RATE < MIN_ALLOWED_FLOW)
  {
    GLOBAL_FLOW_RATE = MIN_FLOW_VALUE;
  }
  else if (GLOBAL_FLOW_RATE > MAX_FLOW_VALUE)
  {
    GLOBAL_FLOW_RATE = MAX_FLOW_VALUE;
  }
}

// READ AIR QUALITY SENSORS
void readAirQuality()
{
  // CCS811 sensor
  if (mySensor.dataAvailable())
  {
    mySensor.readAlgorithmResults();
    GLOBAL_CO2 = mySensor.getCO2();
    GLOBAL_TVOC = mySensor.getTVOC();
  }
}
// ------------------------------ SETUP ------------------------------
void setup()
{
  Serial.begin(9600);

  // Serial commands
  serial_commands_.AddCommand(&cmd_hello_);
  serial_commands_.AddCommand(&cmd_read_air_flow_);
  serial_commands_.AddCommand(&cmd_read_air_quality_);

  // Hardware setup
  pinMode(T1H_PIN, INPUT);
  pinMode(T1C_PIN, INPUT);
  pinMode(T2H_PIN, INPUT);
  pinMode(T2C_PIN, INPUT);

  // CCS811 setup
  Wire.begin(); // Inialize I2C Hardware

  if (mySensor.begin() == false)
  {
    serial_commands_.GetSerial()->println("CCS811 error. Please check wiring. Freezing...");
    while (1)
      ;
  }
}

// ------------------------------ LOOP ------------------------------
void loop()
{
  serial_commands_.ReadSerial();
  eventLoop(serial_commands_);
  readAirQuality();
}