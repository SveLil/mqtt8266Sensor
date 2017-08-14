#include <BoardConfiguration.h>
#include <ESP8266WiFi.h>
#include "DHTSensor.h"
#include "BME280Sensor.h"
#include <Esp.h>

const char currentVersion[] = "V0.2";
const char v01String[] = "V0.1";

WiFiClientSecure secureNet;
WiFiClient net;

BoardConfiguration::BoardConfiguration() {
  // Check status
  // 1: Not configured yet => Show Wifi BoardConfiguration page
  // 2: Wifi configured (and working?) => Connect to Wifi and show MQTT BoardConfiguration page
  // 3: MQTT confiugured => Normal startup. Keep Server running? Show status? Show BoardConfiguration page?
  char s[5];
  int loc = sizeof(currentVersion);
  Serial.begin(115200);
  Serial.println("SizeOf: "+String(loc)+"+" + String(sizeof(data)+"+"+String(sizeof(sensorConfig))));
  EEPROM.begin(4096);
  EEPROM.get(0, s);
  if (digitalRead(15) == HIGH) {
    Serial.println("Reset settings");
    wipe();
    delay(500);
  } else if (String(s) != String(currentVersion)) {
    if (String(s) == String(v01String)) {
      updateConfig01To02();
    } else {
      wipe();
    }
  } else {
    EEPROM.get(loc, data);
    loc += sizeof(data);
    Serial.println("Loaded");
    if (data.status > 1) {
    }
    if (data.sensorCount > 0) {
      for (int i = 0; i < data.sensorCount; i++) {
        EEPROM.get(loc, sensorConfig[i]);
        loc += sizeof(SensorConfiguration);
      }
      debugPrintSensorConfig();
    }
    debugPrintConfig(true, true, data.status > 1);
  }
  sensorsInitialized = false;
}

void BoardConfiguration::wipe() {
  data.wifiConfig.ssid[0] = 0;
  data.wifiConfig.password[0] = 0;
  data.sensorCount = 0;
  data.status = 0;
  data.mqttConfig.server[0] = 0;
  data.mqttConfig.user[0] = 0;
  data.mqttConfig.password[0] = 0;
  data.mqttConfig.baseTopic[0] = 0;
  data.mqttConfig.useSSL = false;
  data.mqttConfig.port = 0;
  data.mqttConfig.readInterval = 5;
  data.enableDeepSleep = false;
  saveSensorConfiguration();
}

void BoardConfiguration::updateConfig01To02() {
  ConfigurationStruct01 data01;
  int loc = sizeof(currentVersion);
  EEPROM.get(loc, data01);
  if (data01.sensorCount > 0 ) {
    for (int i = 0; i < data01.sensorCount; i++) {
      EEPROM.get(loc, sensorConfig[i]);
      loc += sizeof(SensorConfiguration);
    }
  }
  data.mqttConfig = data01.mqttConfig;
  data.sensorCount = data01.sensorCount;
  data.status = data01.status;
  data.enableDeepSleep = false;
  strcpy( data.wifiConfig.ssid, data01.wifiConfig.ssid);
  strcpy( data.wifiConfig.password, data01.wifiConfig.password );
  data.wifiConfig.enableAP = true;
  saveSensorConfiguration();
}

void BoardConfiguration::debugPrintSensorConfig() {
  for (int i = 0; i < data.sensorCount; i++) {
    String sensorType;
    if (sensorConfig[i].sensorType == DHT22_COMPATIBLE) {
      sensorType = "DHT22";
    } else if (sensorConfig[i].sensorType == SIMPLE_ANALOG) {
      sensorType = "Analog";
    } else if (sensorConfig[i].sensorType == SIMPLE_DIGITAL) {
      sensorType = "Digital";
    }
    Serial.println("sensorConfig["+String(i)+"].pin : " + String(sensorConfig[i].pin));
    Serial.println("sensorConfig["+String(i)+"].sensorName : " + String(sensorConfig[i].sensorName));
    Serial.println("sensorConfig["+String(i)+"].sensorType : " + sensorType);
  }
}

