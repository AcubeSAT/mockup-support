#ifndef STM32_COMPONENT_DRIVERS_SPIDEVICE_H
#define STM32_COMPONENT_DRIVERS_SPIDEVICE_H

#include "stm32l4xx_hal.h"

class SPIDevice {
private:
    /**
     * The GPIO of the NSS (Negative Slave Select) pin
     */
    GPIO_TypeDef* nssGPIO;

    /**
     * The GPIO mask of the NSS (Negative Slave Select) pin. One of the HAL provided values.
     */
    uint16_t nssPin;
protected:
    static constexpr uint8_t WriteMask = 0x80;

    /**
     * SPI handle provided by HAL
     */
    SPI_HandleTypeDef *spi;

    /**
     * Set the NSS pin to LOW in order to select this device
     */
    void slaveSelect() {
        HAL_GPIO_WritePin(nssGPIO, nssPin, GPIO_PIN_RESET);
    }

    /**
     * Set the NSS pin to HIGH in order to deselect this device
     */
    void slaveUnselect() {
        HAL_GPIO_WritePin(nssGPIO, nssPin, GPIO_PIN_SET);
    }

    /**
     * Write a single value of data to a single register
     * @param address The address of the register
     * @param data The data to write to the register
     */
    void writeReg(uint8_t address, uint8_t data);

    /**
     * Read a single value of data from a single register
     * @param address The address of the register
     * @param data The data to write to the register
     */
    uint8_t readReg(uint8_t address);

    /**
     * A small buffer to store temporary SPI data
     */
    uint8_t txBuffer[4] = { 0 };

    /**
     * A small buffer to store temporary SPI data
     */
    uint8_t rxBuffer[4] = { 0 };
public:
    /**
     * Initialize a new controller object for this device
     * @param spi HAL handle to the appropriately-set and enabled SPI peripheral
     */
    SPIDevice(SPI_HandleTypeDef *spi, GPIO_TypeDef* nssGPIO, uint16_t nssPin);
};

#endif //STM32_COMPONENT_DRIVERS_SPIDEVICE_H
