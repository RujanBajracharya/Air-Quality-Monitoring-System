# Smart Systems Documentation

Status: Done
Subject: Smart Systems
Team: College

# ESP32:

I have used two ESP32. The first ESP32 is reading data from DHT11, BMP180 and MQ-135 sensors. The second ESP32 is reading data from PMS5003 sensor.

### Main:

![Untitled](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Untitled.png)

This is the code uploaded to the first ESP32. I have modified the example PubSubClient library code to publish data to Raspberry Pi using MQTT. Similarly, I also used the example ArduinoOTA code to enable me to upload my code On-The-Air. This was done because I didn’t have a USB port on my laptop.

Furthermore, I referenced Random Nerd Tutorials’ BMP32 guide/tutorial and DHT11/DHT22 guide (Random Nerd Tutorials, n.d.) for the wiring of BMP180 and DHT11 sensor.

Below is the connectivity table for the main ESP32:

| ESP32 | DHT11 | BMP180 | MQ-135 |
| --- | --- | --- | --- |
| D4 | Data | - | - |
| D21 | - | SDA | - |
| D22 | - | SCL | - |
| VP | - | - | AO (1.8k Resistor) |

![Screenshot 2023-12-06 at 19.04.03.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_19.04.03.png)

```cpp
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

const char* ssid = "";    //Add wifi configurations.
const char* password = "";

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
```

### PMS Sensor:

![Untitled](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Untitled%201.png)

For the PMS sensor, I referenced Mabel Lu’s blog on Tino IoT for wiring and retrieving sensor data (Mabel Lu, 2020). The reason this sensor is connected to a separate ESP32 is to comply with our floor plan in task 2.

# Raspberry Pi:

### MongoDB:

This is the first script used in raspberry pi. This script retrieves data from an MQTT message and stores it in a Mongo DB database. The MongoDB database is hosted on a macOS laptop, acting as the database server. This code is a modification of lab 4a lab code with code for mongo db insertion added (Kesterton, 2023).

### CosmosDB:

This python script is scheduled to run every 30 minutes using crontab on raspberry pi. This script retrieves all the data stored in MongoDB in the last 30 minutes and calculates the average, minimum and maximum of every parameter and inserts it into Azure Cosmos DB.

![Screenshot 2023-12-06 at 19.17.17.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_19.17.17.png)

# Azure IoT Central Applications:

To send the data to Azure IoT Central Applications from Raspberry Pi and create a dashboard, I followed the lab 10b instructions and made necessary changes along the way (Kesterton, 2023).

### Node-Red Configurations:

![Screenshot 2023-12-06 at 10.28.50.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_10.28.50.png)

---

### MQTT-In Node Configurations:

![Screenshot 2023-12-06 at 13.49.17.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.49.17.png)

![Screenshot 2023-12-06 at 13.49.22.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.49.22.png)

![Screenshot 2023-12-06 at 13.49.27.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.49.27.png)

![Screenshot 2023-12-06 at 13.49.13.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.49.13.png)

![Screenshot 2023-12-06 at 13.49.08.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.49.08.png)

![Screenshot 2023-12-06 at 13.49.04.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.49.04.png)

![Screenshot 2023-12-06 at 13.48.58.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.58.png)

---

### Change Node Configurations:

![Screenshot 2023-12-06 at 13.48.16.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.16.png)

![Screenshot 2023-12-06 at 13.48.21.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.21.png)

![Screenshot 2023-12-06 at 13.48.27.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.27.png)

![Screenshot 2023-12-06 at 13.48.47.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.47.png)

![Screenshot 2023-12-06 at 13.48.42.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.42.png)

![Screenshot 2023-12-06 at 13.48.37.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.37.png)

![Screenshot 2023-12-06 at 13.48.32.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.32.png)

---

### Last Change Node (msg.payload):

![Screenshot 2023-12-06 at 13.48.09.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.09.png)

![Screenshot 2023-12-06 at 13.48.01.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_13.48.01.png)

---

### IoT Central Applications Configurations:

![Screenshot 2023-12-06 at 14.08.48.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_14.08.48.png)

![Screenshot 2023-12-06 at 14.08.56.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_14.08.56.png)

![Azure IoT Central Applications - Dashboard.JPG.jpg](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Azure_IoT_Central_Applications_-_Dashboard.JPG.jpg)

![Screenshot 2023-12-06 at 20.57.43.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_20.57.43.png)

![Screenshot 2023-12-06 at 11.00.05.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_11.00.05.png)

![Screenshot 2023-12-06 at 20.57.58.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_20.57.58.png)

# Email Alert:

![Screenshot 2023-12-06 at 14.09.26.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_14.09.26.png)

![Screenshot 2023-12-06 at 14.09.36.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_14.09.36.png)

![Screenshot 2023-12-06 at 14.09.48.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_14.09.48.png)

![Screenshot 2023-12-06 at 19.41.27.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_19.41.27.png)

# MongoDB:

<aside>
💡 Note: The provided json export of the database may contain some test schemas for the first 100 results.

</aside>

![Screenshot 2023-12-06 at 19.42.44.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_19.42.44.png)

![Screenshot 2023-12-06 at 19.43.09.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_19.43.09.png)

# Cosmos DB:

<aside>
💡 Note: First few documents of the cosmos db exported data submitted with this file may contain test data.

</aside>

![Screenshot 2023-12-06 at 19.45.11.png](Smart%20Systems%20Documentation%20f4693fee0d0c46a598924a2a1533833e/Screenshot_2023-12-06_at_19.45.11.png)

# References:

Random Nerd Tutorials (n.d.) *ESP32 with BMP180 Barometric Sensor - Guide.* Available at: [https://randomnerdtutorials.com/esp32-with-bmp180-barometric-sensor/](https://randomnerdtutorials.com/esp32-with-bmp180-barometric-sensor/) [Accessed 24 November 2023]

Random Nerd Tutorials (n.d.) *ESP32 with DHT11/DHT22 Temperature and Humidity Sensor using Arduino IDE.* Available at: [https://randomnerdtutorials.com/esp32-with-bmp180-barometric-sensor/](https://randomnerdtutorials.com/esp32-with-bmp180-barometric-sensor/) [Accessed 2 December 2023]

Kesterton, R. (2023) The MQTT Protocol. [lab work] *CMP5324 Smart Systems AITA S3A 2022/3.* Faculty of Computing, Engineering and the Built Environment, Birmingham City University. Available through: [https://moodle.bcu.ac.uk/pluginfile.php/9402941/course/section/1455207/Smart Systems Lab 4a (1).pdf](https://moodle.bcu.ac.uk/pluginfile.php/9402941/course/section/1455207/Smart%20Systems%20Lab%204a%20%281%29.pdf) [Accessed 26 November 2023].

Kesterton, R. (2023) Visualising Sensor Data using Azure IoT. [lab work] *CMP5324 Smart Systems AITA S3A 2022/3.* Faculty of Computing, Engineering and the Built Environment, Birmingham City University. Available through: [https://moodle.bcu.ac.uk/pluginfile.php/9403077/mod_resource/content/2/lab10b.pdf](https://moodle.bcu.ac.uk/pluginfile.php/9403077/mod_resource/content/2/lab10b.pdf) [Accessed 29 November 2023].
