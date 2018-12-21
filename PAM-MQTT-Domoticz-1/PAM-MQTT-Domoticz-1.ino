#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "MySSID";
const char* password = "MyPassword";
const char* mqtt_server = "mqtt.broker.com";
#define LED D4
#define MYIDX 1528

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
char macAddr[18];
char mqttClient[25];

void setup_wifi() {
  delay(10);
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

void actionOn () {
  digitalWrite(LED,LOW);
}

void actionOff () {
  digitalWrite(LED,HIGH);
}

void errorOccurred (String err) {
  Serial.println(err);
}

void callback(char* topic, byte* payload, unsigned int length) {
  DynamicJsonBuffer jsonBuffer(512);
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    errorOccurred("Parse json failed");
  } else {
    int idx = root["idx"];
    if (idx==MYIDX) {
      if (root["nvalue"]==0) {
        actionOff();
      } else {
        actionOn();
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    sprintf(mqttClient,"ESP%s",macAddr);
    Serial.println(mqttClient);
    if (client.connect(mqttClient)) {
      Serial.println("connected");
      client.publish("outTopic","hello world");
      client.subscribe("domoticz/out");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
  byte ar[6];
  WiFi.macAddress(ar);
  sprintf(macAddr,"%02X%02X%02X%02X%02X%02X",ar[0],ar[1],ar[2],ar[3],ar[4],ar[5]);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