void BoardConfiguration::debugPrintConfig(bool printData, bool printWifi, bool printMQTT) {
  if (printData) {
    Serial.println("data.status : " + String(data.status));
    Serial.println("data.sensorCount : " + String(data.sensorCount));
    Serial.println("data.enableDeepSleep : " + String(data.enableDeepSleep));
  }
  if (printWifi) {
    Serial.println("data.wifiConfig.ssid : " + String(data.wifiConfig.ssid));
    Serial.println("data.wifiConfig.password : " + String(data.wifiConfig.password));
    Serial.println("data.wifiConfig.enableAP : " + String(data.wifiConfig.enableAP));
  }
  if (printMQTT) {
    Serial.println("data.mqttConfig.server : " + String(data.mqttConfig.server));
    Serial.println("data.mqttConfig.port : " + String(data.mqttConfig.port));
    Serial.println("data.mqttConfig.ssl : " + String(data.mqttConfig.useSSL));
    Serial.println("data.mqttConfig.user : " + String(data.mqttConfig.user));
    Serial.println("data.mqttConfig.password : " + String(data.mqttConfig.password));
    Serial.println("data.mqttConfig.baseTopic : " + String(data.mqttConfig.baseTopic));
    Serial.println("data.mqttConfig.readInterval : " + String(data.mqttConfig.readInterval));
  }
}

void BoardConfiguration::save() {
  int loc = sizeof(currentVersion);
  EEPROM.put(0, currentVersion);
  EEPROM.put(loc,data);
  EEPROM.commit();
  Serial.println("Saved");
  debugPrintConfig(true, true, true);
}


void BoardConfiguration::saveSensorConfiguration() {
  save();
  int loc = sizeof(currentVersion);
  loc += sizeof(data);
  for (int i = 0; i < data.sensorCount; i++) {
    EEPROM.put(loc, sensorConfig[i]);
    loc += sizeof(SensorConfiguration);
  }
  EEPROM.commit();
  debugPrintSensorConfig();
  Serial.println("Saved sensorConfig");
}


BoardConfiguration& BoardConfiguration::getInstance() {
  static BoardConfiguration BoardConfiguration;
  return BoardConfiguration;
}

void BoardConfiguration::saveWifiConfiguration(const String& s_ssid, const String& s_password, const bool enableAP) {
  Serial.println("Saving WiFi configuration");
  s_ssid.toCharArray(data.wifiConfig.ssid,32);
  s_password.toCharArray(data.wifiConfig.password,64);
  data.wifiConfig.enableAP = enableAP;
  if (data.status < 1) {
    data.status = 1;
    data.mqttConfig.server[0] = 0;
    data.mqttConfig.user[0] = 0;
    data.mqttConfig.password[0] = 0;
    data.mqttConfig.baseTopic[0] = 0;
    data.mqttConfig.useSSL = false;
    data.mqttConfig.port = 0;
    data.mqttConfig.readInterval = 5;
  }
  save();
}

void BoardConfiguration::saveMQTTConfiguration(const String& s_server, const int port, const bool useSSL, const String& s_user, const String& s_password, const String& s_baseTopic, const int readInterval, const bool enableDeepSleep) {
  Serial.println("Saving MQTT configuration");
  s_server.toCharArray(data.mqttConfig.server,256);
  data.mqttConfig.port=port;
  data.mqttConfig.useSSL=useSSL;
  data.enableDeepSleep = enableDeepSleep;
  s_user.toCharArray(data.mqttConfig.user,64);
  s_password.toCharArray(data.mqttConfig.password,64);
  s_baseTopic.toCharArray(data.mqttConfig.baseTopic,256);
  data.mqttConfig.readInterval=readInterval;
  if (data.status < 2) {
    data.status = 2;
  }
  save();
}

bool BoardConfiguration::connectToWifi() {
  if (data.status < 1) {
    return false;
  }
  WiFi.mode(WIFI_STA);
  Serial.println("Configuration found, connecting to " + String(data.wifiConfig.ssid));
  WiFi.begin(data.wifiConfig.ssid, data.wifiConfig.password);
  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startMillis < 10000)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  return true;
}

bool BoardConfiguration::connectToMQTT(PubSubClient &client) {
  if (data.status < 2) {
    return false;
  }
  if (client.loop()) {
      return true;
  } else {
    Serial.println("Board says non connected, state is: "+ String(client.state()));
  }
  if (data.mqttConfig.useSSL) {
    client.setClient(secureNet);
  } else {
    client.setClient(net);
  }
  client.setServer(data.mqttConfig.server, data.mqttConfig.port);
  Serial.println("Connect MQTT");
  connectedToMQTT = client.connect(String(ESP.getChipId()).c_str(), data.mqttConfig.user, data.mqttConfig.password);
  Serial.println("Connection: " + String(connectedToMQTT));
  delay(100);
  client.loop();
  return connectedToMQTT;
}

ConfigurationStruct BoardConfiguration::getConfig() {
  return data;
}

