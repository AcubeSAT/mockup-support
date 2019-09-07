#include <main.h>
#include <mockup.hpp>
#include <cstdio>
#include <queue>
#include <AX5043.h>
#include "mockup.h"
#include "UARTMessage.h"
#include "at86rf2xx.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
// A container of bytes
std::queue<uint8_t> uartQueue;

extern "C"
{
void gotDatum(uint8_t datum) {
    uartQueue.push(datum);
}

void main_cpp() {
    static uint8_t messageType;
    static uint32_t lastPingTime = HAL_GetTick();
    uartLog("Welcome\r\n");

    // Init AT86RF233
    AT86RF2XX at86Rf233(&hspi1);
    at86Rf233.init();
    at86Rf233.set_chan(26);

    AX5043 rx(&hspi1, GPIOA, GPIO_PIN_8);
    rx.enterReceiveMode();


    while (true) {
        if (HAL_GetTick() - lastPingTime > 100) {
            uint8_t data[2] = {'c','s'};
            UARTMessage ping{(char*)data, 2, UARTMessage::Ping};
            uartSend(ping);
            lastPingTime = HAL_GetTick();
        }

        // UART queue for RX data
        if (!uartQueue.empty()) {
            uint8_t messageType = uartQueue.front();
            uartQueue.pop();

            if (messageType == 0) {
                // We got a null byte. This shouldn't happen
            } else {
                static uint8_t receptionBuffer[257];
                int index = 1;

                while (index < sizeof(receptionBuffer)) {
                    // Wait until the queue gets data
                    uint32_t startTicks = HAL_GetTick();

                    while (uartQueue.empty()) {
                        if (HAL_GetTick() - startTicks > 100) {
                            // Timeoout, cancel reception
                            uartLog("Timeout\r\n");
                            break;
                        }
                    }

                    if (uartQueue.empty()) break;

                    receptionBuffer[index] = uartQueue.front();
                    uartQueue.pop();

                    // We have a datum!

                    if (receptionBuffer[index] == 0) {
                        // Null byte
                        // We have a new message (of size index), it should be handled
                        uartLog("New msg\r\n");

                        break; // Done with this operation
                    }

                    index++;
                }
            }
        }

    }
}
}

void uartLog(const char* data) {
    uint8_t size = strlen(data) ;
    char moreData[size + 2];
    memcpy(moreData, data, size);
    moreData[size] = '\r';
    moreData[size + 1] = '\n';

    UARTMessage message = {
            moreData, static_cast<uint8_t>(size + 3), UARTMessage::Log
    };
    uartSend(message);
}

void uartSend(const uint8_t* data, uint16_t size) {
    // This function does not append the message type byte, use with care
    for (int i = 0; i < size; i++) {
        while (!LL_USART_IsActiveFlag_TXE(USART2)) {}

        LL_USART_TransmitData8(USART2, data[i]);
    }
}

void uartSend(UARTMessage& message) {
    static uint8_t buffer[256];

    cobs_encode_result result = message.encode((char*)buffer, 256);

    uartSend(buffer, result.out_len);
}

#pragma clang diagnostic pop