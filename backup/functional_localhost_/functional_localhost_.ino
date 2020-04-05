
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define DHTPIN D4                     // what pin we're connected to
#define DHTTYPE DHT11                 // DHT 11
DHT dht(DHTPIN, DHTTYPE);             // initialize DHT sensor

#define durationSleep  600              // sleep time in [s]

// Network SSID
//const char* ssid = "Nemo";
//const char* password = "wasfuereinschoenername";
const char* ssid = "PienzNet5";
const char* password = "!PienzNet5-2OG";
//const char* ssid = "obenW";
//const char* password = "BBWFBUJOSYYHENVL";
byte mac[6];

const String host = "http://192.168.178.115";
const String port = ":5000";
const String url = "/add/sensor-value";

const int datapoints_max = 10;        // amount of datapoints per wake up

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

void postData(float t, float h, int hi, int moi, int moi_or, int i, int datapoints_max) {
  
  DynamicJsonDocument JSONencoder(300);

  JSONencoder["air_humidity"] = h;
  JSONencoder["air_temperature"] = t;
  JSONencoder["heat_index_C"] = hi;
  JSONencoder["soil_moisture"] = moi;
  JSONencoder["soil_moisture_analog"] = moi_or;
  JSONencoder["data_point"] = i;
  JSONencoder["max_data_point"] = datapoints_max;

  char JSONmessageBuffer[300];
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
}

void readSensors() {

    
    dht.begin();   

    int moi_or[datapoints_max];
    //int moi[datapoints_max];
    float h[datapoints_max];
    float t[datapoints_max];

    
    
    for (int i = 0; i < datapoints_max; i++)
    {
      //Serial.print("Measuring data point ");
      //Serial.println(i+1);

      
      //digitalWrite(BUILTIN_LED, LOW);
      //delay(100);
      moi_or [i] = analogRead(A0);                        // read capacitive soil moisture
      //moi [i] = map(moi_or,400,850,100,0);                // map from 0% to 100%
      h [i] = dht.readHumidity();                       // read air humidity
      t [i] = dht.readTemperature();                    // read air temperature
      //float hi = dht.computeHeatIndex(t, h, false);       // Compute heat index in Celsius (isFahreheit = false)
      //digitalWrite(BUILTIN_LED, HIGH);
      //delay(100);

      delay(10000);                    // wait a bit before measuring 
    
      if (isnan(h[i]) || isnan(t[i])) {
        Serial.println("ERROR reading DHT11");
        return;
      }
      else if (isnan(moi_or[i])) {
        Serial.println("ERROR reading MS V1.2");
        return;
      }

   /*
      Serial.print("\tHumidity: ");
      Serial.print(h);
      Serial.println(" [%] ");
      Serial.print("\tTemperature: ");
      Serial.print(t);
      Serial.println(" [°C] ");
      Serial.print("\tHeat index: ");
      Serial.print(hi);
      Serial.println(" [°C] ");   
      Serial.print("\tMOI_or: ");
      Serial.print(moi_or);
      Serial.println(" [-] ");
      Serial.print("\tMOI: ");
      Serial.print(moi);
      Serial.println(" [%] ");
*/
   //   postData( t, h, hi, moi, moi_or, i, datapoints_max);
      
    }
      Serial.print("\n");
      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
        Serial.print("\nHumidity: ");
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
      /*
      for (int i = 0; i < datapoints_max; i++) {
        Serial.print("\tMoi: ");
      Serial.print(moi[i]);
      }
      */
      for (int i = 0; i < datapoints_max; i++) {
        if (i == 0) {
        Serial.print("\nMoi_or: ");
        delay(100);
        }
      Serial.print(moi_or[i]);
      Serial.print(" ");
      }
      Serial.print("\n");
}
