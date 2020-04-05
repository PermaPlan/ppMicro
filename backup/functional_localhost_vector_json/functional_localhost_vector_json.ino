#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define DHTPIN D4                     // what pin we're connected to
#define DHTTYPE DHT11                 // DHT 11
#define durationSleep  5              // sleep time in [s]

DHT dht(DHTPIN, DHTTYPE);             // initialize DHT sensor

////// Network SSID //////
const char* ssid = "Nemo";
const char* password = "wasfuereinschoenername";
//const char* ssid = "PienzNet5";
//const char* password = "!PienzNet5-2OG";
//const char* ssid = "obenW";
//const char* password = "BBWFBUJOSYYHENVL";

////// Mac-adress //////
byte mac[6];

////// Web-CLient //////
//const String host = "http://192.168.178.115";

const String host = "http://192.168.178.128";
const String port = ":5000";
const String url = "/add/sensor-value";

////// NTP-Client //////
const long utcOffset = 3600;          //  time zone: UTC +1 in seconds
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset);


////// OTHER //////
const int datapoints_max = 10;        // amount of datapoints per wake up

const size_t CAPACITY = JSON_ARRAY_SIZE(datapoints_max);


//--------------------------------------------------------------------------------------------//

void setup() {
  
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  //digitalWrite(LED_BUILTIN, HIGH);
  //delay(100);
  Serial.begin(9600);               // Set bauD rate 
  while (!Serial) {
    ;                               // wait for serial communication to be build up
  }

  Serial.print("\n\nWaking up ...");
  delay(2000);

  wifiConnect();
  readSensors();

  Serial.print("Going to sleep ... "); 
  ESP.deepSleep(durationSleep * 1000000);  
  delay(100);
}

void loop() { 
}


void wifiConnect() {
    
  if (WiFi.status() != WL_CONNECTED)  {
    // Connect WiFi
    Serial.print("\nConnecting to Wifi");
    WiFi.hostname("D1 mini");
    WiFi.begin(ssid, password);
    
    int connectionTry = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      connectionTry++;
      if ( connectionTry >= 50 ) {
        Serial.println("Connection not possible");
        Serial.print("Restarting ESP ... "); 
        ESP.restart();
      }
    }
  }
   
  Serial.print("\nWiFi connected to '");
  Serial.print(ssid);
  Serial.println("'");
    // Print the IP address
  Serial.print("Host IP-address: ");
  Serial.println(WiFi.localIP());

  //Print d1 minis mac adress
  WiFi.macAddress(mac);
  Serial.print("D1mini MAC-adress: ");
  Serial.print(mac[0],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.println(mac[5],HEX);
}

void postData(float* t, float* h, float* hi, int* moi, int* moi_or, String ts) {


  
  
 /* 
  DynamicJsonDocument JSONencoder(300);
// try to send array: nested array// send array arduinoJson
  JSONencoder["air_humidity"] = h;
  JSONencoder["air_temperature"] = t;
  JSONencoder["heat_index_C"] = hi;
  JSONencoder["soil_moisture"] = moi;
  JSONencoder["soil_moisture_analog"] = moi_or;
  JSONencoder["timestamp"] = ts;

  char JSONmessageBuffer[CAPACITY];
  serializeJson(JSONencoder, JSONmessageBuffer);
  Serial.println(JSONmessageBuffer);

  HTTPClient http;    //Declare object of class HTTPClient

  http.begin(host + port + url);      //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header

  int httpCode = http.POST(JSONmessageBuffer);   //Send the request
  String payload = http.getString();             //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
  */
}

void readSensors() {
    
    dht.begin(); 
    timeClient.begin();  

    int moi_or[datapoints_max];
    int moi[datapoints_max];
    float h[datapoints_max];
    float t[datapoints_max];
    float hi[datapoints_max];
    String ts;

    for (int i = 0; i < datapoints_max; i++)
    {
      //Serial.print("Measuring data point ");
      //Serial.println(i+1);

      DynamicJsonDocument doc(2048);
      JsonArray temperature = doc.createNestedArray("air_temperature");
      JsonArray humidity = doc.createNestedArray("air_humidity");
      JsonArray moisture = doc.createNestedArray("soil_moisture");
      JsonArray moisture_or = doc.createNestedArray("soil_moisture_analog");
      JsonArray heat_index = doc.createNestedArray("heat_index_C");
      JsonArray timestamp = doc.createNestedArray("timestamp");
      
      //digitalWrite(BUILTIN_LED, LOW);
      //delay(100);
      moi_or [i] = analogRead(A0);                        // read capacitive soil moisture
      moi [i] = map(moi_or [i],400,850,100,0);                // map from 0% to 100%
      h [i] = dht.readHumidity();                       // read air humidity
      t [i] = dht.readTemperature();                    // read air temperature
      hi[i] = dht.computeHeatIndex(t[i], h[i], false);       // Compute heat index in Celsius (isFahreheit = false)
      //digitalWrite(BUILTIN_LED, HIGH);
      //delay(100);
      timeClient.update();
      ts = timeClient.getFormattedDate();

      temperature.add(t);
      humidity.add(h);
      moisture.add(moi);
      moisture_or.add(moi_or);
      heat_index.add(hi);
      timestamp.add(ts);

      serializeJson(doc, Serial);
      
           
      delay(2000);                    // wait a bit before measuring 
    
      if (isnan(h[i]) || isnan(t[i])) {
        Serial.println("ERROR reading DHT11");
        return;
      }
      else if (isnan(moi_or[i])) {
        Serial.println("ERROR reading MS V1.2");
        return;
      }      
    }


    // get epoch time unix: macht mehr sinn für front end
    
    Serial.println(ts);
    
      postData(t, h, hi, moi, moi_or, ts);
        
      //Serial.print("\n");
      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
        Serial.print("Humidity: ");
        delay(100);
        }
      Serial.print(h[i]);
      Serial.print(" ");
      }
      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
          Serial.print("\nTemperature: ");
          delay(100);
        }
      Serial.print(t[i]);
      Serial.print(" ");
      }
      
      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
        Serial.print("\nMoi: ");
        }
      Serial.print(moi[i]);
      Serial.print(" ");
      }
      
      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
        Serial.print("\nMoi_or: ");
        delay(100);
        }
      Serial.print(moi_or[i]);
      Serial.print(" ");
      }

      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
        Serial.print("\nHeat_index: ");
        delay(100);
        }
      Serial.print(hi[i]);
      Serial.print(" ");
      }
      
      Serial.print("\n");
}
