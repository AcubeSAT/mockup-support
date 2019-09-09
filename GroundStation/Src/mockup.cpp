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

AT86RF2XX at86Rf233(&hspi1);

void at86_eventHandler();


void main_cpp() {
    static uint8_t messageType;
    static uint32_t lastPingTime = HAL_GetTick();
    uartLog("Welcome\r\n");

    // Init AT86RF233
    at86Rf233.init();
    at86Rf233.set_chan(26);

//    AX5043 rx(&hspi1, GPIOA, GPIO_PIN_8);
//    rx.enterTransmitMode();


    while (true) {
        if (HAL_GetTick() - lastPingTime > 100) {
            uint8_t data[2] = {'c','s'};
            UARTMessage ping{(char*)data, 2, UARTMessage::Ping};
            uartSend(ping);
            lastPingTime = HAL_GetTick();
        }

        at86_eventHandler();

        // UART queue for RX data
        if (!uartQueue.empty()) {
            uint8_t messageType = uartQueue.front();
            uartQueue.pop();

            if (messageType == 0) {
                // We got a null byte. This shouldn't happen
            } else {
                static uint8_t receptionBuffer[257];
                int index = 1;
                receptionBuffer[0] = messageType;

                while (index < sizeof(receptionBuffer)) {
                    // Wait until the queue gets data
                    uint32_t startTicks = HAL_GetTick();

                    while (uartQueue.empty()) {
                        if (HAL_GetTick() - startTicks > 100) {
                            // Timeout, cancel reception
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

                        uint8_t decodedBuffer[256];
                        // Decode the data via COBS
                        auto result = cobs_decode(decodedBuffer, 256, receptionBuffer, index);
                        if (decodedBuffer[0] == static_cast<uint8_t >(UARTMessage::SpacePacket)) {
                            uartLog("Space\r\n");

                            // Now, we can transmit the packet via AT86RF233
                            at86Rf233.send(decodedBuffer + 1, result.out_len - 1);
                        } else {
                            char t[255];
                            snprintf(t, 255, "other %d\r\n", index);
                            uartLog(t);
                        }

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

void at86_receive_data() {
    // Get the packet length (first item in FIFO)
    uint16_t pkt_len = at86Rf233.rx_len();

    uint8_t data[pkt_len];
    at86Rf233.rx_read(data, pkt_len, 0);

//    uartLog("Got some datas");

    if (pkt_len > 255) {
        uartLog("Too long pkt");
    } else {
        UARTMessage message = {
                reinterpret_cast<char*>(data), static_cast<uint8_t>(pkt_len), UARTMessage::SpacePacket
        };
        uartSend(message);
    }
}


void at86_eventHandler() {
    /* If transceiver is sleeping register access is impossible and frames are
     * lost anyway, so return immediately.
     */
    uint8_t state = at86Rf233.get_status();
    if (state == AT86RF2XX_STATE_SLEEP)
        return;

    /* read (consume) device status */
    uint8_t irq_mask = at86Rf233.reg_read(AT86RF2XX_REG__IRQ_STATUS);

    /*  Incoming radio frame! */
    if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__RX_START) {
        // Evt event start
    }

    /*  Done receiving radio frame; call our receive_data function.
     */
    if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__TRX_END) {
        if (state == AT86RF2XX_STATE_RX_AACK_ON ||
            state == AT86RF2XX_STATE_BUSY_RX_AACK) {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);
            at86_receive_data();
        }
    }
}

#pragma clang diagnostic pop