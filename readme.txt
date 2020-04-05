This repository contains the source codes that will be loaded onto the respective micro-controller using Arduino-IDE

ESP_8266_V1:
	Installed Sensors: DHT11, Moisture Sensor
	Infrastructure: WLAN
	Data: soil_moisture_analog (from 0 to 1024), soil_moisture (in %), air_temperature (in C), air_humidity, heat_index_C (felt temperature in C) 
	Content: Reading sensors and transferring the content to a server as a JSON with UNIX timestamps
		 
