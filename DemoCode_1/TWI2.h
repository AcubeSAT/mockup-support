#ifndef TWI2_H_
#define TWI2_H_

#include <stm32f10x.h>

#define I2C1_INIT 1
#define I2C2_INIT 2
#define I2C_CHOICE I2C2

extern void TWIInit2(uint8_t i2cNum); //Initialize the I2C interface
extern void TWIStart2(void); //Send a start signal
extern void TWIStop2(void); //Send a stop signal
extern void TWIWrite2(uint8_t u8data); //Send 8 bits of data
extern uint8_t TWIReadACK2(void); //Check if you received the acknowledgment from the other device
extern uint8_t TWIReadNACK2(void); //Expect no acknowledgment
extern void TWISendAddr2(uint8_t addr, uint8_t tr_dir); //Set tr_dir to one if sending a command and zero if receiving

extern uint8_t TWIReadByte2(uint8_t dev_addr, uint8_t registe); //Read one byte from the slave
extern void TWIReadBytes2(uint8_t dev_addr, uint8_t registe, uint8_t *byte_read, uint8_t byte_count); //Read bytes from slave
extern void TWIWriteByte2(uint8_t dev_addr, uint8_t command_register, uint8_t command); //Write a byte to the slave

#endif
