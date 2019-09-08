#ifndef MOCKUP_4_ECSSOBJECTS_H
#define MOCKUP_4_ECSSOBJECTS_H

#include <map>

extern std::map<uint32_t, std::string> parIdToString;

// LED strip parameters
extern Parameter<uint8_t> redBrightness;
extern Parameter<uint8_t> greenBrightness;
extern Parameter<uint8_t> blueBrightness;
extern Parameter<uint8_t> parameterColourMaster;
extern Parameter<uint32_t> colourPeriod;
extern Parameter<float> colourPhase;

// MPU9250 parameters
extern Parameter<float> angleX;
extern Parameter<float> angleY;
extern Parameter<float> angleZ;

// Various sensors
extern Parameter<float> brightness;
extern Parameter<float> tempInternal;
extern Parameter<float> tempExternal;

/**
 * Adds all the parameters to the parameter service
 */
void addECSSObjects();

// Functions
void led_toggle(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Toggle the LED itself
void led_strip(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Switch the LED strip to the defined colours
void led_strip_m(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Set LED strip to manual mode
void led_strip_c(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Set LED strip to color mode
void led_strip_g(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Set LED strip to gradual color mode
void calib_euler(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Calibrate gyroscope Euler angles
void calib_gyro(String<ECSS_FUNCTION_MAX_ARG_LENGTH> a); ///< Calibrate gyroscope and accelerometer values

#endif //MOCKUP_4_ECSSOBJECTS_H
