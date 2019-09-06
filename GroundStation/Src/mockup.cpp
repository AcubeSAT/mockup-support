#include <main.h>
#include "mockup.h"

extern "C"
{

    void main_cpp() {
        HAL_GPIO_TogglePin(LD_R_GPIO_Port, LD_R_Pin);
        HAL_UART_Transmit(&huart2, (uint8_t*) "Hello\r\n", 7, 100);
        HAL_Delay(200);
    }


}
