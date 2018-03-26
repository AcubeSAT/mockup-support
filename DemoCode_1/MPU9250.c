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

void MPU9250GetAcceleration(double *acc)
{
	int16_t data[3]; //Save X,Y,Z acceleration data
	uint8_t scale = 0; //Save the current scale, as read from the register
	double multFactor = 0.0; //Save the conversion factor for the acelerometer
	
	scale = (TWIReadByte2(MPU9250_ADDR, ACCEL_CONFIG) & (0x03 << 3)) >> 3; //Get the current reading scale
	MPU9250ReadAccelDataRaw(data); //Get the raw accelerometer data
	
	//Select the conversion factor according to scale setting
	switch(scale)
	{
		case 0:
			multFactor = 1.0/16384.0;
			break;
		case 1:
			multFactor = 1.0/8192.0;
			break;
		case 2:
			multFactor = 1.0/4096.0;
			break;
		case 3:
			multFactor = 1.0/2048.0;
			break;
	}
	acc[0] = (double)data[0]*multFactor;
	acc[1] = (double)data[1]*multFactor;
	acc[2] = (double)data[2]*multFactor;
}

void MPU9250GetAngularVel(double *angVel)
{
	int16_t data[3]; //Save raw X,Y,Z angular velocity data
	uint8_t scale = 0; //Save the current scale, as read from the register
	double multFactor = 0.0; //Save the conversion factor for the gyroscope
	
	scale = (TWIReadByte2(MPU9250_ADDR, GYRO_CONFIG) & (0x03 << 3)) >> 3; //Get the current reading scale
	MPU9250ReadGyroDataRaw(data); //Get the raw accelerometer data
	
	//Select the conversion factor according to scale setting
	switch(scale)
	{
		case 0:
			multFactor = 1.0/131.0;
			break;
		case 1:
			multFactor = 1.0/65.5;
			break;
		case 2:
			multFactor = 1.0/32.8;
			break;
		case 3:
			multFactor = 1.0/16.4;
			break;
	}
	angVel[0] = data[0]*multFactor;
	angVel[1] = data[1]*multFactor;
	angVel[2] = data[2]*multFactor;
}
