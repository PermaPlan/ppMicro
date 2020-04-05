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
//const char* ssid = "Nemo";
//const char* password = "wasfuereinschoenername";
const char* ssid = "PienzNet5";
const char* password = "!PienzNet5-2OG";
//const char* ssid = "obenW";
//const char* password = "BBWFBUJOSYYHENVL";

////// Mac-adress //////
byte mac[6];

////// Web-CLient //////
//const String host = "http://192.168.178.115";
const String host = "http://192.168.178.125";
//const String host = "http://192.168.178.128";
const String port = ":5000";
const String url = "/add/sensor-value";

////// NTP-Client //////
const long utcOffset = 0;          //  time zone: UTC +1 in seconds // for unix time: 0 s
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
  delay(100);
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
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);
}

void postData(JsonArray t, JsonArray h, JsonArray hi, JsonArray moi, JsonArray moi_or, long ts, DynamicJsonDocument doc) {


  Serial.print("Posting data to: ");
  Serial.print(host);
  Serial.print(port);
  Serial.println(url);
  serializeJsonPretty(doc, Serial);

  char JSONmessageBuffer[2048];
  serializeJson(doc, JSONmessageBuffer);

  HTTPClient http;

  http.begin(host + port + url);      //Specify request destination
  http.addHeader("Content-Type", "application/json");  //Specify content-type header

  int httpCode = http.POST(JSONmessageBuffer);   //Send the request
  String payload = http.getString();             //Get the response payload

  Serial.print("\n");
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection

}

void readSensors() {

  dht.begin();
  timeClient.begin();

  DynamicJsonDocument doc(2048);
  JsonArray soil_moisture_analog = doc.createNestedArray("soil_moisture_analog");
  JsonArray soil_moisture = doc.createNestedArray("soil_moisture");
  JsonArray air_temperature = doc.createNestedArray("air_temperature");
  JsonArray air_humidity = doc.createNestedArray("air_humidity");
  JsonArray heat_index_C = doc.createNestedArray("heat_index_C");

  for (int i = 0; i < datapoints_max; i++)
  {

    //Serial.print("Measuring data point ");
    //Serial.println(i+1);

    int moi_or = analogRead(A0);                        // read capacitive soil moisture
    int moi = map(moi_or, 400, 850, 100, 0);            // map from 0% to 100%
    float t = dht.readTemperature();                    // read air temperature
    float h = dht.readHumidity();                       // read air humidity
    float hi = dht.computeHeatIndex(t, h, false);       // Compute heat index in Celsius (isFahreheit = false)

    soil_moisture_analog.add(moi_or);
    soil_moisture.add(moi);
    air_temperature.add(t);
    air_humidity.add(h);
    heat_index_C.add(hi);

    delay(2000);                    // wait a bit after measuring

    if (isnan(h) || isnan(t)) {
      Serial.println("ERROR reading DHT11");
      return;
    }
    else if (isnan(moi_or)) {
      Serial.println("ERROR reading MS V1.2");
      return;
    }
  }


  timeClient.update();
  long ts = timeClient.getEpochTime();    // unix time stamp
  doc["timestamp"] = ts;

  Serial.println(ts);
  //JsonObject obj = doc.to<JsonObject>();
  //serializeJson(doc, Serial);
  //Serial.println(obj);
  //Serial.println(doc);
  postData(air_temperature, air_humidity, heat_index_C, soil_moisture, soil_moisture_analog, ts, doc);

}
