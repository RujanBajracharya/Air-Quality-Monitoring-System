#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include "DHT.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

#define LENG 31   // 0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

unsigned long lastTime = 0;
unsigned long timerDelay = 60000;

Adafruit_BMP085 bmp;

const char* ssid = "AITM-LAB";
const char* password = "aitm@lab";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

const char* mqtt_user = "";
const char* mqtt_password = "";

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// the setup loop's purpose is just to ensure connectivity and that your wifi is working
void setup() {
    Serial.begin(115200);
    Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.setDebugOutput(true);
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
    
  }

  // Port defaults to 3232
  ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  client.setServer(mqtt_server, mqtt_port);

  dht.begin();

  if (!bmp.begin()) {
	  Serial.println("Failed to detect BMP180, check wiring!");
	  while (1) {}
  }
}

void reconnect() {
  while(!client.connected()) {
    Serial.print("Trying to reconnect");

    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  ArduinoOTA.handle();
   if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = bmp.readTemperature();
  float p = bmp.readPressure();

  if (isnan(h)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(1000);
    return;
  }
    int MQ135_data = analogRead(A0);

    char humidityBuffer[10];
    dtostrf(h, 4, 2, humidityBuffer);  // Convert the float to a string
    Serial.println("Humidity Buffer: ");
    Serial.println(humidityBuffer);

    char temperatureBuffer[10];
    dtostrf(t, 4, 2, temperatureBuffer);
    Serial.println("temperature Buffer: ");
    Serial.println(temperatureBuffer);

    char pressureBuffer[10];
    dtostrf(p, 4, 2, pressureBuffer);
    Serial.println("pressure Buffer: ");
    Serial.println(pressureBuffer);

    char gasBuffer[10];  
    dtostrf(MQ135_data, 4, 2, gasBuffer);
    Serial.println("gas Buffer: ");
    Serial.println(gasBuffer);

    const char* humidityTopic = "esp32/rujanHospital/hum";
    const char* temperatureTopic = "esp32/rujanHospital/temp";
    const char* pressureTopic = "esp32/rujanHospital/pre";
    const char* co2Topic = "esp32/rujanHospital/gas";

    client.publish(humidityTopic, humidityBuffer);
    delay(2000);
    client.publish(temperatureTopic, temperatureBuffer);
    delay(2000);
    client.publish(pressureTopic, pressureBuffer);
    delay(2000);
    client.publish(co2Topic, gasBuffer);
    delay(3000);
}