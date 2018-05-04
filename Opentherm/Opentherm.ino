// Usefull links:
// https://github.com/ihormelnyk/arduino_opentherm_controller
// http://ihormelnyk.com/Content/Pages/arduino_opentherm_controller/Opentherm%20Protocol%20v2-2.pdf
// http://otgw.tclcode.com/schematic.html
// https://www.domoticaforum.eu/viewforum.php?f=75

// set according to own wiring
const int OT_IN_PIN = 15;   //D8
const int OT_OUT_PIN = 13;  //D7
const unsigned int bitPeriod = 1020; //1020 //microseconds, 1ms -10%+15%

// a string to hold incoming serial data
String inputString = "";

// serial input delimiter
const char delimiter = ' ';

// message types sent to boiler
enum MasterToSlaveMsgType : byte
{
  READ_DATA = 0,
  WRITE_DATA = 1,
  INVALID_DATA = 2,
  RESERVED = 3
};

// message types received from boiler
enum SlaveToMasterMsgType : byte
{
  READ_ACK = 4,
  WRITE_ACK = 5,
  DATA_INVALID = 6,
  UNKNOWN_DATAID = 7
};

// message direction
enum DataIdDirection : byte
{
  DIR_READ,
  DIR_WRITE,
  DIR_BOTH
};

// message data type
enum DataType : byte
{
  DT_FLAG8,
  DT_U8,
  DT_S8,
  DT_F88,
  DT_U16,
  DT_S16
};


enum DataId : byte
{
  R_STATUS          = 0,
  W_TSET            = 1,
  R_TBOILER         = 25
                      // to be extended if usefull
};

#define MESSAGES_COUNT 54

// message definition
typedef struct
{
  DataId dataId;
  DataIdDirection dir;
  char dataObject[32];
  DataType dataType;
  char description[100];
} OTMessage;

