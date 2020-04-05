#include "DHT.h"
#include <ESP8266WiFi.h>

#define DHTPIN D4                     // what pin we're connected to
#define DHTTYPE DHT11                 // DHT 11
DHT dht(DHTPIN, DHTTYPE);             // initialize DHT sensor

#define durationSleep  5              // sleep time in [s]

// Network SSID
//const char* ssid = "Nemo";
//const char* password = "wasfuereinschoenername";
//const char* ssid = "PienzNet5";
//const char* password = "!PienzNet5-2OG";
//const char* ssid = "obenW";
//const char* password = "BBWFBUJOSYYHENVL";
WiFiServer server(80); //Port auf welchem der Server laufen soll.
byte mac[6];

String prepareHeader()
{
  String htmlHeader =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "Refresh: 2\r\n" +  // refresh the page automatically every 5 sec
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

  Serial.print("\n\nWaking up ...");
  delay(2000);

  wifiConnect();
  serverConnect();
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

void serverConnect() {

  server.begin(); // Starten des Servers.
  Serial.println("Server established"); //Ausgabe auf der Seriellen Schnittstelle das der Server gestartet wurde.
 
  // Ausgabe der IP Adresse 
  Serial.print("URL : http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.println("--------------------------------------");

}

void readSensors() {

    dht.begin();   
    for (int i = 0; i < 50; i++)
    {
      
      Serial.print("Measuring data point ");
      Serial.println(i+1);
      
      delay(1000);                    // wait a bit before measuring 
      
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

      WiFiClient client = server.available();
      if (client) {
        Serial.println("new client");        // Kontrollelement im seriellen Monitor
        while (client.connected()) {
          if (client.available())  {         
            response(client, t, h, hi, moi);
            break;    
          }  
        }
      }
    }
}

void response(WiFiClient client, float t, float h, int hi, int moi){              

  client.println(prepareHeader());
  client.println("<html>");                                                     
  client.println("Air humdity: ");
  client.println(h);
  client.println("%");
  client.println("<br>");
  client.println("Air temperature: ");
  client.println(t);
  client.println(" &deg;C");
  client.println("<br>");
  client.println("Heat index: ");
  client.println(hi);
  client.println(" &deg;C");
  client.println("<br>");
  client.println("Moi: ");
  client.println(moi);
  client.println(" % ");
  client.println("<br>");
  client.println("</html>"); 

}
