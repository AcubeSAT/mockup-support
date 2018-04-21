#include <SPI.h> //It has to be included before including RF24, because it is used there
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define BUTTON_PIN 4

/* NRF24L01+ library link: https://github.com/nRF24/RF24 */

/* NRF24L01 connection to Arduino:
 * Connect Vcc to 3.3V, CE to PIN 7, CSN to PIN 8, SCK to PIN 13, MOSI to PIN 11, MISO to PIN 12
 * The pin numbering on the NRF24 starts from the pin with the white square which is pin 1.
 * The one next to this pin is number 2. The column that contains the first pin has an odd numbering
 * and the column next to it has an even numbering.
 * On the NRF24 PIN 1 is GND, PIN 2 in Vcc, PIN 3 is CE, PIN 4 is CSN, PIN 5 is SCK, PIN 6 is MOSI, PIN 7 is MISO and PIN 8 is the IRQ.
 * For the moment the IRQ pin is not used, so it is not connected on the board. The paln for a future use is to include the IRQ pin.
 */
RF24 radio(7,8); //Make the radio object from the RF24 class and assign CE to PIN 7 and CSN to PIN 8

uint8_t address[6] = "SesaB"; //Address of the dongles (For some reason the address is saved in reverse order)
uint8_t address_tx[6] = "SebuC"; //Address of the receiving end
uint8_t r_data[32] = ""; //Save the received data
uint8_t i = 0; //Indexing variable used in printing out the received data

uint8_t nRF24_payload[32];
uint8_t ledStat = 0;

uint8_t buttonState = 0; //Save the state of the button
uint8_t lastButtonState = 0;

void setup() 
{
  Serial.begin(115200); //Initialize the USART to 115200 baud
  radio.begin(); //Initialize the NRF
  //printf_begin(); //Initialize the printing funcion (used only for debugging in printDetails)

  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
  
  //Serial.println("The program has started."); //Indicate that we are running
  
  /*if(radio.isChipConnected()) //Check whether the cip is connected or not
  {Serial.println("Chip is connected");}*/

  radio.setPALevel(RF24_PA_MAX); //Set the TX power to the highest possible
  radio.setDataRate(RF24_2MBPS); //Set the data rate to 2Mbps
  radio.setCRCLength(RF24_CRC_16); //Set the CRC length to 2 bytes
  radio.setChannel(99); //Set the TX frequency to 2499MHz
  radio.setPayloadSize(32); //Set the payload size to 32 bytes (Not necessary, just added for completeness)

  radio.openWritingPipe(address_tx); //Set the address for the transmitting pipe
  radio.openReadingPipe(1, address); //Set the receiving address for the pipe

  //radio.printDetails(); //Used in debugging to print out some register values of the NRF

  radio.startListening(); //Put the NRF into receiving mode
}

void loop() 
{
  /*
   * Get inside only when there is data. A better implementation would be to use the IRQ pin from the NRF.
   * In a future version of this code, this will be included to enchance performance, by using interrupts.
  */
  radio.startListening();
  if(radio.available())
  {
    //Serial.println("We received something."); //Used in debugging the code
    while (radio.available()) //Loop while there is more data in the RX FIFO
    {
      radio.read(&r_data, sizeof(r_data)); //Read the data from the RX FIFO
      //Serial.print("Received some data: "); //Indicate that we have received something from the RX FIFO
      
      /*while(r_data[i] != '\0') //Print the received data (if the received data is not a string, that implementation is dangerous)
      {Serial.print((char)r_data[i++]);} //Print each character*/
      
      //Serial.println(""); //Print a new line to have a clearer output
      i = 0; //Reset the counting variable for the next round
    }
  }

  buttonState = digitalRead(BUTTON_PIN);
  if(buttonState != lastButtonState)
  {
    Serial.println("We are in the button");
    if (buttonState == LOW)
    {
      radio.stopListening();
      ledStat ^= 1;
      Serial.println(ledStat);
      memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
      sprintf((char *)nRF24_payload, "L1:%d", ledStat);
      Serial.println((char *)nRF24_payload);
      radio.write( nRF24_payload, sizeof(nRF24_payload));

      delay(10);
    }
  }
  lastButtonState = buttonState; //Save it avoid getting in again
  
  //delay(100);
  /*radio.stopListening();
  ledStat ^= 1;
  memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
  sprintf((char *)nRF24_payload, "L1:%d", ledStat);*/
  //radio.write( nRF24_payload, sizeof(nRF24_payload));
  /*if(!radio.write( nRF24_payload, sizeof(nRF24_payload) ))
  {Serial.println("Failed to send");}*/
  //delay(50);
}
