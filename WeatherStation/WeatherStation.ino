/*
 Weather Station
 Midgard Team Weather Station
 */

/****************************** Sensors Code *******************************/
#include <Wire.h> //I2C needed for sensors
#include "SparkFunMPL3115A2.h" //Pressure sensor
#include "SparkFunHTU21D.h" //Humidity sensor

MPL3115A2 myPressure; //Create an instance of the pressure sensor
HTU21D myHumidity; //Create an instance of the humidity sensor

//Hardware pin definitions
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;

const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;

//Global Variables
//long lastSecond; //The millis counter to see when a second rolls by

/******************************** Wifi Code ********************************/
//Wifi connection libraries
#include <SPI.h>
#include <WiFi.h>

char ssid[] = "TURBONETT_727C84"; // network SSID (name)
char pass[] = "6c4908a8a6";    // network password
int keyIndex = 1;            // network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
char server[] = "iotgatewayro.azurewebsites.net"; // DNS of the server

// Initialize the Ethernet client library
WiFiClient client;

unsigned long lastConnectionTime = 0; // last time connected to the server, in milliseconds
const unsigned long postingInterval = 20L * 1000L; // delay between updates, in milliseconds

void setup() {
  Serial.begin(9600);

  /*************** Wifi **************/
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, keyIndex, pass);
    
    // wait 5 seconds for connection:
    delay(5000);
  }
  // connected to wifi
  printWifiStatus();

  /*************** Sensors **************/
  pinMode(STAT_BLUE, OUTPUT); //Status LED Blue
  pinMode(STAT_GREEN, OUTPUT); //Status LED Green

  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  //Configure the pressure sensor
  myPressure.begin(); // Get sensor online
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags

  //Configure the humidity sensor
  myHumidity.begin();

  Serial.println("Weather Shield online!");
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {

    //Get metrics
    float humidity = get_humidity();
    float temp_h = get_humidity_temp();
    float pressure = get_pressure();
    float temp_p = get_pressure_temp();
    float light_lvl = get_light_level();
    float batt_lvl = get_battery_level();

    String data[] = {
      ",\"Humidity\": \"" + String(humidity, 3) + "\"",
      ",\"TemperatureH\": \"" + String(temp_h, 3) + "\"",
      ",\"Pressure\": \"" + String(pressure, 3) + "\"",
      ",\"TemperatureP\": \"" + String(temp_p, 3) + "\"",
      ",\"Light\": \"" + String(light_lvl, 3) + "\"",
      ",\"Battery\": \"" + String(batt_lvl, 3) + "\"",
    };
    
    httpRequest(data, 6);
  }
}

// this method makes a HTTP connection to the server:
void httpRequest(String data[], int array_size) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  //Processing data
  String device = "\"name\":\"midgard\"";
  int body_length = device.length() + 2; // opening and closing brackets of the json
  //String test = "{" + device;
  for(int i=0; i < array_size; i++){
    body_length += data[i].length();
  }
  
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("\nconnected, sending data ...");
    
    client.println("POST /api/IoTGateway HTTP/1.1");
    client.println("Host: iotgatewayro.azurewebsites.net");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.print(body_length);
    //client.print(test.length());
    client.println("\r\n");
    //client.println(test);
    client.print("{" + device);
    Serial.print("{" + device);
    for(int i=0; i < array_size; i++){
      client.print(data[i]);
      Serial.print(data[i]);
    }
    client.println("}");
    Serial.println("}");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/******************** Sensors functions *********************/
//Reads humidity sensor
float get_humidity(){
  float humidity = myHumidity.readHumidity();
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.print("%,");

  return (humidity);
}

//Gets temperature from humidity sensor
float get_humidity_temp(){
  float temp_h = myHumidity.readTemperature();
  Serial.print(" temp_h = ");
  Serial.print(temp_h, 2);
  Serial.print("C,");

  return (temp_h);
}

//Reads pressure sensor
float get_pressure(){
  float pressure = myPressure.readPressure();
  Serial.print(" Pressure = ");
  Serial.print(pressure);
  Serial.print("Pa,");

  return (pressure);
}

//Gets temperature from perssure sensor
float get_pressure_temp(){
  float temp_p = myPressure.readTempF();
  Serial.print(" temp_p = ");
  Serial.print(temp_p, 2);
  Serial.print("F,");

  return (temp_p);
}

//Returns the voltage of the light sensor based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
float get_light_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);

  float lightSensor = analogRead(LIGHT);

  operatingVoltage = 3.3 / operatingVoltage; //The reference voltage is 3.3V

  lightSensor = operatingVoltage * lightSensor;

  Serial.print(" light_lvl = ");
  Serial.print(lightSensor);
  Serial.print("V,");

  return (lightSensor);
}

//Returns the voltage of the raw pin based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
//Battery level is connected to the RAW pin on Arduino and is fed through two 5% resistors:
//3.9K on the high side (R1), and 1K on the low side (R2)
float get_battery_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);

  float rawVoltage = analogRead(BATT);

  operatingVoltage = 3.30 / operatingVoltage; //The reference voltage is 3.3V

  rawVoltage = operatingVoltage * rawVoltage; //Convert the 0 to 1023 int to actual voltage on BATT pin

  rawVoltage *= 4.90; //(3.9k+1k)/1k - multiple BATT voltage by the voltage divider to get actual system voltage

  Serial.print(" VinPin = ");
  Serial.print(rawVoltage);
  Serial.print("V");
  
  return (rawVoltage);
}
