#include <main.h>
#include <mockup.hpp>
#include "mockup.h"
#include "UARTMessage.h"

extern "C"
{

    void main_cpp() {
        HAL_GPIO_TogglePin(LD_R_GPIO_Port, LD_R_Pin);
        uartSend((uint8_t*)"Hello\r\n", 7);

        UARTMessage message ={
                "Video\r\n", 7, UARTMessage::Log
        };
        uartSend(message);

        HAL_Delay(200);
    }


}


void uartSend(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&huart2, data, 7, 100);
}

void uartSend(UARTMessage& message) {
    static uint8_t buffer[256];

    cobs_encode_result result = message.encode((char*)buffer, 256);

    HAL_UART_Transmit(&huart2, buffer, result.out_len, 100);
}