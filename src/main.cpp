#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <RGBWWLedControl.h>
#include <MQTTDebugger.h>
#include <MqttIdentify.h>


#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"

#define SENSORNAME "LedStripControl" //change this to whatever you want to call your device

//============================================================================
// Multiwifi definition
//============================================================================
#if defined (ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#endif

#if defined (ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#endif
//============================================================================

WiFiClient espClient;
PubSubClient pubSubClient(espClient);

MqttIdentify mqttIdentify(SENSORNAME);


char msg[50];
int value = 0;

#define BLUEPIN D7
#define REDPIN D6 
#define GREENPIN D5
// #define BLUEPIN D6
// #define REDPIN D5 
// #define GREENPIN D7
#define WHITE1PIN 0
#define WHITE2PIN 0

RGBWWLedControl rgbwwLedControl(REDPIN, GREENPIN, BLUEPIN, WHITE1PIN, WHITE2PIN);

// ICACHE_RAM_ATTR void detectsButton1() {
//   Serial.println("button one pressed");
//   //client.publish(SENSORNAME + "/ledControl", "{\"state\": \"ON\"}");
// }

// ICACHE_RAM_ATTR void detectsButton2() {
//   Serial.println("button two pressed");
//   //client.publish(SENSORNAME + "/ledControl", "{\"state\": \"OFF\"}");
// }

void connectWifi(){
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    // Serial.print("WiFi connected: ");
    // Serial.print(WiFi.SSID());
    // Serial.print(" ");
    // Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi not connected!");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  //===========================================
  if (strcmp(topic,SENSORNAME "/Control")==0) {
    //Serial.println(SENSORNAME + "/Control");
    StaticJsonDocument<200> jsonBuffer;
    char s[length];
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      s[i]=payload[i];
    }
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, s);
      if (error){
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
      } 
      else {
        if (doc.containsKey("action")) {
          const char* action = doc["action"];        
          Serial.printf("action found %s \n", action);

          //====================================================
          // setOperationalState
          //====================================================
          if(strcmp(action, "setOperationalState") == 0) {
            Serial.println("Set the Operational State"); 
            if (doc.containsKey("state")) {  
              const char* operationalState  = doc["state"];       
              Serial.printf("Value operational State = %s\n", operationalState);
              if(strcmp(operationalState, "on") == 0) {
                rgbwwLedControl.on();
                pubSubClient.publish(SENSORNAME "/state", "{\"state\": \"ON\"}");
              } else {
                rgbwwLedControl.off();
                pubSubClient.publish(SENSORNAME "/state", "{\"state\": \"OFF\"}");
              }
            }
          }

          //====================================================
          // setBrightness
          //====================================================
          if(strcmp(action, "setBrightness") == 0) {
            //Serial.println("Set the brightness"); 
            if (doc.containsKey("brightness")) {  
              int brightness  = doc["brightness"];       
              //Serial.printf("Value Brightness = %d\n", brightness);
              rgbwwLedControl.setBrightnessRGB(brightness);
            }
          }

          //====================================================
          // autoBrightness
          //====================================================
          if(strcmp(action, "autoBrightness") == 0) {
            Serial.println("auto the brightness"); 
            double step  = 100;
            uint16 Brightness = 0;
            if (doc.containsKey("Brightness")) {  
              Brightness = doc["Brightness"];       
            }
            if (doc.containsKey("step")) {  
              step = doc["step"];       
            }            
            if (doc.containsKey("step")) {  
              step = doc["step"];       
            }            
            if (doc.containsKey("increase")) {  
              bool increase  = doc["increase"];       
              rgbwwLedControl.autoBrightnessRGB(increase, step, Brightness);
            }
          }

          //====================================================
          // setRGB
          //====================================================          
          if(strcmp(action, "setRGB") == 0) {
            Serial.println("Set the RGB"); 

            int red = ((int)doc["color"]["r"]) * 4;
            int green = ((int)doc["color"]["g"]) * 4;
            int blue = ((int)doc["color"]["b"]) * 4;

            //int red  = doc["r"];
            Serial.printf("Value Red = %d\n", red);
            //int green  = doc["g"];
            Serial.printf("Value Green = %d\n", green);
            //int blue  = doc["b"];       
            Serial.printf("Value Blue = %d\n", blue);
            rgbwwLedControl.setRGB(red, green, blue);
          }
        }
      }
  }
}


void reconnect() {
  // Loop until we're reconnected

  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (pubSubClient.connect(SENSORNAME, mqtt_username, mqtt_password)) {
      Serial.println("connected");

      Serial.print(pubSubClient.publish("MQTTIdentify", SENSORNAME));

      mqttIdentify.init(&pubSubClient,&WiFi);
      mqttIdentify.report();
      
      std::string controlChannel;
      controlChannel = SENSORNAME + std::string("/Control");

      pubSubClient.subscribe(controlChannel.c_str());
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/******************************************************************************
 * 
 * Callback functions RGBWWLedControl
 * 
 *****************************************************************************/
int CallbackRGBWW(String parameter, int value) {
    Serial.println("\CallbackRGBWW: a=" + parameter + "/b=" + String(value));

    if (parameter == "state") {
      Serial.println("State change");
      if( value == 1){
        //pubSubClient.publish(statusChannel.c_str(), "{\"state\": \"ON\"}");
      } 
      else {
        //pubSubClient.publish(statusChannel.c_str(), "{\"state\": \"OFF\"}");
      }
    }
    return 32000;
}

void setup() {
  WiFi.persistent(false);
  Serial.begin(115200);
  Serial.println("wakeuplight Booting");
  WiFi.mode(WIFI_STA);

  wifiMulti.addAP("H369A6B77CF", "Spanning!");
  
  connectWifi();

    Serial.print("WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());

  // pinMode(D1, INPUT);
  // attachInterrupt(digitalPinToInterrupt(D1), detectsButton1, RISING);
  // pinMode(D2, INPUT);
  // attachInterrupt(digitalPinToInterrupt(D2), detectsButton2, RISING);

  rgbwwLedControl.init();
  rgbwwLedControl.setCallback(CallbackRGBWW);
  rgbwwLedControl.off();

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("Ready via OTA");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pubSubClient.setServer(mqtt_serverName, 1883);
  pubSubClient.setCallback(callback);

  // reconnect();
  // ++value;
  // snprintf (msg, 75, "online");
  // Serial.print("Publish message: ");
  // Serial.println(msg);
  // pubSubClient.publish("home/bedroom/switch1/available", msg);

  rgbwwLedControl.setBrightnessRGB(255);
  rgbwwLedControl.setRGB(1023,512,255);
  rgbwwLedControl.on();
}



void loop() {

  connectWifi();
  ArduinoOTA.handle();

  if (!pubSubClient.connected()) {
    reconnect();
  }
  pubSubClient.loop();
  rgbwwLedControl.loop();
  mqttIdentify.loop();  

}
