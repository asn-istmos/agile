#include <Arduino.h>
#include <Wire.h>
#include <Sodaq_wdt.h>
#include <Sodaq_nbIOT.h>

#if defined(ARDUINO_AVR_LEONARDO)
#define DEBUG_STREAM Serial 
#define MODEM_STREAM Serial1

#elif defined(ARDUINO_AVR_UNO)
 SoftwareSerial softSerial(10, 11); // RX, TX
 // You can connect an uartsbee or other board (e.g. 2nd Uno) to connect the softserial.
#define DEBUG_STREAM softSerial 
#define MODEM_STREAM Serial

#elif defined(ARDUINO_SODAQ_EXPLORER)
#define DEBUG_STREAM SerialUSB
#define MODEM_STREAM Serial

#elif defined(ARDUINO_SAM_ZERO)
#define DEBUG_STREAM SerialUSB
#define MODEM_STREAM Serial1

#else
#error "Please select a Sodaq ExpLoRer, Arduino Leonardo or add your board."
#endif

Sodaq_nbIOT nbiot;

int connected_nbiot = 0;
int sensorPin = A0;
int sensorValue = 0;
float Vout = 0;
int16_t UDP_socket = -1;
int loops = 0;
int max_loops = 24;
float temperature_av = 0;
float humidity_av = 0;
int Station_Number = 1001;
 
void setup()
  {
	  while ((!DEBUG_STREAM) && (millis() < 10000)) {
      // Wait for serial monitor for 10 seconds
    }
    
    DEBUG_STREAM.begin(9600);
    MODEM_STREAM.begin(nbiot.getDefaultBaudrate());

	  DEBUG_STREAM.println("\r\nSODAQ HTS221 Arduino Example\r\n");

	  nbiot.init(MODEM_STREAM, 7);
	  nbiot.setDiag(DEBUG_STREAM);

    Serial.begin(9600);

	  delay(2000);

    if (nbiot.connect("xxxxx", "xxx.xxx.xxx.xxx", "xxxxx")) {
      DEBUG_STREAM.println("Connected succesfully!");
      connected_nbiot = 1;
    }
    else {
      DEBUG_STREAM.println("Failed to connect!");
      //return;
    }
	  // CONNECT TO ANY SENSOR
	  /*if (sensor.begin() == false) {
		  DEBUG_STREAM.println("Error while retrieving WHO_AM_I byte...");
	  }
    else {
      DEBUG_STREAM.println("Sensor works fine...");
    }*/
  }


void loop()
  {
    loops++; 
    if ((connected_nbiot == 0) && (loops == max_loops )) {
      if (nbiot.connect("xxxxx", "xxx.xxx.xxx.xxx", "xxxxxx")) {
        DEBUG_STREAM.println("Connected succesfully!");
        connected_nbiot = 1;
      }
      else {
        DEBUG_STREAM.println("Failed to connect!");
        connected_nbiot = 0;
      }
    }

	  // Create the message
	  byte message[8];
	  uint16_t cursor = 0;
	  int16_t temperature;
	  int16_t humidity;
    if (loops <= max_loops) {
      temperature = sensor.readTemperature() * 100;
      temperature_av = temperature_av + temperature;
      DEBUG_STREAM.print('#');
      DEBUG_STREAM.println(loops);
      DEBUG_STREAM.print('T');
      DEBUG_STREAM.print(temperature);
      DEBUG_STREAM.println();
      humidity = sensor.readHumidity() * 100;
      humidity_av = humidity_av + humidity;
      DEBUG_STREAM.print('H');
      DEBUG_STREAM.print(humidity);
      DEBUG_STREAM.println();
    }
    else {
      message[cursor++] = Station_Number >> 8;
      message[cursor++] = Station_Number;
      temperature = ((temperature_av/100)/max_loops) * 100;
      message[cursor++] = temperature >> 8;
      message[cursor++] = temperature;
      humidity = ((humidity_av/100)/max_loops) * 100;
      message[cursor++] = humidity >> 8;
      message[cursor++] = humidity;
      // Print the message we want to send
      for (int i = 0; i < cursor; i++) {
        if (message[i] < 10) {
          DEBUG_STREAM.print("0");
        }
        DEBUG_STREAM.print(message[i], HEX);
        DEBUG_STREAM.print(":");
      }
      DEBUG_STREAM.println();
      
      // Indication about NB-IoT connection
      DEBUG_STREAM.print('C');
      DEBUG_STREAM.print(connected_nbiot);
      DEBUG_STREAM.println();
      // Send the message
      DEBUG_STREAM.println(nbiot.sendUDP(message, cursor, 1883));
      loops = 0;
      temperature_av = 0;
      humidity_av = 0;
    }
	 // Wait some time between messages
	 delay(5000); // 1000 = 1 sec
 }