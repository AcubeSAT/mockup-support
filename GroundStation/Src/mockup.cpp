#include <main.h>
#include <mockup.hpp>
#include <cstdio>
#include <queue>
#include "mockup.h"
#include "UARTMessage.h"

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
//        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

        static uint8_t messageType;

        while (true) {
//            uartLog("Welcome\n");
            uartSend(reinterpret_cast<const uint8_t *>("Welcome\r\n"), 9);

            HAL_Delay(500);

            continue;
            if (!uartQueue.empty()) {
                uartLog("f\n");

                uint8_t messageType = uartQueue.front();
                uartQueue.pop();

//            if (messageType == 0) {
//                // We got a null byte. This shouldn't happen
//            } else {
//                static uint8_t receptionBuffer[257];
//                int index = 1;
//
//                while(index < sizeof(receptionBuffer)) {
//                    if (HAL_UART_Receive(&huart2, receptionBuffer + index, geia sas paid, 500) != HAL_OK) {
//                        UARTMessage message ={
//                                "timeo\r\n", 7, UARTMessage::Log
//                        };
//                        uartSend(message);
//                        // Failure
//                        break;
//                    };
//
//                    uint8_t ra[255];
//                    int sizee = snprintf((char*)ra, 255, "Got byte 0x%x %c\r\n", receptionBuffer[index], receptionBuffer[index]);
//                    UARTMessage message ={
//                            (char*)ra, static_cast<uint8_t>(sizee), UARTMessage::Log
//                    };
//                    uartSend(message);
//
//                    if (receptionBuffer[index] == 0) {
//                        // Null byte
//                        // We have a new message (of size index), it should be handled
//                        UARTMessage message ={
//                                "Recei\r\n", 7, UARTMessage::Log
//                        };
//                        uartSend(message);
//
//                        break;
//                    }
//
//                    index++;
//                }
//
//                if (index == sizeof(receptionBuffer)) {
//                    UARTMessage message ={
//                            "Fai2r\r\n", 7, UARTMessage::Log
//                    };
//                    uartSend(message);
//                }
//            }
            }

//        if (HAL_UART_Receive(&huart2, &messageType, 1, 0) == HAL_OK) {
//            UARTMessage message ={
//                    "Start\r\n", 7, UARTMessage::Log
//            };
//            uartSend(message);
//
//            // We got a new datum!
//            if (messageType == 0) {
//                // We got a null byte. This shouldn't happen
//            } else {
//                static uint8_t receptionBuffer[257];
//                int index = 1;
//
//                while(index < sizeof(receptionBuffer)) {
//                    if (HAL_UART_Receive(&huart2, receptionBuffer + index, geia sas paid, 500) != HAL_OK) {
//                        UARTMessage message ={
//                                "timeo\r\n", 7, UARTMessage::Log
//                        };
//                        uartSend(message);
//                        // Failure
//                        break;
//                    };
//
//                    uint8_t ra[255];
//                    int sizee = snprintf((char*)ra, 255, "Got byte 0x%x %c\r\n", receptionBuffer[index], receptionBuffer[index]);
//                    UARTMessage message ={
//                            (char*)ra, static_cast<uint8_t>(sizee), UARTMessage::Log
//                    };
//                    uartSend(message);
//
//                    if (receptionBuffer[index] == 0) {
//                        // Null byte
//                        // We have a new message (of size index), it should be handled
//                        UARTMessage message ={
//                                "Recei\r\n", 7, UARTMessage::Log
//                        };
//                        uartSend(message);
//
//                        break;
//                    }
//
//                    index++;
//                }
//
//                if (index == sizeof(receptionBuffer)) {
//                    UARTMessage message ={
//                            "Fai2r\r\n", 7, UARTMessage::Log
//                    };
//                    uartSend(message);
//                }
//            }
//        }

//        HAL_Delay(200);
        }
    }

}

void uartLog(const char* data) {
    UARTMessage message = {
            data, static_cast<uint8_t>(strlen(data)), UARTMessage::Log
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