// messages data
OTMessage messages[MESSAGES_COUNT] = {
  {(DataId)0, DIR_READ,   "Status", DT_FLAG8, "Master and Slave Status flags."},
  {(DataId)1, DIR_WRITE,  "TSet", DT_F88, "Control setpoint ie CH water temperature setpoint (°C) "},
  {(DataId)2, DIR_WRITE,  "M-Config / M-MemberIDcode", DT_FLAG8, "Master Configuration Flags / Master MemberID Code "},
  {(DataId)3, DIR_READ,   "S-Config / S-MemberIDcode", DT_FLAG8, "Slave Configuration Flags / Slave MemberID Code "},
  {(DataId)4, DIR_WRITE,  "Command", DT_U8, "Remote Command "},
  {(DataId)5, DIR_READ,   "ASF-flags / OEM-fault-code", DT_FLAG8, "Application-specific fault flags and OEM fault code "},
  {(DataId)6, DIR_READ,   "RBP-flags", DT_FLAG8, "Remote boiler parameter transfer-enable & read/write flags "},
  {(DataId)7, DIR_WRITE,  "Cooling-control", DT_F88, "Cooling control signal (%) "},
  {(DataId)8, DIR_WRITE,  " TsetCH2", DT_F88, "Control setpoint for 2e CH circuit (°C) "},
  {(DataId)9, DIR_READ,   "TrOverride", DT_F88, "Remote override room setpoint "},
  {(DataId)10, DIR_READ,  "TSP", DT_U8, "Number of Transparent-Slave-Parameters supported by slave "},
  {(DataId)11, DIR_BOTH,  "TSP-index / TSP-value", DT_U8, "Index number / Value of referred-to transparent slave parameter."},
  {(DataId)12, DIR_READ,  "FHB-size", DT_U8, "Size of Fault-History-Buffer supported by slave "},
  {(DataId)13, DIR_READ,  "FHB-index / FHB-value", DT_U8, "Index number / Value of referred-to fault-history buffer entry."},
  {(DataId)14, DIR_WRITE, "Max-rel-mod-level-setting", DT_F88, "Maximum relative modulation level setting (%) "},
  {(DataId)15, DIR_READ,  "Max-Capacity / Min-Mod-Level", DT_U8, "Maximum boiler capacity (kW) / Minimum boiler modulation level(%) "},
  {(DataId)16, DIR_WRITE, "TrSet", DT_F88, "Room Setpoint (°C) "},
  {(DataId)17, DIR_READ,  "Rel.-mod-level", DT_F88, "Relative Modulation Level (%) "},
  {(DataId)18, DIR_READ,  "CH-pressure", DT_F88, "Water pressure in CH circuit (bar) "},
  {(DataId)19, DIR_READ,  "DHW-flow-rate", DT_F88, "Water flow rate in DHW circuit.(litres/minute) "},
  {(DataId)20, DIR_BOTH,  "Day-Time", DT_U8, "Day of Week and Time of Day "},
  {(DataId)21, DIR_BOTH,  "Date", DT_U8, "Calendar date "},
  {(DataId)22, DIR_BOTH,  "Year", DT_U16, "Calendar year "},
  {(DataId)23, DIR_WRITE, "TrSetCH2", DT_F88, "Room Setpoint for 2nd CH circuit (°C) "},
  {(DataId)24, DIR_WRITE, "Tr", DT_F88, "Room temperature (°C) "},
  {(DataId)25, DIR_READ,  "Tboiler", DT_F88, "Boiler flow water temperature (°C) "},
  {(DataId)26, DIR_READ,  "Tdhw", DT_F88, "DHW temperature (°C) "},
  {(DataId)27, DIR_READ,  "Toutside", DT_F88, "Outside temperature (°C) "},
  {(DataId)28, DIR_READ,  "Tret", DT_F88, "Return water temperature (°C) "},
  {(DataId)29, DIR_READ,  "Tstorage", DT_F88, "Solar storage temperature (°C) "},
  {(DataId)30, DIR_READ,  "Tcollector", DT_F88, "Solar collector temperature (°C)"},
  {(DataId)31, DIR_READ,  "TflowCH2", DT_F88, "Flow water temperature CH2 circuit (°C) "},
  {(DataId)32, DIR_READ,  "Tdhw2", DT_F88, "Domestic hot water temperature 2 (°C) "},
  {(DataId)33, DIR_READ,  "Texhaust", DT_S16, "Boiler exhaust temperature (°C) "},
  {(DataId)48, DIR_READ,  "TdhwSet-UB / TdhwSet-LB", DT_S8, "DHW setpoint upper & lower bounds for adjustment (°C) "},
  {(DataId)49, DIR_READ,  "MaxTSet-UB / MaxTSet-LB", DT_S8, "Max CH water setpoint upper & lower bounds for adjustment (°C) "},
  {(DataId)50, DIR_READ,  "Hcratio-UB / Hcratio-LB", DT_S8, "OTC heat curve ratio upper & lower bounds for adjustment"},
  {(DataId)56, DIR_BOTH,  "TdhwSet", DT_F88, "DHW setpoint (°C)"},
  {(DataId)57, DIR_BOTH,  "MaxTSet", DT_F88, "Max CH water setpoint (°C) (Remote parameters 2) "},
  {(DataId)58, DIR_BOTH,  "Hcratio", DT_F88, "OTC heat curve ratio (°C) (Remote parameter 3) "},
  {(DataId)100, DIR_READ, "Remote override function", DT_FLAG8, "Function of manual and program changes in master and remote room setpoint."},
  {(DataId)115, DIR_READ, "OEM diagnostic code", DT_U16, "OEM-specific diagnostic/service code "},
  {(DataId)116, DIR_BOTH, "Burner starts", DT_U16, "Number of starts burner "},
  {(DataId)117, DIR_BOTH, "CH pump starts", DT_U16, "Number of starts CH pump "},
  {(DataId)118, DIR_BOTH, "DHW pump/valve starts", DT_U16, "Number of starts DHW pump/valve "},
  {(DataId)119, DIR_BOTH, "DHW burner starts", DT_U16, "Number of starts burner during DHW mode "},
  {(DataId)120, DIR_BOTH, "Burner operation hours", DT_U16, "Number of hours that burner is in operation (i.e.flame on) "},
  {(DataId)121, DIR_BOTH, "CH pump operation hours", DT_U16, "Number of hours that CH pump has been running "},
  {(DataId)122, DIR_BOTH, "DHW pump/valve operation hours", DT_U16, "Number of hours that DHW pump has been running or DHW valve has been opened "},
  {(DataId)123, DIR_BOTH, "DHW burner operation hours", DT_U16, "Number of hours that burner is in operation during DHW mode "},
  {(DataId)124, DIR_WRITE,"OpenTherm version Master", DT_F88, "The implemented version of the OpenTherm Protocol Specification in the master."},
  {(DataId)125, DIR_READ, "OpenTherm version Slave", DT_F88, "The implemented version of the OpenTherm Protocol Specification in the slave."},
  {(DataId)126, DIR_WRITE,"Master-version", DT_U8, "Master product version number and type "},
  {(DataId)127, DIR_READ, "Slave-version", DT_U8, "Slave product version number and type"}
};

