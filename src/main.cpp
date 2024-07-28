// M5ATOM TP-Link SmartPlug Control Switch
//
// Requirement: HW
//  - M5Atom Lite
//  - TP-Link SmartPlug HS105
//
// Requirement: SW
//  - PlatformIO on VSCode
//  - Libraries
//    - M5Atom
//    - FastLED
//    - PubsubClient
//  - TPLinkSmartPlug.h from
//  https://github.com/mnakai3/WioTerminal_CO2_Controller/tree/master

#include <FastLED.h>
#include <M5Atom.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "TPLinkSmartPlug.h"

//-- 自分の環境設定用ファイル
#include "../../include/wifiinfo.h"

//-- 自分の環境に合わせて変更
const char* ssid = MY_WIFI_SSID;
const char* wifiPassword = MY_WIFI_PASSWORD;
const char* smartplugIpAddr = MY_TPLINK_PLUG3;
const char* ntpServer1 = MY_NTP_SERVER1;
const char* ntpServer2 = MY_NTP_SERVER2;  // Optional
const char* ntpServer3 = MY_NTP_SERVER3;  // Optional
const char* mqttServer = MY_MQTT_BROKER;
const char* thisMqttTopic = "smartplug3";
//--

boolean timerEnable = true;     // 組み込みタイマーを使わないならここはfalseに
const char* onTime  = "19:15";  // On time: 19:15
const char* offTime = "01:00";  // Off time: 01:00

CRGB dispColor(uint8_t r, uint8_t g, uint8_t b) {
    return (CRGB)((r << 16) | (g << 8) | b);
}
boolean switchIsOn = false;
const int retryMax = 20;  // WiFi retry limit (sec)

//-- Datetime related
const long gmtOffset_sec = 3600 * 9;  // JST (UTC+9)
const int daylightOffset_sec = 3600 * 0;  // 夏時間オフセット(日本は無し)
struct tm timeInfo;
uint16_t nowYear;
uint8_t nowMon;
uint8_t nowDay;
uint8_t nowHour;
uint8_t nowMin;
uint8_t nowSec;
char nowHM[5];  // "HH:MM"

//-- WiFi related
WiFiClient espClient;
WiFiUDP udp;
PubSubClient mqttClient(espClient);
TPLinkSmartPlug smartplug;

void smartplug_off() {
    switchIsOn = false;
    Serial.printf("SmartPulg off\n");
    M5.dis.drawpix(0, dispColor(0, 0, 71));  // Off: Dark Blue
    if (!smartplug.setRelayState(false))
        Serial.printf("setRelayState(false) = failed\n");
}

void smartplug_on() {
    switchIsOn = true;
    Serial.printf("SmartPulg ON\n");
    M5.dis.drawpix(0, dispColor(0, 127, 0));  // On: Green
    if (!smartplug.setRelayState(true))
        Serial.printf("setRelayState(true) = failed\n");
}

void callback(char* topic, byte* payload, unsigned int length) {
    char msgText[256];
    int i;

    for (i = 0; i < (int)length; i++) {
        msgText[i] = (char)payload[i];
    }
    msgText[(int)length] = 0;

    Serial.printf("## called: ");
    Serial.printf(",%d,", length);
    Serial.printf(msgText);
    if (strcmp(msgText, "ON") == 0) {
        Serial.printf("ON\n");
        switchIsOn = true;
        smartplug_on();
    } else if (strcmp(msgText, "OFF") == 0) {
        Serial.printf("OFF\n");
        switchIsOn = false;
        smartplug_off();
    }
}

void reconnect() {
    // Loop until reconnected
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "M5ATOM-";
        clientId += String(random(0xffff), HEX);
        //// Attempt to connect
        if (mqttClient.connect(clientId.c_str())) {
            // Serial.println("connected");
            //// ... and subscribe to topic
            mqttClient.subscribe(thisMqttTopic);
            Serial.print("Connected\n");
            delay(1000);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.print(" try again in 5 seconds\n");
            //// Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    int secondPassed = 0;

    M5.begin(true, false, true);
    M5.dis.drawpix(0, dispColor(0, 0, 0));  // LED off
    Serial.printf("trying to connect to AP\n");
    WiFi.mode(WIFI_STA);
    if (WiFi.begin(ssid, wifiPassword) != WL_DISCONNECTED) {
        M5.dis.drawpix(0, dispColor(127, 0, 0));  // Red
        delay(500);
        ESP.restart();
    }
    while (WiFi.status() != WL_CONNECTED) {
        // Trying to connect: Blink with orange
        M5.dis.drawpix(0, dispColor(127, 48, 0));  // Orange
        delay(500);
        M5.dis.drawpix(0, dispColor(0, 0, 0));  // Black
        delay(500);
        secondPassed += 1;
        if (secondPassed > 20) {
            M5.dis.drawpix(0, dispColor(127, 0, 0));  // Red
            delay(500);
            ESP.restart();
        }
    }
    Serial.printf("WiFi conected\n");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2,
               ntpServer3);
    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setCallback(callback);

    smartplug.begin(espClient, udp);
    smartplug.setTarget(smartplugIpAddr);

    smartplug_off();
}

void loop() {
    M5.update();
    getLocalTime(&timeInfo);
    nowHour = timeInfo.tm_hour;
    nowMin = timeInfo.tm_min;
    sprintf(nowHM, "%02d:%02d", (int)timeInfo.tm_hour, (int)timeInfo.tm_min);

    if (M5.Btn.wasPressed()) {
        switchIsOn = !switchIsOn;
        if (switchIsOn) {
            smartplug_on();
            //            Serial.printf("%02d:%02d\n", nowHour, nowMin);
            Serial.printf("%s\n", nowHM);
        } else {
            smartplug_off();
            //            Serial.printf("%02d:%02d\n", nowHour, nowMin);
            Serial.printf("%s\n", nowHM);
        }
    }
    if (timerEnable && (switchIsOn == false) && (strcmp(onTime, nowHM) == 0)) {
        smartplug_on();
    }
    if (timerEnable && (switchIsOn == true) && (strcmp(offTime, nowHM) == 0)) {
        smartplug_off();
    }

    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.loop();
}