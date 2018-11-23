#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <OpenTherm.h>

#include "User.h"

//OpenTherm input and output wires connected to 4 and 5 pins on the OpenTherm Shield
const int inPin = 15;  //D8
const int outPin = 13; //D7

//Data wire is connected to 14 pin on the OpenTherm Shield
#define ONE_WIRE_BUS 14

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
OpenTherm ot(inPin, outPin);
WiFiClient espClient;
PubSubClient client(espClient);
char buf[10];

float sp = 23, //set point
mt1 = 0,      //remote measured temperature 1
mt1_last = 0, //prior remote temperature 1
mt2 = 0,      //remote measured temperature 2
mt2_last = 0, //prior remote temperature 2
mt3 = 0,      //remote measured temperature 3
mt3_last = 0, //prior remote temperature 3
mt4 = 0,      //remote measured temperature 4
mt4_last = 0, //prior remote temperature 4
pv = 0,       //current temperature
pv_last = 0,  //prior temperature
ierr = 0,     //integral error
dt = 0,       //time between measurements
op = 0;       //PID controller output
unsigned long ts = 0, new_ts = 0; //timestamp


void handleInterrupt() {
  ot.handleInterrupt();
}

float getTemp() {
  return sensors.getTempCByIndex(0); 
}

float pid(float sp, float pv, float pv_last, float& ierr, float dt) {
  float Kc = 5.0; // K / %Heater
  float tauI = 50.0; // sec
  float tauD = 1.0;  // sec
  // PID coefficients
  float KP = Kc;
  float KI = Kc / tauI;
  float KD = Kc*tauD; 
  // upper and lower bounds on heater level
  float ophi = 50;
  float oplo = 9;
  // calculate the error
  float error = sp - pv;
  // calculate the integral error
  ierr = ierr + KI * error * dt;  
  // calculate the measurement derivative
  float dpv = (pv - pv_last) / dt;
  // calculate the PID output
  float P = KP * error; //proportional contribution
  float I = ierr; //integral contribution
  float D = -KD * dpv; //derivative contribution
  float op = P + I + D;
  // implement anti-reset windup
  if ((op < oplo) || (op > ophi)) {
    I = I - KI * error * dt;
    // clip output
    op = max(oplo, min(ophi, op));
  }
  ierr = I; 
  Serial.println("sp="+String(sp) + " pv=" + String(pv) + " dt=" + String(dt) + " op=" + String(op) + " P=" + String(P) + " I=" + String(I) + " D=" + String(D));
  return op;
}

void setup_wifi() {
  delay(10);
  //Connect to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {  
  Serial.begin(115200);
  setup_wifi();

  //Init DS18B20 Sensor
  sensors.begin();
  sensors.requestTemperatures();
  sensors.setWaitForConversion(false); //switch to async mode
  pv, pv_last = sensors.getTempCByIndex(0);
  ts = millis();

  //Init OpenTherm Controller
  ot.begin(handleInterrupt);

  //Init MQTT Client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void publish_temperature() {
  Serial.println("t=" + String(pv));    
  String(pv).toCharArray(buf, 10);
  client.publish("pv", buf);  
}

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic, "sp") == 0){
    String str = String();    
    for (int i = 0; i < length; i++) {
      str += (char)payload[i];
    }
    Serial.println("sp=" + str);  
    sp = str.toFloat();
  }
  if(strcmp(topic, "/ESP01_03_boti/DHT-22/Temperature") == 0){
    String str = String();    
    for (int i = 0; i < length; i++) {
      str += (char)payload[i];
    }
    Serial.println("mt1=" + str);  
    mt1 = str.toFloat();
  }
}

void reconnect() {  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      publish_temperature();
      // ... and resubscribe
      client.subscribe("sp");
      client.subscribe("/ESP01_03_boti/DHT-22/Temperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }    
  }
}

void loop(void) { 
  new_ts = millis();
  if (new_ts - ts > 1000) {   
    //Set/Get Boiler Status
    bool enableCentralHeating = true;
    bool enableHotWater = true;
    bool enableCooling = false;
    unsigned long response = ot.setBoilerStatus(enableCentralHeating, enableHotWater, enableCooling);
    OpenThermResponseStatus responseStatus = ot.getLastResponseStatus();
    if (responseStatus != OpenThermResponseStatus::SUCCESS) {
      Serial.println("Error: Invalid boiler response " + String(response, HEX));
    }   

//  pv = sensors.getTempCByIndex(0);
    pv = mt1;
    
    dt = (new_ts - ts) / 1000.0;
    ts = new_ts;
    if (responseStatus == OpenThermResponseStatus::SUCCESS) {
      op = pid(sp, pv, pv_last, ierr, dt);
      //Set Boiler Temperature
      ot.setBoilerTemperature(op);
      if (op < 10) {
        ot.buildRequest(OpenThermRequestType::WRITE, OpenThermMessageID::Status, 0x1);
      }
    }
    pv_last = pv;
    
    sensors.requestTemperatures(); //async temperature request
    
    publish_temperature();
  }
  
  //MQTT Loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
