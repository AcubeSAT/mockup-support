#include <stm32f10x.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include "delay.h"
#include "nrf24.h"
#include "BMP180.h"
#include "MPU9250.h"

int main(void)
{
	uint8_t nRF24_payload[32]; //Buffer to store a payload of maximum width	
	int16_t accelData[3], gyroData[3];
	
	char dataString[100];
	
	MPU9250Init(); //Initialize MPU9250, includes I2C2 init
	BMP180_Init(); //Initialize BM180, which icludes I2C1 init
	UART_Init(115200);
	
	nRF24_GPIO_Init();
	nRF24_Init(); //Initialize the nRF24L01 to its default state
	Delay_Init(); //Initialize delays
	
	nRF24_CE_L(); //RX/TX disabled

	UART_SendStr("nRF24L01+ check: ");
	if (!nRF24_Check()) 
	{
		UART_SendStr("FAIL\r\n");
		while (1);
	}
	UART_SendStr("OK\r\n");
	
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
	static const uint8_t nRF24_ADDR[] = { '2', 'N', 'o', 'd', 'e' };
	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR); //Program TX address
	nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR); //Program address for pipe#0, must be same as TX (for Auto-ACK)

	nRF24_SetTXPower(nRF24_TXPWR_0dBm);	//Set TX power (maximum)
	nRF24_SetAutoRetr(nRF24_ARD_2500us, 10); //Configure auto retransmit: 10 retransmissions with pause of 2500s in between
	nRF24_EnableAA(nRF24_PIPE0); //Enable Auto-ACK for pipe#0 (for ACK packets)
	
	nRF24_SetOperationalMode(nRF24_MODE_TX); //Set operational mode (PTX == transmitter)
	nRF24_ClearIRQFlags(); //Clear any pending IRQ flags
	nRF24_SetPowerMode(nRF24_PWR_UP); //Wake the transceiver

	
	while (1)
	{
		MPU9250ReadAccelDataRaw(accelData);
		MPU9250ReadGyroDataRaw(gyroData);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "A: X=%d, Y=%d, Z=%d", accelData[0], accelData[1], accelData[2]);
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "G: X=%d, Y=%d, Z=%d", gyroData[0], gyroData[1], gyroData[2]);
		nRF24_TransmitPacket(nRF24_payload, 32);
		
		memset((uint8_t *)nRF24_payload, '\0', 32); //Fill all the array space with zeros
		sprintf((char *)nRF24_payload, "Press: %.2fhPa, Temp: %.1fC", BMP180_Get_hPa_Press(), BMP180_Get_Celcius_Temp());
		nRF24_TransmitPacket(nRF24_payload, 32);
				
		Delay_ms(500);
	}
}
