/*
 * Copyright (C) 2013 Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_at86rf2xx
 * @{
 *
 * @file
 * @brief       Implementation of driver internal functions
 *
 * @author      Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Mark Solters <msolters@gmail.com>
 *
 * @}
 */

#include "at86rf2xx.hpp"

void AT86RF2XX::reg_write(const uint8_t addr, const uint8_t value) {
  uint8_t writeCommand = addr | AT86RF2XX_ACCESS_REG | AT86RF2XX_ACCESS_WRITE;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  // digitalWrite(cs_pin, LOW);
  HAL_SPI_Transmit(spi, &writeCommand, 1, 100);
  // SPI.transfer(writeCommand);
  HAL_SPI_Transmit(spi, (uint8_t *)&value, 1, 100);
  // SPI.transfer(value);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  // digitalWrite(cs_pin, HIGH);
}

uint8_t AT86RF2XX::reg_read(const uint8_t addr) {
  uint8_t value;
  uint8_t readCommand = addr | AT86RF2XX_ACCESS_REG | AT86RF2XX_ACCESS_READ;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  // digitalWrite(cs_pin, LOW);

  HAL_SPI_TransmitReceive(spi, &readCommand, &value, 1, 100);
  while (HAL_SPI_GetState(spi) != HAL_SPI_STATE_READY)
    ;
  // SPI.transfer(readCommand);

  HAL_SPI_TransmitReceive(spi, &readCommand, &value, 1, 100);
  // value = SPI.transfer(0x00);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  // digitalWrite(cs_pin, HIGH);

  return (uint8_t)value;
}

void AT86RF2XX::sram_read(const uint8_t offset, uint8_t *data,
                          const uint16_t len) {
  uint8_t readCommand = AT86RF2XX_ACCESS_SRAM | AT86RF2XX_ACCESS_READ;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_SPI_Transmit(spi, &readCommand, 1, 100);
  // SPI.transfer(readCommand);
  HAL_SPI_Transmit(spi, (uint8_t *)&offset, 1, 100);
  // SPI.transfer((char)offset);
  uint8_t zero = 0x00;
  for (int b = 0; b < len; b++) {
    uint8_t currByte = 0;
    HAL_SPI_TransmitReceive(spi, &zero, &currByte, 1, 100);
    data[b] = currByte;
  }
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

void AT86RF2XX::sram_write(const uint8_t offset, const uint8_t *data,
                           const uint16_t len) {
  uint8_t writeCommand = AT86RF2XX_ACCESS_SRAM | AT86RF2XX_ACCESS_WRITE;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_SPI_Transmit(spi, &writeCommand, 1, 100);
  // SPI.transfer(writeCommand);
  HAL_SPI_Transmit(spi, (uint8_t *)&offset, 1, 100);
  // SPI.transfer((char)offset);
  for (int b = 0; b < len; b++) {
    uint8_t currData = data[b];
    HAL_SPI_Transmit(spi, &currData, 1, 100);
    // SPI.transfer(data[b]);
  }
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

void AT86RF2XX::fb_read(uint8_t *data, const uint16_t len) {
  uint8_t readCommand = AT86RF2XX_ACCESS_FB | AT86RF2XX_ACCESS_READ;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_SPI_Transmit(spi, &readCommand, 1, 100);
  // SPI.transfer(readCommand);
  for (int b = 0; b < len; b++) {
    uint8_t currByte = 0;
    uint8_t zero = 0x00;
    HAL_SPI_TransmitReceive(spi, &zero, &currByte, 1, 100);
    data[b] = currByte;
  }
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

uint8_t AT86RF2XX::get_status() {
  /* if sleeping immediately return state */
  if (state == AT86RF2XX_STATE_SLEEP)
    return state;

  return reg_read(AT86RF2XX_REG__TRX_STATUS) &
         AT86RF2XX_TRX_STATUS_MASK__TRX_STATUS;
}

void AT86RF2XX::assert_awake() {
  if (get_status() == AT86RF2XX_STATE_SLEEP) {
    /* wake up and wait for transition to TRX_OFF */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    // digitalWrite(sleep_pin, LOW);
    HAL_Delay(AT86RF2XX_WAKEUP_DELAY);
    // delayMicroseconds(AT86RF2XX_WAKEUP_DELAY);

    /* update state */
    state = reg_read(AT86RF2XX_REG__TRX_STATUS) &
            AT86RF2XX_TRX_STATUS_MASK__TRX_STATUS;
  }
}

void AT86RF2XX::hardware_reset() {
  /* wake up from sleep in case radio is sleeping */
  // delayMicroseconds(50); // Arduino seems to hang without some minimum pause
  // here
  assert_awake();

  /* trigger hardware reset */

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
  // digitalWrite(reset_pin, LOW);
  HAL_Delay(AT86RF2XX_RESET_PULSE_WIDTH);
  // delayMicroseconds(AT86RF2XX_RESET_PULSE_WIDTH);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
  // digitalWrite(reset_pin, HIGH);
  HAL_Delay(AT86RF2XX_RESET_DELAY);
  // delayMicroseconds(AT86RF2XX_RESET_DELAY);
}

void AT86RF2XX::force_trx_off() {
  reg_write(AT86RF2XX_REG__TRX_STATE, AT86RF2XX_TRX_STATE__FORCE_TRX_OFF);
  while (get_status() != AT86RF2XX_STATE_TRX_OFF)
    ;
}
