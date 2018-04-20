/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

long lastTime = 0;
float r_temperature, r_magx, r_magy, r_magz, r_pressure, r_battery, r_level;
float signalAverage = 0.8;

const float signal_refresh_per_second = 20;

void setup() {
  Serial.begin(115200); // Debugging only
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

  if (millis() - lastTime >= (1000.0)/signal_refresh_per_second) {
    lastTime = millis();

    r_magy = analogRead(A1);
    r_magz = analogRead(A2);
    r_magx = analogRead(A0);
    r_temperature = analogRead(A3) / 1024.0 * 50.0;
    r_pressure = analogRead(A4);
    r_battery = min(r_battery,5.5) + random(0,100)/1000.0;

      Serial.print((float) r_temperature, 1); // 1 decimal accuracy
      Serial.print(" ");
      Serial.print((float) r_magx, 2);
      Serial.print(" ");
      Serial.print((float) r_magy, 2);
      Serial.print(" ");
      Serial.print((float) r_magz, 2);
      Serial.print(" ");
      Serial.print((float) r_pressure, 2);
      Serial.print(" ");
      Serial.print((float) r_battery, 2);
      Serial.print(" ");
      Serial.print((float) signalAverage * 100, 1);
    
      Serial.println("");
  }
}

