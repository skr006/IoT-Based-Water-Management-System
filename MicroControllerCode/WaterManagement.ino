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
const char* root_ca = "-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";

const char* certificate = "-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUd8r0c+c3OTzlklo/ce3XIEblkUwwDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTAwMjE1NTgw\n"
"MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALr7Urv9xuvXCxk/WktL\n"
"P9aUTK/nPG1f4RJUfnV/qK+tn+ZTTxW3J6htHz372vAwIHjQqyLg5/Xx5DbCXJV6\n"
"zpGyBNuovBWRo8vnem2ElSmw48iatKwzwzBah0QrKZyDc07Gp/T1hXnR5VCwNit5\n"
"cIKiA3r0Ec3/d+1klij50NJxZG3TYpj/ATpDZbs8Cz9KhgPQTgxTOv78rz+dN2w4\n"
"B92IC4Ffk6/flPIK+qZRxR7mk8zAaJJOGA91BAaaaQci/NvLwVuE2Jgvz++3Z4Rb\n"
"K7N5IPAFrPp9ITas2PxzBPQFJKLsKjTjM7oextz0jtNSLxfIZHjh6iyPWF4Lenxm\n"
"HhsCAwEAAaNgMF4wHwYDVR0jBBgwFoAUD+47+mnKvawRQd+vRamyHG6o4KgwHQYD\n"
"VR0OBBYEFIL8tTXIEwT5NFLANbmZSHcvMrtHMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQC4ahFKpxdtq0w7Kx2WKpwmZCLy\n"
"3eZfU7B/AaMu4P1A1GVje78mCaooqu7vnfOgkE04p3fGGz4si6Y7ZLfkO2xTqtY9\n"
"GbVPj+JcRzmMg2OLomRAewZeofX35j8IUq6bypjF1mKbH6tg7+h/bSLUZoLItAOc\n"
"0sfwdE6EIdIfKq4Rfkf54z32FzpyeJyK7uEHyFyfIdWpLKxoGbxNPzGNrMG1KmUB\n"
"w+cPTa2d0CpkElqej0UnLmgSN63kx2lFBsF165woYHDuVLypuPbXq+m0bYp/pckS\n"
"yhtMNeG5YaJBe/xpwXdK/Bqpe2583FdIIfsbishhS1wDFiJgCaiXGXsHvuiM\n"
"-----END CERTIFICATE-----\n";

const char* private_key = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAuvtSu/3G69cLGT9aS0s/1pRMr+c8bV/hElR+dX+or62f5lNP\n"
"FbcnqG0fPfva8DAgeNCrIuDn9fHkNsJclXrOkbIE26i8FZGjy+d6bYSVKbDjyJq0\n"
"rDPDMFqHRCspnINzTsan9PWFedHlULA2K3lwgqIDevQRzf937WSWKPnQ0nFkbdNi\n"
"mP8BOkNluzwLP0qGA9BODFM6/vyvP503bDgH3YgLgV+Tr9+U8gr6plHFHuaTzMBo\n"
"kk4YD3UEBpppByL828vBW4TYmC/P77dnhFsrs3kg8AWs+n0hNqzY/HME9AUkouwq\n"
"NOMzuh7G3PSO01IvF8hkeOHqLI9YXgt6fGYeGwIDAQABAoIBAQCCdaQuIkzOdEX8\n"
"IaXAbwpljydKfA4/Sexhu65YEPADUyMDsxC02AvFig2IU9dYSlv9r6oYNc/iXBmc\n"
"01OBwCOsqAaXtE02x1z/gcx76UuhwRfM+ZQhqiP/8sn6GBt6ZoTna9f4I+4zJu3W\n"
"tHu3LY4At0WyXeSmVB669bmHcXXvvZbHtstJ/sBpHsc4Hkv2l74A+nluDcL01+Rp\n"
"n+37WENitDPavRu+Y/o0soMbYBJyc4Gf71WN9v3XQ+y9T1MXnSDYluAWuiXY9wM9\n"
"vokLwwi+AkpGkdiVe5NH25rKG5dypoewZhGtvwLXchtW45GI06puymLUjchX/9SD\n"
"J7vTmnqRAoGBAPU5oBbnYAWGZ54nUAWd2Wwiv1qM+rpjWEJLx+PnXfTsuiNrTlRi\n"
"oBtPcpORqf64vl0rtFpdT9mTf98DG4Kd4PekkpOomIIcw7ZXSKxZdsTtnmtZmk7P\n"
"/7sPD8R/kRCL/8eIZuVWEEDTuy2+mikp0Li8eWZmfc/TTaQBkd7BtUf/AoGBAMMy\n"
"jokVUe743vgIsxWJahad5Fbp74vHggVe2oNBolSg+JkeZ2JURSYUECJ5+5kDn2JP\n"
"7gFYbdMXUbBYBcGLgjjeohpf+Tj10xFRB5njVLSy9YChq/3AUVhirGqeJa1tJjyP\n"
"PabYqWd4gZz8uRmexasPG4V5NfoXSD2ky8GUS0nlAoGBAJ8jUFLMwDGVsXSfCn06\n"
"nLhiLE/1IVkH02TijMvHf9/MKFeH8YMtpb1TZ4WAiBM3jMIl9JqX55PI9rig6Z9w\n"
"GWhcMosEZEQ7qn6QUicWv864bZA4y7zNmqWn9pGMP3W/hLPWPwAhQ9buEJB1GtIP\n"
"wtW/6gCcDpVqCP6fziwDBUO9AoGAVPEgM6kycWKdbKQkgj+9CcCFyAie9F3KR9oC\n"
"DtI1+MvfwooHhTOUr8BVURtyECESb67GrJE1oYFpS+rR/mpeLa88LWwMQOxAgC2Z\n"
"uC7EAqpJyUMRXCgffe3ah77wOGriE8JlmsB3344CITWcuvhrnkfBuzCG+6C8EA8x\n"
"l7iXlr0CgYBl1tqrWJ0vXQtPd+pkUmV5qbfxEtyiRwqgQZnOLv4oGu0fzV9UrRGF\n"
"pXAyI938gTzYkQHpy2oBgsNNv+75OK5eddW7mrq/KYQ7uULUOOt40vBxovLB7IH2\n"
"xiOYaahzuEfXC8Ag98vdWPTOotFAj1XTLTNl5++kTCQw+k6PFTAMlQ==\n"
"-----END RSA PRIVATE KEY-----\n";
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
