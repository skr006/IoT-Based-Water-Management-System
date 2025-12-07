#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = YOUR_SSID;
const char* password = YOUR_PASSWORD;

// Ultrasonic sensor pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// AWS IoT Core MQTT details
#define AWS_IOT_ENDPOINT "a3f7jh0r0h6hlu-ats.iot.us-east-1.amazonaws.com"
#define CLIENT_ID "ESP32-Ultrasonic"
#define TOPIC "water/level"


float latitude = 10.9038;   
float longitude = 76.8984;  

// Certificates
const char* root_ca = ;

const char* certificate = ;

const char* private_key = ;
WiFiClientSecure net;
PubSubClient client(net);

// Function to connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// Function to connect to AWS IoT
void connectAWS() {
  net.setCACert(root_ca);
  net.setCertificate(certificate);
  net.setPrivateKey(private_key);

  client.setServer(AWS_IOT_ENDPOINT, 8883);

  while (!client.connected()) {
    Serial.println("Connecting to AWS IoT...");
    if (client.connect(CLIENT_ID)) {
      Serial.println("Connected to AWS IoT!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5s");
      delay(5000);
    }
  }
}

// Function to read distance from ultrasonic sensor
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1; // no echo
  return (duration * 0.0343) / 2; // distance in cm
}

// Send sensor data to AWS
void sendToAWS(float distance) {
  if (!client.connected()) {
    connectAWS();
  }

  StaticJsonDocument<256> doc;
  doc["device_id"] = CLIENT_ID;
  doc["distance_cm"] = distance;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;
  doc["timestamp"] = millis();

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  client.publish(TOPIC, jsonBuffer);
  Serial.println("Data sent: " + String(jsonBuffer));
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  connectWiFi();
  connectAWS();
}

void loop() {
  client.loop();

  float distance = getDistance();
  if (distance != -1) {
    Serial.print("Distance: ");
    Serial.println(distance);
    sendToAWS(distance);
  } else {
    Serial.println("No valid reading.");
  }

  delay(30000); // send every 30s
}
