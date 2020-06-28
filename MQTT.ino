#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


// Update these with values suitable for your network.
const char* ssid = "flex4";
const char* password = "nali12345";
const char* mqtt_server = "mqtt.eclipse.org";
#define mqtt_port 1883
#define MQTT_USER "eapcfltj"
#define MQTT_PASSWORD "3EjMIy89qzVn"
#define MQTT_SERIAL_PUBLISH_CH "EN3250/ESP32"
#define MQTT_SERIAL_RECEIVER_CH "BLEBeacon"

String device;
JSONVar Beacon;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

int scanTime = 3; //In seconds
BLEScan* pBLEScan;

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      //client.publish(MQTT_SERIAL_PUBLISH_CH, "hello world-mqtt");
      // ... and resubscribe
      //client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      
      //Serial.print("Advertised Device: ");
      //Serial.print(advertisedDevice.getAddress().toString().c_str());
      //publishSerialData((char*) advertisedDevice.getAddress().toString().c_str());
      device = advertisedDevice.getAddress().toString().c_str();
      Beacon[device]= advertisedDevice.getRSSI();
      
      //client.publish(MQTT_SERIAL_PUBLISH_CH, jsonString.c_str());
      //Serial.print("    RSSI: ");
      //Serial.println(advertisedDevice.getRSSI());
      //publishSerialData((char*)advertisedDevice.getRSSI());
    }
};

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

//# BLE
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}


void loop() {
 
  
  client.loop();
  Beacon = NULL;
  Beacon["id"] = 6;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Number of Devices: ");
  Serial.print(foundDevices.getCount());
  Serial.print("\n");
  String jsonString = JSON.stringify(Beacon);
  client.publish(MQTT_SERIAL_PUBLISH_CH, jsonString.c_str());
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(10);
 }
