#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "AITM-LAB";
const char* password = "aitm@lab";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

const char* mqtt_user = "";
const char* mqtt_password = "";

WiFiClient espClient;
PubSubClient client(espClient);

#define LENG 31   // 0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value = 0;
int PM2_5Value = 0;
int PM10Value = 0;

unsigned long lastTime = 0;
unsigned long timerDelay = 6000;

void setup() {
    Serial.begin(9600);
		Serial.println("Booting");
		  WiFi.mode(WIFI_STA);
		  
		  WiFi.begin(ssid, password);
		  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		    Serial.setDebugOutput(true);
		    Serial.println("Connection Failed! Rebooting...");
		    delay(2000);
		    ESP.restart();
		    
		  }
		client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  while(!client.connected()) {
    Serial.print("TRying to reconnect");

    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

    if (Serial.find(0x42)) {
        Serial.readBytes(buf, LENG);

        if (buf[0] == 0x4d) {
            if (checkValue(buf, LENG)) {
                PM01Value = transmitPM01(buf);  // count PM1.0 value of the air detector module
                PM2_5Value = transmitPM2_5(buf);  // count PM2.5 value of the air detector module
                PM10Value = transmitPM10(buf);
            }
        }
    }

    const char* pmsTopic1 = "esp32/rujanHospital/pms1.0";
		const char* pmsTopic2 = "esp32/rujanHospital/pms2.5";
    const char* pmsTopic10 = "esp32/rujanHospital/pms10.0";

		char pmsBuffer1[10]; 
    dtostrf(PM01Value, 4, 2, pmsBuffer1);  // Convert the float to a string
    // Serial.println(pmsBuffer);

    char pmsBuffer2[10];  
    dtostrf(PM2_5Value, 4, 2, pmsBuffer2);

    char pmsBuffer10[10];  
    dtostrf(PM10Value, 4, 2, pmsBuffer10);

		client.publish(pmsTopic1, pmsBuffer1);
    client.publish(pmsTopic2, pmsBuffer2);
    client.publish(pmsTopic10, pmsBuffer10);

    delay(2000);
}

char checkValue(unsigned char *thebuf, char leng) {
    char receiveflag = 0;
    int receiveSum = 0;

    for (int i = 0; i < (leng - 2); i++) {
        receiveSum = receiveSum + thebuf[i];
    }
    receiveSum = receiveSum + 0x42;

    if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) {
        receiveSum = 0;
        receiveflag = 1;
    }
    return receiveflag;
}

// transmit PM Value to PC
int transmitPM01(unsigned char *thebuf) {
    int PM01Val;
    PM01Val = ((thebuf[3] << 8) + thebuf[4]);
    return PM01Val;
}

int transmitPM2_5(unsigned char *thebuf) {
    int PM2_5Val;
    PM2_5Val = ((thebuf[5] << 8) + thebuf[6]);
    return PM2_5Val;
}

int transmitPM10(unsigned char *thebuf) {
    int PM10Val;
    PM10Val = ((thebuf[7] << 8) + thebuf[8]);
    return PM10Val;
}