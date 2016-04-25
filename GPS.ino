/*
    GPS
    
      References:
        http://www.sparkfun.com/tutorials/176
        
*/
int numSats = 0;                             // number of satallites tracking
int fixType = 0;
int time[] = {0, 0, 0};                      // The current time 
double latitude = 0.0;
double longitude = 0.0;
long gpsaltitude = 0;
long adjustedGpsAltitude;
long maxAlt = 0;
int speed = 0;
int txCount = 0;
int ExOnce = 0;
int FalcomCheck = 0;
unsigned long GpsOffTime = 0;  

// All the Functions used for the GPS, reading the NMEA statement, formatting the data
void GPS(){
  Serial1.flush();

  // Get a GGA string from the GPS, and
  // check if it's a valid fix, and extract the data.
  getNMEA("$GPGGA");    
  //delay(100);
  numSats = getSats();
  fixType = getFixType();
  // Make sure we have a valid fix
  if (fixType != 0) {
    getTime(time);
    latitude = getLat();
    longitude = getLong();
    gpsaltitude = getAlt();
  }     

  
  // Convert latitude and longitude into strings.
  char latString[12]; 
  char longString[12];
  doubleToString(latitude, 4, latString);
  doubleToString(longitude, 4, longString);
  //gpsaltitude=(gpsaltitude - Stepsealevel);
  sprintf(GPSBUFFER, "%02d%02d%02d %s %s %ld %d", 
  time[0], time[1], time[2], latString, longString , gpsaltitude ,numSats);
  //Serial.println(GPSBUFFER);

  DATAFILE = SD.open("datalog.txt", FILE_WRITE);             //
  DATAFILE.print("GPS_Time: ");   DATAFILE.print(time[0]); DATAFILE.print(time[1]); DATAFILE.println(time[2]); 
  DATAFILE.print("GPS_Latitude: ");   DATAFILE.println(latString); 
  DATAFILE.print("GPS_Longitude: ");   DATAFILE.println(longString);
  DATAFILE.print("GPS_Altitude: ");   DATAFILE.println(gpsaltitude);
  DATAFILE.print("GPS_NumSats: ");   DATAFILE.println(numSats);
  DATAFILE.close();  

  if (fixType > 0) {
    if (ExOnce==0){
      //digitalWrite(13, HIGH);
    }
  }
}




// Reads a line from the GPS NMEA serial output
// Give up after trying to read 1000 bytes (~2 seconds)
int readLine(void) {
  char c;
  byte bufferIndex = 0;
  boolean startLine = 0;
  byte retries = 0;
  while (retries < 20) {
    c = Serial1.read();
    
    if (c == -1) { 
      //delay(2); 
      continue; 
    }

    if (c == '\n') continue;
    if (c == '$') startLine = 1;
    if ((bufferIndex == 99) || (c == '\r')) {
      if (startLine) {
        GPSBUFFER[bufferIndex] = 0;
        
        DATAFILE = SD.open("datalog.txt", FILE_WRITE);    
        DATAFILE.print("GPS_NMEA: ");   DATAFILE.println(String(GPSBUFFER));
        DATAFILE.close(); 
        
        return 1;
      }
    }
    if (startLine)
      GPSBUFFER[bufferIndex++] = c;
    //} 
    else {
      retries++;
      //delay(50);
    }
  }
  

  
  return 0;
}

// Returns a specific field from the buffer
void getField(int getId, char *field, int maxLen) {
  byte bufferIndex = 0;
  byte fieldId = 0;
  byte i = 0;
  while (bufferIndex < sizeof(GPSBUFFER)) {
    if (fieldId == getId) {
      // End of string, or string overflow
      if (GPSBUFFER[bufferIndex] == ',' || i > (maxLen - 2)) {
        field[i] = 0;   // Null terminate
        return;
      }
      // Buffer chars to field
      field[i++] = GPSBUFFER[bufferIndex++];
    } 
    else {
      // Advance field on comma
      if (GPSBUFFER[bufferIndex] == ',') {
        bufferIndex++;          //Advance in  buffer
        fieldId++;              // Increase field     position counter
      } 
      else {
        bufferIndex++;          // Advance in  buffer
      }
    }
  }
  // Null terminate incase we didn't already..
  field[i] = 0;
}

