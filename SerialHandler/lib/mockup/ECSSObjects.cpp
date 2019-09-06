#include <ecss-services/inc/Services/ParameterService.hpp>
#include <ecss-services/inc/ServicePool.hpp>
#include "ECSSObjects.h"


Parameter<uint8_t> redBrightness = Parameter<uint8_t>(0, 0, 3);
Parameter<uint8_t> greenBrightness = Parameter<uint8_t>(0, 0, 3);
Parameter<uint8_t> blueBrightness = Parameter<uint8_t>(0, 0, 255);
Parameter<uint8_t> parameterColourMaster = Parameter<uint8_t>(0, 0, 128);
Parameter<uint32_t> colourPeriod = Parameter<uint32_t>(0, 0, 500);
Parameter<float> colourPhase = Parameter<float>(0, 0, 0.03);

Parameter<double> angleX = Parameter<double>(0,0,0);
Parameter<double> angleY = Parameter<double>(0,0,0);
Parameter<double> angleZ = Parameter<double>(0,0,0);

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
    parameters.addNewParameter(2, greenBrightness);
    parameters.addNewParameter(3, blueBrightness);
    parameters.addNewParameter(4, parameterColourMaster);
    parameters.addNewParameter(5, colourPeriod);
    parameters.addNewParameter(6, colourPhase);

    parameters.addNewParameter(10, angleX);
    parameters.addNewParameter(11, angleY);
    parameters.addNewParameter(12, angleZ);

    parameters.addNewParameter(20, brightness);
    parameters.addNewParameter(21, tempInternal);
    parameters.addNewParameter(22, tempExternal);

    FunctionManagementService & functions = Services.functionManagement;
    functions.include("led_toggle", n);
    functions.include("led_strip", n);
    functions.include("led_strip_m", n);
    functions.include("led_strip_c", n);
    functions.include("led_strip_g", n);
    functions.include("calib_euler", n);
    functions.include("calib_gyro", n);

    HousekeepingService & housekeeping = Services.housekeeping;
    housekeeping.addHousekeepingStructure(1, {250, {10, 11, 12}});
    housekeeping.addHousekeepingStructure(2, {60, {20, 21, 22}});
}

