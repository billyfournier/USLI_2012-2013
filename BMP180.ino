/* This Header File contains functions needed to operate the BMP180. 
   Discriptions of each funtion are included above the fucntions themselves.
   NOTE: If you change the OSS value, you must also change the delay value
         in the bmp180ReadUT() funtion.   
*/

#define BMP180_ADDRESS 0x77  // I2C address of BMP085 is 0x77

/* NOTE: OSS: can be set between 0-3, which requires 
changes to the delay in the bmpReadUT();
{0 = 4.5ms, 1 = 7.5ms, 2 = 13.5ms, 3 = 25.5ms}
*/
const unsigned char OSS = 3;  // Oversampling Setting 

int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;
long b5; 

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
///////////////////////////////////////////////////////////////////////
short BMP180_getTemperature(unsigned int ut)
{
  long x1, x2;
  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}
///////////////////////////////////////////////////////////////////////


// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
////////////////////////////////////////////////////////////////////////
long BMP180_getPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}
/////////////////////////////////////////////////////////////////////

//Getting altitude
// NOTE:  * 100 to get altitude
///////////////////////////////////////////////////
long BMP180_getAltitude(float pressure, float ini_pressure)
{
  long altitude;
  float x1,x2,x3;
  x1= (pressure/ini_pressure);
  x2= (1/5.255);
  x3= pow(x1,x2);
  altitude = 443300*(1-x3);
  return altitude;
}
///////////////////////////////////////////////////



// Read 1 byte from the BMP085 at 'address'
/////////////////////////////////////////////////
char BMP180_read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP180_ADDRESS, 1);
  while(!Wire.available());
    
  return Wire.read();
}
/////////////////////////////////////////////////


// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
/////////////////////////////////////////////////
int BMP180_readInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP180_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) msb<<8 | lsb;
}
//////////////////////////////////////////////////


// Read the uncompensated temperature value
// NOTE: the delay here may change depending on the Over Sampling selected.
//////////////////////////////////////////////////
unsigned int BMP180_readUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(26);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = BMP180_readInt(0xF6);
  
  
  return ut;
}
/////////////////////////////////////////////////////


// Read the uncompensated pressure value
/////////////////////////////////////////////////////////
unsigned long BMP180_readUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP180_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
 
  
  return up;
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void BMP180_calibration()
{
  ac1 = BMP180_readInt(0xAA);
  ac2 = BMP180_readInt(0xAC);
  ac3 = BMP180_readInt(0xAE);
  ac4 = BMP180_readInt(0xB0);
  ac5 = BMP180_readInt(0xB2);
  ac6 = BMP180_readInt(0xB4);
  b1 = BMP180_readInt(0xB6);
  b2 = BMP180_readInt(0xB8);
  mb = BMP180_readInt(0xBA);
  mc = BMP180_readInt(0xBC);
  md = BMP180_readInt(0xBE);
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);             //
  DATAFILE.println("BMP180_Calibration_values: "); 
  DATAFILE.print("\tac1: "); DATAFILE.println(ac1);
  DATAFILE.print("\tac2: "); DATAFILE.println(ac2);
  DATAFILE.print("\tac3: "); DATAFILE.println(ac3);
  DATAFILE.print("\tac4: "); DATAFILE.println(ac4);
  DATAFILE.print("\tac5: "); DATAFILE.println(ac5);
  DATAFILE.print("\tac6: "); DATAFILE.println(ac6);
  DATAFILE.print("\tb1: "); DATAFILE.println(b1);
  DATAFILE.print("\tb2: "); DATAFILE.println(b2);
  DATAFILE.print("\tmb: "); DATAFILE.println(mb);
  DATAFILE.print("\tmc: "); DATAFILE.println(mc);
  DATAFILE.print("\tmd: "); DATAFILE.println(md);
  DATAFILE.close(); 
}
/////////////////////////////////////////////////////////////////////


//Setting baseline altitude.
//////////////////////////////////////////////////
long BMP180_getPressureIni(){

  long x1 = 0;
  double ini_pressure;
  long total = 0;
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);             //
  DATAFILE.println("BMP180 Initial Pressure Values: "); 
  DATAFILE.print("\t");
  DATAFILE.close(); 
  for(int i=1; i <=20; i++){
    x1 = BMP180_getPressure(BMP180_readUP());
    
    DATAFILE = SD.open("datalog.txt", FILE_WRITE);      
    DATAFILE.print(x1); DATAFILE.print(" ");
    DATAFILE.close(); 
    
    total += x1;
  }
  
  ini_pressure = double(total) / 20;  
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);      
  DATAFILE.print("\n\nAverage Initial Pressure: "); DATAFILE.println(ini_pressure);
  DATAFILE.close(); 
  
  return long(ini_pressure);
}

//Setting baseline temperature.
//////////////////////////////////////////////////
long BMP180_getTemperatureIni(){

  long x1 = 0;
  double ini_temperature;
  long total = 0;
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);             //
  DATAFILE.println("BMP180 Initial Temperature Values: "); 
  DATAFILE.print("\t");
  DATAFILE.close(); 
  
  for(int i=1; i <=20; i++){
    x1 = BMP180_getTemperature(BMP180_readUT());
    
    DATAFILE = SD.open("datalog.txt", FILE_WRITE);      
    DATAFILE.print(x1); DATAFILE.print(" ");
    DATAFILE.close(); 
    
    total += x1;
  }
  
  ini_temperature = double(total) / 20;  
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);      
  DATAFILE.print("\n\nAverage Initial Temperature: "); DATAFILE.println(ini_temperature);
  DATAFILE.close();   
  
  return long(ini_temperature);
}


