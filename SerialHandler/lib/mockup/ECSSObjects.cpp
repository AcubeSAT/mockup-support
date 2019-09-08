#include <ecss-services/inc/Services/ParameterService.hpp>
#include <ecss-services/inc/ServicePool.hpp>
#include "ECSSObjects.h"
#include <map>

std::map<uint32_t, std::string> parIdToString;

Parameter<uint8_t> redBrightness = Parameter<uint8_t>(0, 0, 3);
Parameter<uint8_t> greenBrightness = Parameter<uint8_t>(0, 0, 3);
Parameter<uint8_t> blueBrightness = Parameter<uint8_t>(0, 0, 255);
Parameter<uint8_t> parameterColourMaster = Parameter<uint8_t>(0, 0, 128);
Parameter<uint32_t> colourPeriod = Parameter<uint32_t>(0, 0, 500);
Parameter<float> colourPhase = Parameter<float>(0, 0, 0.03);

Parameter<float> angleX = Parameter<float>(0,0,0);
Parameter<float> angleY = Parameter<float>(0,0,0);
Parameter<float> angleZ = Parameter<float>(0,0,0);
Parameter<float> gyroX = Parameter<float>(0,0,0);
Parameter<float> gyroY = Parameter<float>(0,0,0);
Parameter<float> gyroZ = Parameter<float>(0,0,0);
Parameter<float> accelX = Parameter<float>(0,0,0);
Parameter<float> accelY = Parameter<float>(0,0,0);
Parameter<float> accelZ = Parameter<float>(0,0,0);

Parameter<float> brightness = Parameter<float>(0,0,0);
Parameter<float> tempInternal = Parameter<float>(0,0,-100);
Parameter<float> tempExternal = Parameter<float>(0,0,-100);

/**
 * Null function for reference
 */
void n(String<ECSS_FUNCTION_MAX_ARG_LENGTH> n) {
}

void addECSSObjects() {
    ParameterService & parameters = Services.parameterManagement;

    parameters.addNewParameter(1, redBrightness);
//    parIdToString[1] = "redBrightness";
    parameters.addNewParameter(2, greenBrightness);
//    parIdToString[2] = "greenBrightness";
    parameters.addNewParameter(3, blueBrightness);
//    parIdToString[3] = "blueBrightness";
    parameters.addNewParameter(4, parameterColourMaster);
//    parIdToString[4] = "totalBrightness";
    parameters.addNewParameter(5, colourPeriod);
//    parIdToString[5] = "Colour Duration";
    parameters.addNewParameter(6, colourPhase);
//    parIdToString[6] = "Colour Phase";

    parameters.addNewParameter(10, angleX);
    parIdToString[10] = "Angle X";
    parameters.addNewParameter(11, angleY);
    parIdToString[11] = "Angle Y";
    parameters.addNewParameter(12, angleZ);
    parIdToString[12] = "Angle Z";

    parameters.addNewParameter(13, gyroX);
    parIdToString[13] = "Gyroscope X";
    parameters.addNewParameter(14, gyroY);
    parIdToString[14] = "Gyroscope Y";
    parameters.addNewParameter(15, gyroZ);
    parIdToString[15] = "Gyroscope Z";
    parameters.addNewParameter(16, accelX);
//    parIdToString[16] = "Acceleration X";
    parameters.addNewParameter(17, accelY);
//    parIdToString[17] = "Acceleration Y";
    parameters.addNewParameter(18, accelZ);
//    parIdToString[18] = "Acceleration Z";

    parameters.addNewParameter(20, brightness);
    parIdToString[20] = "Brightness";
    parameters.addNewParameter(21, tempInternal);
    parIdToString[21] = "Temperature Internal";
    parameters.addNewParameter(22, tempExternal);
    parIdToString[22] = "Temperature External";

    FunctionManagementService & functions = Services.functionManagement;
    functions.include("led_toggle", n);
    functions.include("led_strip", n);
    functions.include("led_strip_m", n);
    functions.include("led_strip_c", n);
    functions.include("led_strip_g", n);
    functions.include("led_AUTOfade_on", n);
    functions.include("led_AUTOfade_off", n);
    functions.include("flash_white", n);
    functions.include("flash_fluorescent", n);
    functions.include("calib_euler", n);
    functions.include("calib_gyro", n);

    HousekeepingService & housekeeping = Services.housekeeping;
    housekeeping.addHousekeepingStructure(1, {250, {10, 11, 12}});
    housekeeping.addHousekeepingStructure(2, {60, {20, 21, 22}});
    housekeeping.addHousekeepingStructure(3, {60, {13, 14, 15}});
    housekeeping.addHousekeepingStructure(4, {50, {10, 11, 12, 20, 21, 22, 13, 14, 15}});
}

