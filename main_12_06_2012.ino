/*******************************************************************************************************************/
/*  Tarleton Aeronautical Team
/*  University Student Launch Initiative 2012-2013
/*  Atmospheric Data Gathering Circuit
/*
/*  Created: November 16, 2012
/********************************************************************************************************************/

// Preprocessing Directives
#include <FreqCounter.h>
#include <Wire.h> // I2C Library
#include <Arduino.h> // This is used for Arduino-1.0 for many functions. 
#include "TSL2561.h"
#include <SD.h>   

#define SERIAL_BAUD_RATE 19200
#define GPS_BAUD_RATE 57600
#define XBEE_BAUD_RATE 19200

// Global Variables

// HIH4030
int HIH4030_RH;
int HIH4030_PIN = 0;

// TSL2561
int TSL2561_LUX;

// BMP180
short BMP180_TEMP;
long BMP180_PRESSURE;
long BMP180_INIPRESSURE;
long BMP180_ALTITUDE;

// GPS
char GPSBUFFER[100];

// XBee
String TELEMETRY_STRING = "";

// SD
File DATAFILE;

// Time
long TIMER;

// Setup
void setup(){
  // XBee Serial connections.
  Serial.begin(SERIAL_BAUD_RATE); 
  
  // GPS Serial Connections.
  Serial1.begin(GPS_BAUD_RATE);
 
  Wire.begin(); 

  // Initiates the SD card using cs pin 53
  SD.begin(53);    

  // Create the SD file and write a title                         //   
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);             //
  DATAFILE.println();
  DATAFILE.println(); 
  DATAFILE.println("System Initializing..."); 
  DATAFILE.print("Serial Baud Rate: ");   DATAFILE.println(SERIAL_BAUD_RATE); 
  DATAFILE.print("GPS Baud Rate: ");   DATAFILE.println(GPS_BAUD_RATE); 
  DATAFILE.print("XBee Baud Rate: ");   DATAFILE.println(XBEE_BAUD_RATE);
  DATAFILE.close();  
  
  //Calibration variables needed for the BMP180 to give accurate data. 
  TSL2561_calibration();
  
  //Calibration variables needed for the BMP180 to give accurate data.
  BMP180_calibration();      
  
  BMP180_TEMP = BMP180_getTemperatureIni();
  BMP180_INIPRESSURE = BMP180_getPressureIni();

  DATAFILE = SD.open("datalog.txt", FILE_WRITE); 
  DATAFILE.println();
  DATAFILE.println("\n\n************************************ END OF SETUP ************************************\n\n"); 
  DATAFILE.println();
  DATAFILE.close();  

}

// Loop
void loop(){
  // Read lux value from the TSL2561.
  TSL2561_LUX = TSL2561_getLux();
  
  BMP180_TEMP     = BMP180_getTemperature(BMP180_readUT());  // temperature must be populated prior to pressure
  BMP180_PRESSURE = BMP180_getPressure(BMP180_readUP());        // pressure must be calculated before altitude
  BMP180_ALTITUDE = BMP180_getAltitude(BMP180_PRESSURE, BMP180_INIPRESSURE);
  
  // Function parameters are pin # and temperature.
  HIH4030_RH = HIH4030_readRH(HIH4030_PIN, float(BMP180_TEMP)/10);
  
  // Gathers GPS data and stores them to GPSBUFFER
  GPS();
  
  // Save the current time since program started.
  TIMER = millis();
  
  // Construct the string that is transmited via the XBee.
  TELEMETRY_STRING = " ";
  TELEMETRY_STRING += HIH4030_RH;
  TELEMETRY_STRING += " ";
  TELEMETRY_STRING += TSL2561_LUX;
  TELEMETRY_STRING += " ";
  TELEMETRY_STRING += BMP180_TEMP;
  TELEMETRY_STRING += " ";
  TELEMETRY_STRING += BMP180_PRESSURE;
  TELEMETRY_STRING += " ";
  TELEMETRY_STRING += BMP180_ALTITUDE;
  TELEMETRY_STRING += " ";
  TELEMETRY_STRING += String(GPSBUFFER);
  TELEMETRY_STRING += " ";  
  TELEMETRY_STRING += TIMER;
  TELEMETRY_STRING += " ";  
  
  // Transmit
  Serial.println(TELEMETRY_STRING);  
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);      
  DATAFILE.println("Telemetry String: \n\t"); 
  DATAFILE.println(TELEMETRY_STRING); 
  DATAFILE.println();
  DATAFILE.println("************************************ END OF LOOP ************************************"); 
  DATAFILE.println();
  DATAFILE.close();  
}
