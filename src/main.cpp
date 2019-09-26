#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>


// Define pin numbers
#define Temp D4
#define durationSleep  3600
#define DHTTYPE    DHT22

#define MQTT_USERNAME "MTQQ-USER"
#define MQTT_PASSWORD "MQTT-PASS"

//Wireless Info
const char* ssid = "SSID";
const char* password = "KEY"; 

//MQTT Info
char* TempTopic = (char *)"TEMP-TOPIC";
char* HumTopic = (char *)"HUM-TOPIC";
char* BatTopic = (char *)"BATTERY-TOPIC";
const char* WILL_TOPIC = (char *)"devices/will";
const char* WILL_MESSAGE = "offline";
const char* server = (char *) "SERVER";
const String HOSTNAME = "NAME";
char message_buff[100]; 

WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);
DHT_Unified dht(Temp, DHTTYPE);


int temp=0;

bool checkMqttConnection(){
  if (!client.connected()) {
    if (client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD, WILL_TOPIC, 0, 0, WILL_MESSAGE, 0)) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED");
    }
  }
  return client.connected();
}

void connectWifi(){
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.println(".");
  // Aguarda até estar ligado ao Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Ligado a ");
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(Temp,INPUT);  
  Serial.begin(115200);
  dht.begin();
  connectWifi();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()){
      client.loop();
      sensors_event_t event;
      dht.temperature().getEvent(&event);
      if ( isnan(event.temperature) ) {
        Serial.println(F("Error reading temperature!"));
      }
      else {      
        String temperature = String(event.temperature);
        char temp_payload[temperature.length()+1];
        temperature.toCharArray(temp_payload, temperature.length()+1); 
        client.publish( TempTopic, temp_payload, true );
        Serial.println(temperature);
      }
      dht.humidity().getEvent(&event);
      if(isnan(event.temperature)){
        Serial.println(F("Error reading humidity!"));
      }else{
        String humidity = String(event.relative_humidity);
        char hum_payload[humidity.length()+1];
        humidity.toCharArray(hum_payload, humidity.length()+1); 
        client.publish( HumTopic, hum_payload, true );        
        Serial.println(humidity);        
      }
      String bat = String(analogRead(A0)/209.66);
      char bat_payload[bat.length()+1];
      bat.toCharArray(bat_payload, bat.length()+1); 
      client.publish( BatTopic, bat_payload, true ); 

      Serial.println(bat);
      delay(1000);
      ESP.deepSleep(36e8);
    }
  }else{
    connectWifi();
  }
}