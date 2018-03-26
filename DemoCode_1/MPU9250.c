#include "MPU9250.h"

void MPU9250Init(void)
{
	uint8_t regVal = 0;
	
	Delay_Init();
	TWIInit2(I2C2_INIT); //Initialize I2C2 interface
	
	TWIWriteByte2(MPU9250_ADDR, PWR_MGMT_1, 0x00); //Clear the sleep mode bit and enable all the sensors
	Delay_ms(100); //Wait for the registers to reset
	TWIWriteByte2(MPU9250_ADDR, PWR_MGMT_1, 0x01);
	Delay_ms(200);
	
	//Disable FSYNC pin and set the thermometer and gyro bandwidth to 41 and 42 Hz respectively
	//Minimum delay for the the setting is 5.9ms, so update can not be higher than 1/0.0059=170Hz
	//Setting the DLPF_CFG[2:0] = 011, sets the sample rate to 1kHz for both
	TWIWriteByte2(MPU9250_ADDR, CONFIG, 0x03);
	
	//Use a 200Hz sample rate
	TWIWriteByte2(MPU9250_ADDR, SMPLRT_DIV, 0x04);
	
	regVal = TWIReadByte2(MPU9250_ADDR, GYRO_CONFIG);
	regVal &= ~(0x02); //Clear Fchoice bits[1:0]
	regVal &= ~(0x18); //Clear AFS[4:3]
	regVal |= 0x03 << 3; //Set the gyro to fullscale
	TWIWriteByte2(MPU9250_ADDR, GYRO_CONFIG, regVal);
	
	regVal = TWIReadByte2(MPU9250_ADDR, ACCEL_CONFIG);
	regVal &= ~(0x18);
	regVal |= 0x03 << 3;
	TWIWriteByte2(MPU9250_ADDR, ACCEL_CONFIG, regVal);
	
	regVal = TWIReadByte2(MPU9250_ADDR, ACCEL_CONFIG2);
	regVal &= ~(0x0F);
	regVal |= 0x03;
	TWIWriteByte2(MPU9250_ADDR, ACCEL_CONFIG2, regVal);
	
	TWIWriteByte2(MPU9250_ADDR, INT_PIN_CFG, 0x22);
	TWIWriteByte2(MPU9250_ADDR, INT_ENABLE, 0x01);
	
	Delay_ms(100);
}

void MPU9250Calibration(void)
{
	
}

void MPU9250ReadAccelDataRaw(int16_t *acceleration)
{
	uint8_t data[6]; //Save X,Y,Z acceleration data
	
	TWIReadBytes2(MPU9250_ADDR, ACCEL_XOUT_H, data, 6);
	
	acceleration[0] = ((int16_t)data[0] << 8)|data[1];
	acceleration[1] = ((int16_t)data[2] << 8)|data[3];
	acceleration[2] = ((int16_t)data[4] << 8)|data[5];
}

void MPU9250ReadGyroDataRaw(int16_t *angular)
{
	uint8_t data[6]; //Save X,Y,Z angular velocity data
	
	TWIReadBytes2(MPU9250_ADDR, GYRO_XOUT_H, data, 6);
	
	angular[0] = ((int16_t)data[0] << 8)|data[1];
	angular[1] = ((int16_t)data[2] << 8)|data[3];
	angular[2] = ((int16_t)data[4] << 8)|data[5];
}

int16_t MPU9250ReadTempDataRaw(void)
{
	uint8_t data[2]; //Save the received temperature bytes
	
	TWIReadBytes2(MPU9250_ADDR, TEMP_OUT_H, data, 2);
	
	return ((int16_t)data[0] << 8)|data[1];
}

void MPU9250GetAcceleration(int16_t *acc)
{
	int16_t data[3]; //Save X,Y,Z acceleration data
	float multFactor = 0;
	
	MPU9250ReadAccelDataRaw(data);
	
	//Wehave to get the current scale
	
}
