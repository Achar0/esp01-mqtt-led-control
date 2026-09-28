/*
  ALL THE WORK IS USABLE ONLY IF THE TOPIC OF THE INPUT IS "led/control"!
  ALL THE CREDENTIALS ARE IN AN EXTERNAL FILE CALLED "secrets.h".
  THE JSON INPUT SHOULD BE ALWAYS LIKE THIS:
  
  {"message": "status"}

  WITH status = 1 THAT MEANS THAT THE LED SHOULD BE ON, AND status = 0 WHEN THE LED SHOULD BE OFF.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Include the ArduinoJson library
#include "secrets.h"      // Include external credentials for connection

// Pins
const int ledPin = 2;  // Built-in LED on ESP01 (GPIO2)

// Wi-Fi and MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200); // The communication is in 115200 baud
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);  // Ensure LED starts off (active-low)

  connectToWiFi();

  client.setServer(BROKER_Server, BROKER_Port);
  client.setCallback(mqttCallback);

  // Flash LED while connecting to MQTT Broker
  while (!client.connected()) {
    digitalWrite(ledPin, !digitalRead(ledPin));  // Toggle LED state
    delay(500);

    String clientId = "ESP8266Client-" + WiFi.macAddress();  // Use MAC address for unique client ID

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT Broker");
      Serial.print("ESP-01 MAC Address: ");
      Serial.println(WiFi.macAddress());
      client.subscribe(BROKER_Topic);  // Subscribe to the "led/control" topic
      client.publish("led/status", "Hello from ESP-01");  // Example publish
    } else {
      Serial.print("MQTT connection failed, state: ");
      Serial.println(client.state());
      delay(2000);  // Retry every 2 seconds
    }
  }

  digitalWrite(ledPin, HIGH);  // Ensure LED is off after successful connection
}

// Ensure if the connection is lost, it will enter into a loop to reconnect
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Log the topic and message
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(message);

  // Check if the topic is 'led/control' and handle the message accordingly
  if (String(topic) == BROKER_Topic) {
    // Parse JSON payload
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
      Serial.println("Failed to parse JSON");
      return;  // If the message is not valid JSON, exit the function
    }

    // Extract the "message" field from JSON
    const char* jsonMessage = doc["message"];
    if (jsonMessage) {
      String command = String(jsonMessage);
      Serial.print("Extracted JSON message: ");
      Serial.println(command);

      // Handle the LED control based on the extracted message
      if (command == "1") {
        digitalWrite(ledPin, LOW);  // Turn LED on
        Serial.println("LED ON");
        client.publish("led/status", "LED is ON");  // Publish status
      } else if (command == "0") {
        digitalWrite(ledPin, HIGH);  // Turn LED off
        Serial.println("LED OFF");
        client.publish("led/status", "LED is OFF");  // Publish status
      } else {
        Serial.println("Invalid message received.");
      }
    } else {
      Serial.println("JSON format is missing the 'message' key.");
    }
  } else {
    Serial.println("Message received from unknown topic, ignoring.");
  }
}

void connectToWiFi() {
  Serial.print("Connecting to primary WiFi...");
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    digitalWrite(ledPin, !digitalRead(ledPin));  // Toggle LED state
    delay(500);
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to primary WiFi");
  } else {
    Serial.println("\nFailed to connect to primary WiFi. Trying secondary WiFi...");
    WiFi.begin(SECRET_SSID2, SECRET_PASS2);
    retries = 0;

    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      digitalWrite(ledPin, !digitalRead(ledPin));  // Toggle LED state
      delay(500);
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to secondary WiFi");
    } else {
      Serial.println("\nFailed to connect to both WiFi networks");
    }
  }
  digitalWrite(ledPin, HIGH);  // Ensure LED is off after connection
}

void reconnectMQTT() {
  static unsigned long lastReconnectAttempt = 0;  // To track time since the last attempt
  const unsigned long reconnectInterval = 5000;   // Retry every 5 seconds

  if (millis() - lastReconnectAttempt >= reconnectInterval) {
    Serial.println("Attempting MQTT connection...");
    lastReconnectAttempt = millis();

    String clientId = "ESP8266Client-" + WiFi.macAddress();  // Use MAC address for unique client ID

    if (client.connect(clientId.c_str())) {
      Serial.println("Reconnected to MQTT Broker");
      client.subscribe(BROKER_Topic);  // Subscribe to the "led/control" topic after reconnection
    } else {
      Serial.print("MQTT connection failed, state: ");
      Serial.println(client.state());
      // No delay here, to avoid blocking the loop for too long
    }
  }
}
