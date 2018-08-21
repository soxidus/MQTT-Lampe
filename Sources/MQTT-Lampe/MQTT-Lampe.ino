#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>

// WiFi config
#define SSID ""
#define PSK ""
#define BROKER ""

// MQTT config
#define MQTT_PORT 1883
#define MQTT_DEVICE_NAME "Tetris-Lampe"
#define MQTT_QUEUE "Tetris/control"

const int relayPin = D1;

// MQTT runtime
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Lampe
int state = LOW;

void setup() {
  yield();
  Serial.begin(115200);
  delay(1000);

  // attach relay
  pinMode(relayPin, OUTPUT);

  // Connect to WiFi network
  Serial.print("\nWiFi connecting...");
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Use this URL to connect: http://");
  Serial.println(WiFi.localIP());

  client.setServer(BROKER, MQTT_PORT);
  client.setCallback(mqttCallback);
}


void reconnect() {
  yield();
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_DEVICE_NAME)) {
      Serial.println("connected");
      // subscribe to MQTT queue
      client.subscribe(MQTT_QUEUE);
    } else {
      Serial.print("mqtt connect failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  yield();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


void setLamp(int newstate) {
  Serial.print("Turning ");
  Serial.println(newstate ? "on" : "off");
  digitalWrite(relayPin, newstate);
  state = newstate;
}

void mqttCallback(const char* topic, byte* payload, unsigned int length) {
  // Komfortjalousiensteuerung will be the only queue that will hit this,
  // because we subscribed for nothing else...
  yield();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
    Serial.print((char) payload[i]);
  Serial.println();

  char* msg = (char *) payload;
  msg[length] = '\0'; // this seems necessary...
  if (strcmp(msg, "1") == 0) {
    setLamp(HIGH);
  } else if (strcmp(msg, "0") == 0 ) {
    setLamp(LOW);
  } else if (strcmp(msg, "toggle") == 0) {
    setLamp( state == HIGH ? LOW : HIGH );
  } else {
    Serial.print("Got payload that isn't a meaningful command: ");
    Serial.println(msg);
  }
}