Sensor* getSensor(const SensorConfiguration &sConfig) {
  switch (sConfig.sensorType) {
    case DHT22_COMPATIBLE:
        Serial.println("Created DHT22 sensor");
        return new DHTSensor(sConfig);
      case BME280:
        Serial.println("Created BME280 sensor");
        return new BME280Sensor(sConfig);
  }
  Serial.println("Unknown sensor: " + String(sConfig.sensorType) + " not: " + String(DHT22_COMPATIBLE));
  return NULL;
}

void BoardConfiguration::initSensors(int index=-1) {
  if (index>-1 && !sensorsInitialized) {
    //If sensors aren't initilized yet, init all
    index = -1;
  }
  if (index > -1) {
    Serial.println("Init for index: " + String(index));
    delete sensors[index];
    SensorConfiguration sConfig = sensorConfig[index];
    sensors[index] = getSensor(sConfig);
  } else {
    if (sensorsInitialized) {
      Serial.println("Delete old array of "+ String(createdSensorCount) + " sensors");
      for (int i = 0; i<createdSensorCount; i++) {
        delete sensors[i];
      }
      if (createdSensorCount > 0) {
        delete[] sensors;
      }
    }
    Serial.println("Creating new sensor array for "+ String(data.sensorCount) + " sensors");
    Serial.flush();
    delay(500);
    sensors = new Sensor*[data.sensorCount];
    for (int i = 0; i<data.sensorCount; i++) {
      SensorConfiguration sConfig = sensorConfig[i];
      Serial.println("create sensor: " + i);
      sensors[i] = getSensor(sConfig);
      createdSensorCount++;
    }
    Serial.println("Return");
  }
  sensorsInitialized = data.sensorCount > 0;
}

Sensor** BoardConfiguration::getSensors() {
  if (!sensorsInitialized) {
    Serial.println("!sensorsInitialized");
    initSensors();
  } else {
    Serial.println("sensorsInitialized");
  }
  return sensors;
}
int BoardConfiguration::getSensorCount() {
  return data.sensorCount;
}

int BoardConfiguration::saveSensorConfiguration( int sensorId, const SensorType& sensorType,const int pin, const String& sensorName) {
  if (sensorId < 0 || sensorId >= getSensorCount()) {
    if (data.sensorCount >= 16) {
      Serial.println("Too many sensors, exiting");
      return -1;
    }
    Serial.println("New sensor for pin" + String(pin) + " of type " + String(sensorType));
    Serial.println("Existing sensors: " + String(data.sensorCount));
    //New sensor
    sensorConfig[data.sensorCount].pin = pin;
    sensorConfig[data.sensorCount].sensorType = sensorType;
    sensorName.toCharArray(sensorConfig[data.sensorCount].sensorName,256);

    Serial.println("Set new data");
    sensorId = data.sensorCount;
    data.sensorCount++;
    Serial.println("now " + String(data.sensorCount) + " sensors");
  } else {
    //Existing sensor
    sensorConfig[sensorId].pin = pin;
    sensorConfig[sensorId].sensorType = sensorType;
  }
  saveSensorConfiguration();
  if (sensorsInitialized) {
    initSensors(sensorId);
  }
  return sensorId;
}

bool BoardConfiguration::deleteSensorConfiguration(const int sensorId) {
  if (sensorId < 0 || sensorId >= getSensorCount()) {
    return false;
  } else {
    //Sensor exists
    data.sensorCount = data.sensorCount - 1;
    // Shift left existing sensors
    for (int i = sensorId; i < data.sensorCount; i++) {
      sensorConfig[i].sensorType = sensorConfig[i+1].sensorType;
      sensorConfig[i].pin = sensorConfig[i+1].pin;
      std::copy(std::begin(sensorConfig[i+1].sensorName), std::end(sensorConfig[i+1].sensorName), std::begin(sensorConfig[i].sensorName));
    }
    sensorConfig[data.sensorCount].sensorType = NO_SENSOR;
    sensorConfig[data.sensorCount].sensorName[0] = 0;
    sensorConfig[data.sensorCount].pin = -1;
    saveSensorConfiguration();
    return true;
  }
}

SensorConfiguration BoardConfiguration::getSensorConfig(const int sensorId) {
  if (sensorId < 0 || sensorId >= getSensorCount()) {
    Serial.println("Index out of range (was: "+String(sensorId)+", max: "+String(getSensorCount())+")");
    Serial.flush();
    panic();
  } else {
    return sensorConfig[sensorId];
  }
}