uint8_t status_activate_CH = 0x1;
uint8_t status_activate_DHW = 0x2;
uint16_t status_activate_CH_DHW = (status_activate_CH | status_activate_DHW) << 8;
uint32_t time= 0L;
 

// checks if value has even parity
int hasEvenParity(unsigned int x)
{
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return (~x) & 1;
}

// converts float to uint16
uint16_t toF88(float f)
{
  uint16_t result = 0;
  if (f >= 0.0f) {
    result = f * 256;
  } else {
    // f = (-1.0f * (0x10000 - b) / 256.0f);
    // b = 0x1000 + f * 256
    result = 0x10000 + f * 256;
  }

  return result;
}

// converts uint16 to float
float fromF88(uint16_t b)
{
  if (b & 0x8000) // sign bit
  {
    //negative
    return float (-1.0f * (0x10000 - b) / 256.0f);
  } else {
    //positive
    return (float)(b / 256.0f);
  }
}

//P MGS-TYPE SPARE DATA-ID  DATA-VALUE
//0 000      0000  00000000 00000000 00000000
// builds message from msg type, data id and value to uint32=unsigned long
unsigned long buildRequest(MasterToSlaveMsgType msgType, DataId dataId, uint16_t dataValue)
{
  //These bits are unused in this release of the protocol. They should always be ‘0’.
  uint8_t spare = 0;
  if (dataId == 0) {
    msgType = READ_DATA;
  }
  unsigned long request = (dataValue) + (dataId << 16) + (spare << 24) + (msgType << 28);
  if (!hasEvenParity(request))
    request |= 1 << 31;
  return request;
}

// builds read request
unsigned long buildReadRequest(DataId dataId, uint16_t v)
{
  return buildRequest(READ_DATA, dataId, v);
}

// builds write request
unsigned long buildWriteRequest(DataId dataId, uint16_t dataValue)
{
  return buildRequest(WRITE_DATA, dataId, dataValue);
}

// sets idle state
void setIdleState() {
  digitalWrite(OT_OUT_PIN, HIGH);
}

// sets active state
void setActiveState() {
  digitalWrite(OT_OUT_PIN, LOW);
}

// activates communication with boiler
void activateBoiler() {
  setIdleState();
  delay(1000);
  //readId(0, value);
}

void sendBit(bool high) {
  if (high) setActiveState(); else setIdleState();
  delayMicroseconds(500);
  if (high) setIdleState(); else setActiveState();
  delayMicroseconds(500);
}

void sendFrame(unsigned long request) {
  sendBit(HIGH); //start bit
  for (int i = 31; i >= 0; i--) {
    sendBit(bitRead(request, i));
  }
  sendBit(HIGH); //stop bit
  setIdleState();
}

void printBinary(unsigned long val, int bits) {
  for (int i = bits - 1; i >= 0; i--) {
    Serial.print(bitRead(val, i));
  }
}

unsigned long sendRequest(unsigned long request) {
  Serial.print("Request:  ");
  printBinary(request, 32);
  Serial.print(" / ");
  Serial.print(request, HEX);
  Serial.println();
  sendFrame(request);

  if (!waitForResponse()) return 0;

  return readResponse();
}

bool waitForResponse() {
  unsigned long time_stamp = micros();
  while (digitalRead(OT_IN_PIN) != HIGH) { //start bit
    if (micros() - time_stamp >= 1000000) {
      Serial.println("Response timeout");
      return false;
    }
  }
  delayMicroseconds(bitPeriod * 1.25); //wait for first bit
  return true;
}

unsigned long readResponse() {
  unsigned long response = 0;
  for (int i = 0; i < 32; i++) {
    response = (response << 1) | digitalRead(OT_IN_PIN);
    delayMicroseconds(bitPeriod);
  }
  Serial.print("Response: ");
  printBinary(response, 32);
  Serial.print(" / ");
  Serial.print(response, HEX);
  Serial.println("");

  if (hasEvenParity(response))
    Serial.println("Parity bit: OK");
  else
    Serial.println("Parity bit: WRONG");

  return response;
}

