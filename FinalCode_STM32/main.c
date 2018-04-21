#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "delay.h"
#include "nrf24.h"
#include "MPU6050.h"
#include "BH1750.h"

volatile uint8_t stopRX = 0; //Logic variable to indicate the stopping of the RX

int main(void)
{
	uint8_t nRF24_payload[32]; //Buffer to store a payload of maximum width	
		
	int16_t accelgyro[6];
	int16_t tempRead = 0;
	
	double bright = 0.0; //Save the brightness received
	
	uint8_t payload_length; //Length of received payload
	char* tokenCh = NULL; //Save the tokenized string
	
	//LED Init
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //Enabling the clock of C pins.
	GPIOB->CRH |= GPIO_CRH_MODE8|GPIO_CRH_MODE9; //Resetting the bits of the register besides the last 4.
	
	//Timer Initialization
	//TODO make the timer for 50ms
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //Enable Timer 3 clock
	TIM3->PSC = 18000; //Set prescaler to 18000 (ticks)
	TIM3->ARR = 300; //Set auto reload to aprrox. 50 (ms)

	NVIC_EnableIRQ(TIM3_IRQn); //Enable Timer 3 interrupt
	TIM3->DIER = TIM_DIER_UIE; //Enable Timer 3 interrupt
	
	UART_Init(115200);
	Delay_Init();
		
	MPU6050_I2C_Init(); //Initialize I2C
	MPU6050_Initialize(); //Initialize the MPU
	MPU6050_SetFullScaleGyroRange(MPU6050_GYRO_FS_2000);
	MPU6050_SetFullScaleAccelRange(MPU6050_ACCEL_FS_2);
	
	BH1750_Init(BH1750_CONTHRES); //I2C is already initialized above
		
	nRF24_GPIO_Init();
	nRF24_Init(); //Initialize the nRF24L01 to its default state
	
	nRF24_CE_L(); //RX/TX disabled

	UART_SendStr("nRF24L01+ check: ");
	if (!nRF24_Check()) 
	{
		UART_SendStr("FAIL\r\n");
		while (1);
	}
	else
	{
		UART_SendStr("OK\r\n");
	}
	
	// This is simple transmitter with Enhanced ShockBurst (to one logic address):
	//   - TX address: 'ESB'
	//   - payload: 10 bytes
	//   - RF channel: 40 (2440MHz)
	//   - data rate: 2Mbps
	//   - CRC scheme: 2 byte
	nRF24_SetRFChannel(99); //Set RF channel
	nRF24_SetDataRate(nRF24_DR_2Mbps); //Set data rate
	nRF24_SetCRCScheme(nRF24_CRC_2byte); //Set CRC scheme
	nRF24_SetAddrWidth(5); //Set address width, its common for all pipes (RX and TX)

	//Configure TX PIPE
	static const uint8_t nRF24_ADDR_Tx[] = { 'B', 'a', 's', 'e', 'S' }; //Address of the receiving end
	static const uint8_t nRF24_ADDR_Rx[] = { 'C', 'u', 'b', 'e', 'S' }; //Address of the current module
	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR_Tx); //Program TX address
	nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR_Tx); //Program address for pipe#0, must be same as TX (for Auto-ACK)
	
	//Configure RX PIPE
	nRF24_SetAddr(nRF24_PIPE1, nRF24_ADDR_Rx); //Program address for pipe
	nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, 32); // Auto-ACK: enabled, payload length: 32 bytes

	nRF24_SetTXPower(nRF24_TXPWR_0dBm);	//Set TX power (maximum)
	nRF24_SetAutoRetr(nRF24_ARD_2500us, 10); //Configure auto retransmit: 10 retransmissions with pause of 2500s in between
	nRF24_EnableAA(nRF24_PIPE0); //Enable Auto-ACK for pipe#0 (for ACK packets)
	
	nRF24_SetOperationalMode(nRF24_MODE_TX); //Set operational mode (PTX == transmitter)
	nRF24_ClearIRQFlags(); //Clear any pending IRQ flags
	nRF24_SetPowerMode(nRF24_PWR_UP); //Wake the transceiver
	
	Delay_ms(100); //Let some time to set things up
	
	while (1)
	{
		MPU6050_GetRawAccelGyro(accelgyro, tempRead);
		//tempRead = ((tempRead + 12412.0)/340.0)*100.0;
		bright = BH1750_GetBrightnessCont();
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "A%.2f", bright);
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "B%d %d %d", accelgyro[0], accelgyro[1], accelgyro[2]);
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "C%d %d %d", accelgyro[3], accelgyro[4], accelgyro[5]);
		nRF24_TransmitPacket(nRF24_payload, 32);
				
		nRF24_SetOperationalMode(nRF24_MODE_RX); //Set operational mode (PRX == receiver)
		nRF24_CE_H(); //Put the transceiver to the RX mode
		TIM3->CR1 = TIM_CR1_CEN; //Start the timer
		while(!stopRX)
		{
			if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY)
			{
				TIM3->CR1 = 0x0; //Stop the timer
				stopRX = 0; //Reset the stop condition
				nRF24_ReadPayload(nRF24_payload, &payload_length); //Get the payload from the transceiver
				nRF24_ClearIRQFlags(); //Clear all pending IRQ flags

				tokenCh = strtok((char*)nRF24_payload, ":");
				if(strstr(tokenCh, "L1"))
				{
					tokenCh = strtok (NULL, ":");
					if(strstr(tokenCh, "1"))
					{
						GPIOB->ODR &= ~GPIO_BSRR_BS8;
						GPIOB->ODR &= ~GPIO_BSRR_BS9;
					}
					else if(strstr(tokenCh, "0"))
					{
						GPIOB->ODR |= GPIO_BSRR_BS8;
						GPIOB->ODR |= GPIO_BSRR_BS9;
					}
				}
				/*else if(strstr(tokenCh, "L2"))
				{
					tokenCh = strtok (NULL, ":");
					if(*tokenCh)
					{GPIOB->ODR |= GPIO_BSRR_BS5;}
					else
					{GPIOB->ODR &= ~GPIO_BSRR_BS5;}
				}
				else if(strstr(tokenCh, "L3"))
				{
					tokenCh = strtok (NULL, ":");
					if(*tokenCh)
					{GPIOB->ODR |= GPIO_BSRR_BS8;}
					else
					{GPIOB->ODR &= ~GPIO_BSRR_BS8;}
				}
				else if(strstr(tokenCh, "L4"))
				{
					tokenCh = strtok (NULL, ":");
					if(*tokenCh)
					{GPIOB->ODR |= GPIO_BSRR_BS9;}
					else
					{GPIOB->ODR &= ~GPIO_BSRR_BS9;}
				}*/
				
				break;
			}
		}
		TIM3->CR1 = 0x0; //Stop the timer
		stopRX = 0; //Reset the stop condition
		nRF24_SetOperationalMode(nRF24_MODE_TX); //Set operational mode (PTX == transmitter)
		nRF24_ClearIRQFlags(); //Clear any pending IRQ flags
				
		//Delay_ms(25);
	}
}

void TIM3_IRQHandler()
{
	if (TIM3->SR & TIM_SR_UIF) 
	{
		TIM3->SR &= ~TIM_SR_UIF;
		stopRX = 1; //Indicate that we have to stop
	}
}
