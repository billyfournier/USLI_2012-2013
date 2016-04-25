/* HIH4030 Humidity Sensor
   Connect to 5V.
References:
  https://www.sparkfun.com/products/9569
  http://www.arduino.cc/forum/index.php/topic,19961.0.html 
  
*/
int HIH4030_readRH(int pin, float temp){
  int raw = analogRead(pin);
  float rv;
  // This formula comes straight from the second reference.
  rv = ((0.0004*temp + 0.149)*float(raw))-(0.0617*temp + 24.436);
  rv = rv * 100;
  
  DATAFILE = SD.open("datalog.txt", FILE_WRITE);             //
  DATAFILE.print("HIH4030_pin: "); DATAFILE.println(pin);
  DATAFILE.print("HIH4030_voltage: "); DATAFILE.println(raw);
  DATAFILE.print("HIH4030_RH: "); DATAFILE.println(rv);
  DATAFILE.close();
  
  return int(rv);  
}