// -----------------------------------------------------------------------------------

void setup() {
  pinMode(OT_IN_PIN, INPUT);
  pinMode(OT_OUT_PIN, OUTPUT);
  setActiveState();
  Serial.begin(115200);
  Serial.println("Start");
}

// -----------------------------------------------------------------------------------

void loop() {
  loopSerial();
  delay(10);
  if (millis()-time > 900) {
	time = millis();
	readId(25);
  }
}

// -----------------------------------------------------------------------------------

void loopSerial()
{
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:

    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' || inChar == '\r') {
      Serial.println(inputString);

      uint16_t v = 0;
      int id = -1;

      String cmd = getValue(inputString, delimiter, 0);
      if (cmd == "OpenthermRead" || cmd == "r" || cmd == "OpenthermWrite" || cmd == "w") {
        id = getValue(inputString, delimiter, 1).toInt();
        if ((cmd == "OpenthermRead" || cmd == "r") && id == 0 || cmd == "OpenthermWrite" || cmd == "w") {
          String str = getValue(inputString, delimiter, 2);
          switch (messages[getIdIdx(id)].dataType)
          {
            case DT_FLAG8:
              v = (uint16_t)str.toInt();
              break;
            case DT_U8:
              v = (uint16_t)str.toInt();
              break;
            case DT_S8:
              v = (uint16_t)str.toInt();
              break;
            case DT_F88:
              // TODO: parse float
              v = toF88(str.toInt());
              break;
            case DT_U16:
              v = (uint16_t)str.toInt();
              break;
            case DT_S16:
              v = (int16_t)str.toInt();
              break;
            default:
              Serial.println("Unknown type!");
              return;
              break;
          }
        }
      }

      if (cmd == "init") {
        Serial.println("Initializing...");
        initialize();
        Serial.println("Initialized.");
      } if (cmd == "reset" || cmd == "restart") {
#ifdef ESP32
        Serial.println("Restarting ESP...");
        ESP.restart();
#endif
      } else if (cmd == "OpenthermRead" || cmd == "r") {
		time = millis();
        readId(id, v);		
      } else if (cmd == "OpenthermWrite" || cmd == "w") {
		time = millis();
        writeId(id, v);		
      }
      inputString = "";
    } else {
      inputString += inChar;
    }	
  }
}

// -----------------------------------------------------------------------------------

int getIdIdx(int id) {
  for (int i = 0; i < MESSAGES_COUNT; i++) {
    if (messages[i].dataId == id)
      return i;
  }
  return -1;
}

void readId(int id, uint16_t v)
{
  int idx = getIdIdx(id);
  if (idx < 0) {
    Serial.println("id= " + String(id) + " does not exist");
    return;
  }

  readIdx(idx, v);
}