// Polls for an NMEA sentence of type requested
// Validates checksum, silently retries on failed checksums
int getNMEA(char *getType) {
  char type[7];
  byte retries = 0;
  while (retries < 2) {
    if (readLine()) {
      ;
      getField(0, type, sizeof(type));
      if (strcmp(type, getType) == 0) {
        return 1;
      }
    } 
    else {
      retries++;
    }
  }


  return 0;
}


// Return the fix type from a GGA string
int getFixType(void) {
  char field[5];
  getField(6, field, sizeof(field));
  int fixType = atoi(field);

  return fixType;
}

// Return the altitude in meters from a GGA string
long getAlt(void) {
  char field[10];
  getField(9, field, sizeof(field));
  long altitude = atol(field);
  return altitude;
}

// Returns the number of satellites being tracked from a GGA string
int getSats(void) {
  char field[3];
  getField(7, field, sizeof(field));
  int numSats = atoi(field);
  return numSats;
}

// Read the latitude in decimal format from a GGA string
float getLat(void) {
  char field[12];
  getField(2, field, sizeof(field));   // read the latitude
  float latitude = atof(field);       // convert to a  double (precise)
  int deg = (int) latitude / 100;      // extract the number of degrees
  float min = latitude - (100 * deg); // work out the number of minutes
  latitude = deg + (float) min/60.0;  // convert to decimal format
  getField(3, field, sizeof(field));   // get the hemisphere (N/S)

  // sign the decimal latitude correctly
  if (strcmp(field, "S") == 0)
    latitude *= -1;

  return latitude;
}

// Read the longitude in decimal format from a GGA string
float getLong(void) {
  char field[12];
  getField(4, field, sizeof(field));    // read the longitude
  float longitude = atof(field);       // convert to a double
  int deg = (int) longitude / 100;      // extract the number of degrees
  float min = longitude - (100 * deg); // work out the number of minutes
  longitude = deg + (float) min/60.00; // convert to decimal format
  getField(5, field, sizeof(field));    // get the E/W status

  // sign decimal latitude correctly
  if (strcmp(field, "W") == 0) 
    longitude *= -1;

  return longitude;
}

// Converts UTC time to the correct timezone
void convertTime(int *time) {
  // How many hours off GMT are we?
  float offset = -5;
  long sectime = ((long)(time[0]) * 3600) + (time[1] * 60) + time[2];
  sectime += (offset * 3600.0);
  // Did we wrap around?
  if (sectime < 0) sectime += 86400;
  if (sectime > 86400) sectime -= 86400;
  // Convert back to time
  time[0] = (int)(sectime / 3600);
  time[1] = (int)((sectime % 3600) / 60);
  time[2] = (int)((sectime % 3600) % 60);
}

// Parses a time field from a GGA string
void parseTime(char *field, int *time) {
  char tmp[3]; 
  tmp[2] = 0; // Init tmp and null terminate
  tmp[0] = field[0]; 
  tmp[1] = field[1]; 
  time[0] = atoi(tmp); // Hours
  tmp[0] = field[2]; 
  tmp[1] = field[3]; 
  time[1] = atoi(tmp); // Minutes
  tmp[0] = field[4]; 
  tmp[1] = field[5]; 
  time[2] = atoi(tmp); // Seconds
}

// Gets the hours, minutes and seconds from a GGA string
void getTime(int *time) {
  char field[12];
  getField(1, field, sizeof(field));
  parseTime(field, time);
  convertTime(time);
}

/* ------------------------- Utility Functions -----------------------------*/
// Returns a string with a textual representation of a float
void doubleToString(double val, int precision, char *string){

  // Print the int part
  sprintf(string, "%d", (int)(val));
  if(precision > 0) {
    // Print the decimal point
    strcat(string, ".");
    unsigned long frac;
    unsigned long mult = 1;
    int padding = precision -1;
    while (precision--) { 
      mult *=10; 
    }
    if (val >= 0)
      frac = (val - (int)(val)) * mult;
    else
      frac = ((int)(val)- val ) * mult;
    unsigned long frac1 = frac;
    while (frac1 /= 10) { 
      padding--; 
    }
    while (padding--) { 
      strcat(string, "0"); 
    }

    // Convert and print the fraction part
    sprintf(string+strlen(string), "%d", (int)(frac));
  }
}



// Converts a HEX string to an int
int atoh(char c) {
  if (c >= 'A' && c <= 'F')
    return c - 55;
  else if (c >= 'a' && c <= 'f')
    return c - 87;
  else
    return c - 48;
} 

