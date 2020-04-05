#include "DHT.h"
#include <ESP8266WiFi.h>

#define DHTPIN D4                     // what pin we're connected to
#define DHTTYPE DHT11                 // DHT 11
DHT dht(DHTPIN, DHTTYPE);             // initialize DHT sensor

#define durationSleep  5              // sleep time in [s]

// Network SSID
const char* ssid = "obenW";
const char* password = "BBWFBUJOSYYHENVL";
byte mac[6];

void setup() {

  Serial.begin(9600);               // Set bauD rate 
  delay(500);                       // wait for serial communication to be build up
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  Serial.println("\n\nWaking up ...");
  delay(1000);

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
    Serial.print("Connecting to Wifi");
    WiFi.hostname("D1 mini");
    WiFi.begin(ssid, password);
    
    int connectionTry = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      connectionTry++;
      if ( connectionTry >= 50 ) {
        Serial.println("Connection not possible");
        Serial.print("Going to sleep ... "); 
        ESP.deepSleep(durationSleep * 1000000);
    }
  }
   
  Serial.println("\nWiFi connected");
    // Print the IP address
  Serial.print("Host IP address: ");
  Serial.println(WiFi.localIP());

  //Print d1 minis mac adress
  WiFi.macAddress(mac);
  Serial.print("D1 mini MAC: ");
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
  Serial.println("");
  }
}


void readSensors() {

    dht.begin();   

    for (int i = 0; i < 10; i++)
    {

      Serial.print("Measuring data point ");
      Serial.println(i+1);
      
      delay(2000);                    // wait a bit before measuring 
      
      digitalWrite(BUILTIN_LED, LOW);
      delay(100);
      float h = dht.readHumidity();
      float t = dht.readTemperature();  
      float hi = dht.computeHeatIndex(t, h, false);       // Compute heat index in Celsius (isFahreheit = false)
      digitalWrite(BUILTIN_LED, HIGH);
      delay(100);
    
      if (isnan(h) || isnan(t)) {
        Serial.println("ERROR reading the DHT!");
        return;
      }
   
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t ");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" °C\t ");
      Serial.print("Heat index: ");
      Serial.print(hi);
      Serial.println(" °C ");   
    }  
}