void readIdx(int idx, uint16_t v)
{
  int i = idx;

  if (1 || messages[i].dir == DIR_READ || messages[i].dir == DIR_BOTH) {
    Serial.println("DataId: " + String(messages[i].dataId));
    Serial.println("Dir: " + String(messages[i].dir));
    Serial.println("DataObject: " + String(messages[i].dataObject));
    Serial.println("DataType: " + String(messages[i].dataType));
    Serial.println("Description: " + String(messages[i].description));

    unsigned long request = buildReadRequest(messages[i].dataId, v);
    unsigned long response = sendRequest(request);

    if (response != 0L) {
      SlaveToMasterMsgType responseMsgType = (SlaveToMasterMsgType)((response >> 28) & 0x7);

      Serial.println("ResponseMsgType: " + String(responseMsgType));

      if (responseMsgType == DATA_INVALID) {
        Serial.println("DATA_INVALID");
      } else if (responseMsgType == UNKNOWN_DATAID) {
        Serial.println("UNKNOWN_DATAID");
      } else if (responseMsgType == READ_ACK) {
        if (messages[i].dataId == 48 || messages[i].dataId == 49) {
          Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String((int8_t)response & 0xFF) + " " + String((int8_t)((response & 0xFF00) >> 8)));
        } else {
          uint16_t dataValue = response & 0xFFFF;

          Serial.println("dataValue: ");

          switch (messages[i].dataType)
          {
            case DT_FLAG8:
              printBinary(dataValue, 8);
              Serial.println();
              Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String((uint8_t)dataValue));
              break;
            case DT_U8:
              Serial.println((uint8_t)dataValue);
              Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String((uint8_t)dataValue));
              break;
            case DT_S8:
              Serial.println((int8_t)dataValue);
              Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String((int8_t)dataValue));
              break;
            case DT_F88:
              Serial.println(fromF88(dataValue));
              Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String(fromF88(dataValue)));
              break;
            case DT_U16:
              Serial.println(dataValue);
              Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String((uint16_t)dataValue));
              break;
            case DT_S16:
              Serial.println((int16_t)dataValue);
              Serial.println("OpenthermReadResponse " + String(messages[i].dataId) + " " + String((int16_t)dataValue));
              break;
            default:
              Serial.println("Unknown type!");
              break;
          }

          if (messages[i].dataId == 0)
          {
            if (bitRead(dataValue, 0)) {
              Serial.println("fault");
            } else {
              //Serial.println("no fault");
            }
            if (bitRead(dataValue, 1)) {
              Serial.println("CH active");
            } else {
              //Serial.println("CH not active");
            }
            if (bitRead(dataValue, 2)) {
              Serial.println("DHW active");
            } else {
              //Serial.println("DHW not active");
            }
            if (bitRead(dataValue, 3)) {
              Serial.println("flame on");
            } else {
              //Serial.println("flame off");
            }
            if (bitRead(dataValue, 4)) {
              Serial.println("cooling mode active");
            } else {
              //Serial.println("cooling mode not active");
            }
            if (bitRead(dataValue, 5)) {
              Serial.println("CH2 active");
            } else {
              //Serial.println("CH2 not active");
            }
            if (bitRead(dataValue, 6)) {
              Serial.println("diagnostics event");
            } else {
              //Serial.println("no diagnostics");
            }
            //if (dataValue && 0x128) { Serial.println("reserved"); } else { Serial.print("reserved"); }
          }
        }
      }
      delay(950);
    }
    Serial.println();
  } else {
    Serial.println("id= " + String(messages[i].dataId) + " is not to read");
  }
}

void writeId(int id, uint16_t v)
{
  int idx = getIdIdx(id);
  if (idx < 0) {
    Serial.println("id= " + String(id) + " does not exist");
    return;
  }

  writeIdx(idx, v);
}

void writeIdx(int idx, uint16_t v)
{
  int i = idx;

  if (1 || messages[i].dir == DIR_WRITE || messages[i].dir == DIR_BOTH || messages[i].dataId == 0) {
    Serial.println("DataId: " + String(messages[i].dataId));
    Serial.println("Dir: " + String(messages[i].dir));
    Serial.println("DataObject: " + String(messages[i].dataObject));
    Serial.println("DataType: " + String(messages[i].dataType));
    Serial.println("Description: " + String(messages[i].description));

    unsigned long request = buildWriteRequest(messages[i].dataId, v);
    unsigned long response = sendRequest(request);

    if (response != 0L) {
      SlaveToMasterMsgType responseMsgType = (SlaveToMasterMsgType)((response >> 28) & 0x7);

      Serial.println("ResponseMsgType: " + String(responseMsgType));

      if (responseMsgType == DATA_INVALID) {
        Serial.println("DATA_INVALID");
      } else if (responseMsgType == UNKNOWN_DATAID) {
        Serial.println("UNKNOWN_DATAID");
      } else if (responseMsgType == WRITE_ACK || messages[i].dataId == 0 && responseMsgType == READ_ACK)
      {
        uint16_t dataValue = response & 0xFFFF;

        Serial.println("dataValue: ");

        switch (messages[i].dataType)
        {
          case DT_FLAG8:
            printBinary(dataValue, 16);
            Serial.println();
            break;
          case DT_U8:
            Serial.println((uint8_t)dataValue);
            break;
          case DT_S8:
            Serial.println((int8_t)dataValue);
            break;
          case DT_F88:
            Serial.println(fromF88(dataValue));
            break;
          case DT_U16:
            Serial.println(dataValue);
            break;
          case DT_S16:
            Serial.println((int16_t)dataValue);
            break;
          default:
            Serial.println("Unknown type!");
            break;
        }
      }
      delay(950);
    }
    Serial.println();
  } else {
    Serial.println("id= " + String(messages[i].dataId) + " is not to read");
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// does initialize from serial input command
void initialize()
{
  activateBoiler();
}
