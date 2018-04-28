#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "uart.h"
#include "delay.h"
#include "nrf24.h"
#include "MPU6050.h"
#include "BH1750.h"
//#include "MadgwickAHRS.h"

volatile uint8_t stopRX = 0; //Logic variable to indicate the stopping of the RX

int main(void)
{
	uint8_t nRF24_payload[32]; //Buffer to store a payload of maximum width	
	
	//Variables used to keep the time, which is necessary for the quaternion update
	//uint16_t timeCountCur = 0;
	//uint16_t timeCountPrev = 0;
	
	double bright = 0.0; //Save the brightness received
	float acgrData[6]; //Save the data of the sensors
	float gyrCal[3]; //Save the calibration values
	//float sampFreq = 0.0; //Update time for integration used in quatenion
	
	//float yaw = 0.0, pitch = 0.0, roll = 0.0;
	
	uint8_t payload_length; //Length of received payload
	char* tokenCh = NULL; //Save the tokenized string
	
	//LED Pins Init
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //Enabling the clock of C pins.
	GPIOB->CRH |= GPIO_CRH_MODE8|GPIO_CRH_MODE9; //Resetting the bits of the register besides the last 4.
	
	//Timer Initialization
	//TODO make the timer for 50ms
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //Enable Timer 3 clock
	//RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM3->PSC = 18000; //Set prescaler to 18000 (ticks)
	//TIM4->PSC = 18000;
	TIM3->ARR = 150; //Set auto reload to aprrox. 75 (ms)
	//TIM4->ARR = 250; //Around 500ms

	NVIC_EnableIRQ(TIM3_IRQn); //Enable Timer 3 interrupt
	TIM3->DIER = TIM_DIER_UIE; //Enable Timer 3 interrupt
	
	UART_Init(115200); //Initialize the UART with the set baud rate
	Delay_Init(); //Initialize the delay
		
	MPU6050_I2C_Init(); //Initialize I2C
	MPU6050_Initialize(); //Initialize the MPU6050
	MPU6050_GyroCalib(gyrCal); //Get the gyroscope calibration values
	MPU6050_SetFullScaleGyroRange(MPU6050_GYRO_FS_2000); //Set the gyroscope scale to full scale
	MPU6050_SetFullScaleAccelRange(MPU6050_ACCEL_FS_2); //Set the accelerometer scale
	
	BH1750_Init(BH1750_CONTHRES); //I2C is already initialized above
		
	nRF24_GPIO_Init(); //Start the pins used by the NRF24
	nRF24_Init(); //Initialize the nRF24L01 to its default state
	
	nRF24_CE_L(); //RX/TX disabled
	
	//A small check for debugging
	UART_SendStr("nRF24L01+ check: ");
	if (!nRF24_Check()) 
	{
		UART_SendStr("FAIL\r\n");
		while (1);
	}
	else
	{UART_SendStr("OK\r\n");}
	
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
	//TIM4->CR1 = TIM_CR1_CEN; //Enable TIM4 timer
	
	while (1)
	{
		MPU6050_GetCalibAccelGyro(acgrData, gyrCal); //Get the accelerometer and gyroscope data
		
		/*timeCountCur = TIM4->CNT; //Get the timer count
		sampFreq = (float)(1.0/((timeCountCur - timeCountPrev)*0.0005)); //Get the past time from the previous run
		timeCountPrev = timeCountCur; //Save the value of the previous count
		TIM4->CNT = 0x0000; //Reset the timer*/
		
		///MadgwickAHRSupdateIMU(acgrData[3], acgrData[4], acgrData[5], acgrData[0], acgrData[1], acgrData[2], sampFreq);
		bright = BH1750_GetBrightnessCont();
		
		/*yaw = atan2f(2.0*(q0*q3 + q1*q2), 1 - 2*(q2*q2 + q3*q3));
		roll = atan2f(2.0*(q0*q1 + q2*q3), 1 - 2*(q1*q1 + q2*q2));
		pitch = asin(2.0*(q0*q2 - q3*q1));*/
		
		//yaw += 4.66; //Compensate for magnetic declination
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "B%.2f", bright);
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		/*memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "B%.2f %.2f %.2f %.2f", q0, q1, q2, q3);
		nRF24_TransmitPacket(nRF24_payload, 32);*/
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "X%d %d", (int32_t)(acgrData[0]*100000.0), (int32_t)(acgrData[3]*100000.0));
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "Y%d %d", (int32_t)(acgrData[1]*100000.0), (int32_t)(acgrData[4]*100000.0));
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "Z%d %d", (int32_t)(acgrData[2]*100000.0), (int32_t)(acgrData[5]*100000.0));
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		//Start receiving and do that until time limit has been reched
		nRF24_SetOperationalMode(nRF24_MODE_RX); //Set operational mode (PRX == receiver)
		nRF24_CE_H(); //Put the transceiver to the RX mode
		TIM3->CR1 = TIM_CR1_CEN; //Start the timer
		
		while(!stopRX)
		{
			if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY)
			{
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
			}
		}
		stopRX = 0; //Reset the stop condition
		nRF24_SetOperationalMode(nRF24_MODE_TX); //Set operational mode (PTX == transmitter)
		nRF24_ClearIRQFlags(); //Clear any pending IRQ flags
				
		Delay_ms(5);
	}
}

void TIM3_IRQHandler()
{
	if (TIM3->SR & TIM_SR_UIF) 
	{
		TIM3->SR &= ~TIM_SR_UIF;
		TIM3->CR1 = 0x00; //Stop the timer
		stopRX = 1; //Indicate that we have to stop
	}
}
