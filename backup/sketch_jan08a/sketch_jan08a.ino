#include "RunningMedian.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

//RunningMedian array = RunningMedian(15);
int array = {2, 4, 5, 3, 5, 6, 20};

void setup() {
  Serial.begin(9600);
  float med = array.getMedian();
  Serial.Println(med);
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
