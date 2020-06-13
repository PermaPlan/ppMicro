/////////////////////////////////////////////
/////------- Perma Plan Sensoren -------/////
// by Faro Schäfer                         //
// Version: V1                             //  
// Code for sensor with Wifi Communication //
/////////////////////////////////////////////


////// Include Libraries //////

  #include "DHT.h"                                                              // DHT sensor
  #include <ESP8266WiFi.h>                                                      // ESP8266 
  // #include <ESP8266Ping.h> // to ping a host with Ping.ping("www.google.de") // Just for test purposes
  #include <ESP8266HTTPClient.h>                                                // 
  #include <ArduinoJson.h>                                                      // Arduino JSON V.6.
  #include <WiFiUdp.h>                                                          // 
  #include <NTPClient.h>                                                        // For Timestamp ( I think this is a special github version )

////// INPUT //////

  /// Sensor Inputs /// 
  
    #define durationSleep 3600           // sleep time in [s]
    const int datapoints_max = 3;        // amount of measured datapoints per wake up
    int min_moi = 400;                   // minimal analoug value from moisture sensor tuning
    int max_moi = 850;                   // maximal analoug value from moisture sensor tuning

  /// Network ID ///
    
    //const char* ssid = "Nemo";
    //const char* password = "wasfuereinschoenername";
    
    const char* ssid = "PienzNet5";
    const char* password = "!PienzNet5-2OG";
    
    //const char* ssid = "obenW";
    //const char* password = "BBWFBUJOSYYHENVL";
  
  /// Web-Client ///

    /// Local:
    //const String host = "http://192.168.178.115"; // local
    //const String host = "http://192.168.178.125"; // local Simon Mac
    //const String host = "http://192.168.178.128"; // local 
    //const String port = ":5000"; 
    //const String url = "/add/sensor-value";

    /// Azure:
    const String host = "http://permaplandata.azurewebsites.net"; // azure student web-server
    const String port = "";                                       // empty string for port
    const String url = "/add/sensor-value";
    
  /// NTP-Client ////
  
    const long utcOffset = 0;          //  time zone: UTC +1 in seconds // for unix time: 0 s

////// Initialization //////

  /// DHT11 Sensor ///

    #define DHTPIN D4                     // what pin DHT 11 is connected to
    #define DHTTYPE DHT11                 // DHT 11
    DHT dht(DHTPIN, DHTTYPE);             // initialize DHT sensor

  /// NTP Client ///

    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset);

  /// LEDs ///
  
    //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

////// Preallocation //////

const size_t CAPACITY = JSON_ARRAY_SIZE(datapoints_max);


//////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////Functions/////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

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
      if ( connectionTry >= 10 ) {
        Serial.println("Connection not possible");
        Serial.print("Restarting ESP ... ");
        ESP.restart();
      }
    }
  }

  Serial.print("\nWiFi connected to '");
  Serial.print(ssid);
  Serial.println("'");
  Serial.print("Host IP-address: ");
  Serial.println(WiFi.localIP());
  Serial.println("D1mini MAC-adress: " + WiFi.macAddress());

}

void postData(JsonArray t, JsonArray h, JsonArray hi, JsonArray moi, JsonArray moi_or, long ts, DynamicJsonDocument doc) {


  Serial.print("Posting data to: ");
  Serial.println(host + port + url);

  serializeJsonPretty(doc, Serial);

  char JSONmessageBuffer[2048];
  serializeJson(doc, JSONmessageBuffer);

  HTTPClient http;

  http.begin(host + port + url);                        //Specify request destination
  http.addHeader("Content-Type", "application/json");   //Specify content-type header

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

    int moi_or = analogRead(A0);                        // read capacitive soil moisture
    int moi = map(moi_or, min_moi, max_moi, 100, 0);    // map from 0% to 100% [rF]
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
  
  Serial.print('Timestamp: ');
  Serial.println(ts);
  
  postData(air_temperature, air_humidity, heat_index_C, soil_moisture, soil_moisture_analog, ts, doc);

}
