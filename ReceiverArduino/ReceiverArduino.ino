#include <SPI.h> //It has to be included before including RF24, because it is used there
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <LiquidCrystal.h>

const int BRIGHTNESS_MAX = 32767;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/* NRF24L01+ library link: https://github.com/nRF24/RF24 */

/* NRF24L01 connection to Arduino:
 * Connect Vcc to 3.3V, CE to PIN 7, CSN to PIN 8, SCK to PIN 13, MOSI to PIN 11, MISO to PIN 12
 * The pin numbering on the NRF24 starts from the pin with the white square which is pin 1.
 * The one next to this pin is number 2. The column that contains the first pin has an odd numbering
 * and the column next to it has an even numbering.
 * On the NRF24 PIN 1 is GND, PIN 2 in Vcc, PIN 3 is CE, PIN 4 is CSN, PIN 5 is SCK, PIN 6 is MOSI, PIN 7 is MISO and PIN 8 is the IRQ.
 * For the moment the IRQ pin is not used, so it is not connected on the board. The paln for a future use is to include the IRQ pin.
 */
RF24 radio(9,10); //Make the radio object from the RF24 class and assign CE to PIN 7 and CSN to PIN 8

uint8_t address[6] = "SesaB"; //Address of the dongles (For some reason the address is saved in reverse order)
uint8_t address_tx[6] = "SebuC"; //Address of the receiving end
uint8_t r_data[32] = ""; //Save the received data
uint8_t i = 0; //Indexing variable used in printing out the received data

uint8_t nRF24_payload[32];
uint8_t ledStat = 0;

volatile int values[9];

void setup() 
{
  Serial.begin(115200); //Initialize the USART to 115200 baud
  radio.begin(); //Initialize the NRF
  //printf_begin(); //Initialize the printing funcion (used only for debugging in printDetails)
  
  Serial.println("The program has started."); //Indicate that we are running
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("CUBESAT BASE");
  lcd.setCursor(0,1);
  lcd.print("STATION RECEIVER");

  pinMode(14,OUTPUT);
  digitalWrite(14, HIGH);
  for (int i = 9; i >= 0; i--) { lcd.setCursor(15,0); lcd.print(i); delay(100); }
  lcd.setCursor(15,0); lcd.print(' ');
  digitalWrite(14, LOW);
  
  /*if(radio.isChipConnected()) //Check whether the cip is connected or not
  {Serial.println("Chip is connected");}*/

  radio.setPALevel(RF24_PA_MAX); //Set the TX power to the highest possible
  radio.setDataRate(RF24_2MBPS); //Set the data rate to 2Mbps
  radio.setCRCLength(RF24_CRC_16); //Set the CRC length to 2 bytes
  radio.setChannel(99); //Set the TX frequency to 2499MHz
  radio.setPayloadSize(32); //Set the payload size to 32 bytes (Not necessary, just added for completeness)

  radio.openWritingPipe(address_tx);
  radio.openReadingPipe(1, address); //Set the receiving address for the pipe

  //radio.printDetails(); //Used in debugging to print out some register values of the NRF

  radio.startListening(); //Put the NRF into receiving mode
  Serial.println("hello");

  pinMode(8, OUTPUT);
}

void loop() 
{
  /*
   * Get inside only when there is data. A better implementation would be to use the IRQ pin from the NRF.
   * In a future version of this code, this will be included to enchance performance, by using interrupts.
  */
  radio.startListening();
  if (Serial.available()) {
    digitalWrite(14, HIGH);
    char c = Serial.read();
    if (c == 'l') {
      lcd.setCursor(0,1);
      lcd.print("Sent sat. comm");

      radio.stopListening();
      ledStat ^= 1;
      memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
      sprintf((char *)nRF24_payload, "L1:%d", ledStat);

      radio.write( nRF24_payload, sizeof(nRF24_payload));
      delay(11);
      radio.write( nRF24_payload, sizeof(nRF24_payload));
      delay(11);
      radio.write( nRF24_payload, sizeof(nRF24_payload));
      delay(11);
      radio.write( nRF24_payload, sizeof(nRF24_payload));
      delay(11);
      radio.write( nRF24_payload, sizeof(nRF24_payload));
      delay(11);
      radio.write( nRF24_payload, sizeof(nRF24_payload));
      delay(11);
      radio.write( nRF24_payload, sizeof(nRF24_payload));
    } else if (c == 'r') {
      void (*reset)(void) = 0;
      reset();
    }
    digitalWrite(14, LOW);
  }
  if(radio.available())
  {
    //Serial.println("We received something."); //Used in debugging the code
    while (radio.available()) //Loop while there is more data in the RX FIFO
    {
      digitalWrite(8, HIGH);
      radio.read(&r_data, sizeof(r_data)); //Read the data from the RX FIFO
      digitalWrite(8, LOW);
      //Serial.print("Received some data: "); //Indicate that we have received something from the RX FIFO
      
      /*while(r_data[i] != '\0') //Print the received data (if the received data is not a string, that implementation is dangerous)
      {Serial.print((char)r_data[i++]);
      } //Print each character*/

      int v1,v2,v3;
      switch (r_data[0]) {
        // Always ignore the first ID character
        case 'A':
          // Various data
          sscanf(r_data + 1, "%d", &(values[6]));
          // Process brightness with gamma
          values[6] = BRIGHTNESS_MAX*pow(values[6]/32387.0,1);
          if(values[6] < 0) values[6] = BRIGHTNESS_MAX;
          break;
        case 'B':
          // Acceleration data
          sscanf(r_data + 1, "%d %d %d", &v1, &v2, &v3);
          values[0] = v1; values[1] = v2; values[2] = v3;
          break;
        case 'C':
          // Gyroscope data
          sscanf(r_data + 1, "%d %d %d", &v1, &v2, &v3);
          values[3] = v1; values[4] = v2; values[5] = v3;
          break;
      }

      Serial.print(values[0]);
      Serial.print(' ');
      Serial.print( values[1] );
      Serial.print(' ');
      Serial.print(values[2]);
      Serial.print(' ');
      Serial.print(values[3]);
      Serial.print(' ');
      Serial.print(values[4]);
      Serial.print(' ');
      Serial.print(values[5]);
      Serial.print(' ');
      Serial.print(values[6]);
      Serial.print(' ');
      Serial.print(values[7]);
      Serial.print(' ');
      Serial.print(values[8]);
      Serial.print(' ');
      Serial.print("\n");
      //Serial.println(""); //Print a new line to have a clearer output
      //Serial.print("Received brightness: ");
      //Serial.println(values[6]);

      if (true) {
        //lcd.setCursor(0,0);
        //lcd.print("Temp: ");
        //lcd.print((values[1])/10.0);
        
        lcd.setCursor(0,1);
        lcd.print("Br:[");
        for (int i = 0; i < 11; i++) {
          if (values[6] > BRIGHTNESS_MAX/12 * i) {
            lcd.print("=");
          } else {
            lcd.print(" ");
          }
        }
        lcd.print("]");
      }
      
      i = 0; //Reset the counting variable for the next round
      
    }
  }

}
