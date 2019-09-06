#include "SPIDevice.h"

SPIDevice::SPIDevice(SPI_HandleTypeDef *spi, GPIO_TypeDef* nssGPIO, uint16_t nssPin) : spi(spi), nssGPIO(nssGPIO), nssPin(nssPin) {}

void SPIDevice::writeReg(uint8_t address, uint8_t data) {
    txBuffer[0] = address | WriteMask;
    txBuffer[1] = data;

    slaveSelect();
    HAL_SPI_Transmit(spi, txBuffer, 2, HAL_MAX_DELAY);
    slaveUnselect();
}

uint8_t SPIDevice::readReg(uint8_t address) {
    txBuffer[0] = address;
    txBuffer[1] = 0;

    slaveSelect();
    HAL_SPI_TransmitReceive(spi, txBuffer, rxBuffer, 2, HAL_MAX_DELAY);
    slaveUnselect();

    return rxBuffer[1]; // Data will be received on the second byte
}

