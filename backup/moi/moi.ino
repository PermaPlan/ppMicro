  
 void setup() {

  Serial.begin(9600);               // Set bauD rate 
  delay(500);                       // wait for serial communication to be build up
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

}

void loop() {

  delay(2000);
  int moi_or = analogRead(A0); 
  int moi = map(moi_or,400,850,100,0);

  Serial.print("\nMOI_or: ");
  Serial.println(moi_or);
  Serial.print("MOI: ");
  Serial.println(moi);
  
}
