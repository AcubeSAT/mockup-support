#ifndef _MPU9250_LIB_H_
#define _MPU9250_LIB_H_

#include <stm32f10x.h>
#include "TWI2.h"
#include "delay.h"
#include "MPU9250_Definitions.h"

#define AD0 1 //Select between two address possible

#if AD0
#define MPU9250_ADDR 0x68
#else
#define MPU9250_ADDR 0x69
#endif

/*******************************************************************************************************
 * Make the necessary initializations for the device and the I2C interface for the MCU.
 *******************************************************************************************************/
extern void MPU9250Init(void);

/*******************************************************************************************************
 * It is a good practice to call this function after the initialization, before any data reading happens
 * to ensure calibrated data is read. If the device is installed in a permanent location, then the
 * calibration parameters only need to be calculated and saved to the appropriate register once.
 *******************************************************************************************************/
extern void MPU9250Calibration(void);

/*******************************************************************************************************
 * Check the condition of the module, according to the factory set values.
 *******************************************************************************************************/
extern void MPU9250SelfTest(void);

/*******************************************************************************************************
 * Read the accelerometer data from the register and save them in the array provided as a pointer
 * The first element is the value for the x-axis, the second element is the y-axis value and the third
 * element of the array is the value of acceleration for the z-axis.
 *******************************************************************************************************/
extern void MPU9250ReadAccelDataRaw(int16_t *acceleration);

/*******************************************************************************************************
 * Read the gysroscope data from the register and save them in the array provided as a pointer
 * The first element is the value for the x-axis, the second element is the y-axis value and the third
 * element of the array is the value of angular velocity for the z-axis.
 *******************************************************************************************************/
extern void MPU9250ReadGyroDataRaw(int16_t *angular);

/*******************************************************************************************************
 * Read the temperature from the integrated thermometer and return it.
 *******************************************************************************************************/
extern int16_t MPU9250ReadTempDataRaw(void);


#endif //_MPU9250_LIB_H_
