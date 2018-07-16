#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define VERSION "1.0.0"

#define WIFI_SSID "Fablab Karlsruhe"
#define WIFI_PASSWORD "foobar42"

#define MQTT_HOST IPAddress(192, 168, 1, 6)
#define MQTT_PORT 1883

#define CHANNEL_PREFIX String("/FLKA/labtivators")

#define Button1Pin D6
#define Button2Pin D5
#define Button1RedPin D3
#define Button1GreenPin D4
#define Button2RedPin D1
#define Button2GreenPin D2

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
String mac;

#define BUILD_TOPIC(postfix) (CHANNEL_PREFIX + postfix).c_str()
#define BUILD_MY_TOPIC(postfix) (CHANNEL_PREFIX + "/" + mac + postfix).c_str()

void turnButton1LedRed()
{
    digitalWrite(Button1RedPin, HIGH);
    digitalWrite(Button1GreenPin, LOW);
}
void turnButton2LedRed()
{
    digitalWrite(Button2RedPin, HIGH);
    digitalWrite(Button2GreenPin, LOW);
}

void turnButton1LedGreen()
{
    digitalWrite(Button1RedPin, LOW);
    digitalWrite(Button1GreenPin, HIGH);
}
void turnButton2LedGreen()
{
    digitalWrite(Button2RedPin, LOW);
    digitalWrite(Button2GreenPin, HIGH);
}

void turnButton1LedBlack()
{
    digitalWrite(Button1RedPin, LOW);
    digitalWrite(Button1GreenPin, LOW);
}
void turnButton2LedBlack()
{
    digitalWrite(Button2RedPin, LOW);
    digitalWrite(Button2GreenPin, LOW);
}

void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void connectToWifi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    mac = WiFi.macAddress();
}

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    Serial.println("Connected to Wi-Fi.");
    connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
    Serial.println("Disconnected from Wi-Fi.");
    mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");

    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    String topic1 = BUILD_MY_TOPIC("/cmd/setbutton1color");
    Serial.print("Subscribe to ");
    Serial.println(topic1);
    mqttClient.subscribe(topic1.c_str(), 2);

    String topic2 = BUILD_MY_TOPIC("/cmd/setbutton2color");
    Serial.print("Subscribe to ");
    Serial.println(topic2);
    mqttClient.subscribe(topic2.c_str(), 2);

    String topic3 = BUILD_MY_TOPIC("/cmd/ping");
    Serial.print("Subscribe to ");
    Serial.println(topic3);
    mqttClient.subscribe(topic3.c_str(), 2);

    uint16_t packetId = mqttClient.publish(BUILD_TOPIC("/discover"), 2, false, (mac + " v" + VERSION).c_str());
    Serial.print("publish discover to");
    Serial.print(BUILD_TOPIC("/discover"));
    Serial.print(" with packetId : ");
    Serial.println(packetId);

    turnButton1LedBlack();
    turnButton2LedBlack();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.print("Disconnected from MQTT. reason: ");
    if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
        Serial.println("TCP_DISCONNECTED");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
        Serial.println("MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
        Serial.println("MQTT_IDENTIFIER_REJECTED");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
        Serial.println("MQTT_SERVER_UNAVAILABLE");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
        Serial.println("MQTT_MALFORMED_CREDENTIALS");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
        Serial.println("MQTT_NOT_AUTHORIZED");
    } else if (reason == AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE) {
        Serial.println("ESP8266_NOT_ENOUGH_SPACE");
    } else if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
        Serial.println("TLS_BAD_FINGERPRINT");
    }

    if (WiFi.isConnected())
    {
        mqttReconnectTimer.once(2, connectToMqtt);
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    Serial.println("Message received.");
    Serial.print("topic: '");
    Serial.print(topic);
    Serial.println("'");
    Serial.print("payload: '");
    Serial.print(payload);
    Serial.println("'");

    String lowerCasePayload = String(payload);
    lowerCasePayload.toLowerCase();

    if (String(topic).equals(BUILD_MY_TOPIC("/cmd/setbutton1color")))
    {
        Serial.println("button1color");

        if (lowerCasePayload.startsWith("red")) {
            turnButton1LedRed();
        }
        else if (lowerCasePayload.startsWith("green"))
        {
            turnButton1LedGreen();
        }
        else
        {
            turnButton1LedBlack();
        }
    }
    else if (String(topic).equals(BUILD_MY_TOPIC("/cmd/setbutton2color")))
    {
        Serial.println("button2color");

        if (lowerCasePayload.startsWith("red"))
        {
            turnButton2LedRed();
        }
        else if (lowerCasePayload.startsWith("green"))
        {
            turnButton2LedGreen();
        }
        else
        {
            turnButton2LedBlack();
        }
    }
    else if (String(topic).equals(BUILD_MY_TOPIC("/cmd/ping")))
    {
        Serial.println("ping");

        mqttClient.publish(BUILD_MY_TOPIC("/stat/pong"), 2, false, "Pong!");
    }
}

void onMqttPublish(uint16_t packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    pinMode(Button1Pin, INPUT);
    pinMode(Button2Pin, INPUT);
    pinMode(Button1RedPin, OUTPUT);
    pinMode(Button1GreenPin, OUTPUT);
    pinMode(Button2RedPin, OUTPUT);
    pinMode(Button2GreenPin, OUTPUT);

    Serial.println("Boot animation");

    delay(100);
    turnButton1LedRed();
    turnButton2LedGreen();
    delay(100);
    turnButton1LedGreen();
    turnButton2LedRed();
    delay(100);
    turnButton1LedRed();
    turnButton2LedGreen();
    delay(100);
    turnButton1LedGreen();
    turnButton2LedRed();
    delay(100);
    turnButton1LedRed();
    turnButton2LedRed();
    delay(100);

    Serial.println("Setup");

    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();
}

void loop()
{/*
    if (digitalRead(Button1Pin) == LOW)
    {
        //Serial.println("Send button1 cmd");
        //mqttClient.publish(BUILD_MY_TOPIC("/cmd/button1"), 2, false);
    }
    else if (digitalRead(Button2Pin) == LOW)
    {
        //Serial.println("Send button2 cmd");
        //mqttClient.publish(BUILD_MY_TOPIC("/cmd/button2"), 2, false);
    }*/
}