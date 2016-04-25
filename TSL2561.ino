/*
 TSL2561 sensor
  
 References:
   http://learn.adafruit.com/tsl2561/overview
   
 connect VDD to 3.3V DC
 connect GROUND to common ground
 ADDR can be connected to ground, or vdd or left floating to change the i2c address

 The address will be different depending on whether you let
 the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
 use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
*/

TSL2561 tsl(TSL2561_ADDR_FLOAT);

void TSL2561_calibration() {
  
  tsl.begin();
    
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2561_GAIN_0X); // set no gain (for bright situtations)
  tsl.setGain(TSL2561_GAIN_16X); // set 16x gain (for dim situations)
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS); // shortest integration time (bright light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS); // medium integration time (medium light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS); // longest integration time (dim light)
  
  // Now we're ready to get readings!
}

/*
*   The problem is that I don't know what units the raw readings are in.
*   Once they are converted to lux it is simple to convert to 
*   W/m^2 for a single wavelength but not for a large range.
*/

int TSL2561_getLux(){
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparions you want!
  int lux;
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  
  lux = tsl.calculateLux(full, ir);
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);  
  DATAFILE.print("TSL_IR: ");   DATAFILE.println(ir); 
  DATAFILE.print("TSL_Full: ");   DATAFILE.println(full); 
  DATAFILE.print("TSL_Lux: ");   DATAFILE.println(lux);
  DATAFILE.close();  
  
  return lux;
}
