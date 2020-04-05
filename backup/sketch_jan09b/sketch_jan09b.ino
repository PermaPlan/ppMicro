#include <ArduinoJson.h>

void setup() {
  
  Serial.begin(9600);               // Set bauD rate 
  while (!Serial) {
    ;                               // wait for serial communication to be build up
  }

}

void loop() {
    int values[10];
    
      for (int i = 0; i < 10; i++) {
      values[i] = i+1;
    }

    Post(values);

}

void Post(int values[] ) {

    StaticJsonDocument<256> doc;
    copyArray( values, doc.to<JsonArray>());
    serializeJson(doc, Serial);
  
}
