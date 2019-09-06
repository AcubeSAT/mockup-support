#ifndef GROUNDSTATION_MOCKUP_HPP
#define GROUNDSTATION_MOCKUP_HPP

#include "UARTMessage.h"

void uartLog(const char* data);
void uartSend(const uint8_t* data, uint16_t size);
void uartSend(UARTMessage& message);

#endif //GROUNDSTATION_MOCKUP_HPP
