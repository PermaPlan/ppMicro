/* WeMos relay example - HobbyComponents.com 
   WeMos compatible DHT library can be downloaded from our support forum here:
   http://forum.hobbycomponents.com/viewtopic.php?f=111&t=2130 */

#include "DHT.h"

/* Define the DIO pin that will be used to communicate with the sensor */
#define DHT11_DIO D4


void setup()
{
  /* Setup the serial port for displaying the output of the sensor */
  Serial.begin(9600);
}

/* Main program loop */
void loop()
{
  /* Perform a read of the sensor and check if data was read OK */
  if (DHT.read11(DHT11_DIO) == DHTLIB_OK)
  {
    /* If so then output the current temperature and humidity to 
    the serial port */
    Serial.print("Temperature: ");
    Serial.print((float)DHT.temperature, 2);
    Serial.print("oC\t");
    Serial.print("Humidity: ");
    Serial.print((float)DHT.humidity, 2);
    Serial.println("%");
  }else
  {
    /* If there was a problem reading from then sensor then output 
    an error */
    Serial.println("ERROR");
  }

  /* Wait at least 2 seconds before reading a new temperature */
  delay(2000);
}
