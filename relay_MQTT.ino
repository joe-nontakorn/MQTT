#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Vansales";
const char* password = "0826269966";
const char* mqtt_server = "13.212.44.97";
const int mqtt_port = 1883;
const char* mqtt_topic_control = "relay2/control";
const char* mqtt_topic_state = "relay2/state";

const int relayPin = D2; // กำหนดขา Relay
const int ledPin = D1; // กำหนดขา LED

bool relayState = false; // เก็บสถานะปัจจุบันของ Relay (เปิด/ปิด)

WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (strcmp(topic, mqtt_topic_control) == 0) {
    // ส่วนสำหรับควบคุม Relay
    if (message == "ON") {
      digitalWrite(relayPin, LOW); // เปิด Relay
      digitalWrite(ledPin, LOW); // เปิด LED
      relayState = true; // อัปเดตสถานะเป็นเปิด
      Serial.println("Relay2 turned ON");
    } else if (message == "OFF") {
      digitalWrite(relayPin, HIGH); // ปิด Relay
      digitalWrite(ledPin, HIGH); // ปิด LED
      relayState = false; // อัปเดตสถานะเป็นปิด
      Serial.println("Relay2 turned OFF");
    }
      Serial.println("--------------------");


    // ส่งสถานะ Relay กลับไปยัง MQTT Broker
    if (client.connected()) {
      if (relayState) {
        client.publish(mqtt_topic_state, "ON");
      } else {
        client.publish(mqtt_topic_state, "OFF");
      }
    }
  }
}

void setup() {
  pinMode(relayPin, OUTPUT); // กำหนดขา Relay เป็น Output
  digitalWrite(relayPin, HIGH); // ปิด Relay ตอนเริ่มต้น
  pinMode(ledPin, OUTPUT); // กำหนดขา LED เป็น Output
  digitalWrite(ledPin, LOW); // ปิด LED ตอนเริ่มต้น
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.subscribe(mqtt_topic_control); // ติดตามควบคุม Relay จาก MQTT Broker
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_control);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
