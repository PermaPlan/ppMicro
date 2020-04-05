#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define DHTPIN D4                     // pin the shield is connected to
#define DHTTYPE DHT11                 // DHT 11
DHT dht(DHTPIN, DHTTYPE);             // initialize DHT sensor

// Network SSID
//const char* ssid = "Nemo";
//const char* password = "wasfuereinschoenername";
const char* ssid = "PienzNet5";
const char* password = "!PienzNet5-2OG";
//const char* ssid = "obenW";
//const char* password = "BBWFBUJOSYYHENVL";
byte mac[6];



String prepareHeader()
{
  String htmlHeader =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "Refresh: 2\r\n" +  // refresh the page automatically every 2 sec
            "\r\n" +
            "<!DOCTYPE HTML>";
  return htmlHeader;
}

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(9600);               // Set bauD rate 
  while (!Serial) {
    ;                               // wait for serial communication to be build up
  }
  wifiConnect();
}

void loop() { 

   readSensors();
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
   
  Serial.println("\nWiFi connected");
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


void postData(float t, float h, int hi, int moi, int moi_or) {
  
  DynamicJsonDocument JSONencoder(300);

  JSONencoder["air_humidity"] = h;
  JSONencoder["air_temperature"] = t;
  JSONencoder["heat_index_C"] = hi;
  JSONencoder["soil_moisture"] = moi;
  JSONencoder["soil_moisture_analog"] = moi_or;

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

  Serial.println("Reading sensors...");
  delay(60000);                    // wait a bit before measuring 
  
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  int moi_or = analogRead(A0);                        // read capacitive soil moisture
  int moi = map(moi_or,400,850,100,0);                // map from 0% to 100%
  float h = dht.readHumidity();                       // read air humidity
  float t = dht.readTemperature();                    // read air temperature
  float hi = dht.computeHeatIndex(t, h, false);       // Compute heat index in Celsius (isFahreheit = false)
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);

  if (isnan(h) || isnan(t)) {
    Serial.println("ERROR reading DHT11");
    return;
  }
  else if (isnan(moi_or)) {
    Serial.println("ERROR reading MS V1.2");
    return;
  }

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

  postData( t, h, hi, moi, moi_or);

}
