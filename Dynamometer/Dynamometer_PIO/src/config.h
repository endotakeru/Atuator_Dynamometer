#pragma once //only define config once
#include <Arduino.h>

//MCU Pins:
constexpr uint8_t PIN_TACH = 2;
constexpr uint8_t PIN_HX711_DT = 4;
constexpr uint8_t PIN_HX711_SCK = 5;
constexpr uint8_t PIN_SERVO1 = 9;
constexpr uint8_t PIN_SERVO2 = 10;
constexpr uint8_t LED_PIN = LED_BUILTIN; //Arduino's Built-In LED Pin (13u)

//Tachometer Slot Count:
constexpr uint16_t ENCODER_SLOTS = 38;

//Control Frequency and Serial Baud
constexpr uint16_t CONTROL_DT_MS = 50; //20 Hz control
constexpr uint32_t SERIAL_BAUD = 115200;

//Torque Limits:
constexpr float TORQUE_LIMIT_DEFAULT_NM = 5.0f;
constexpr float TORQUE_LIMIT_ABS_NM = 15.0f;

//Brake/Servo---------------------------------
//Digital signals limits
constexpr int SERVO_RELEASED_US_DEFAULT = 1000;  // digital signal at BD = 0, TBD
constexpr int SERVO_FULL_US_DEFAULT     = 2000;  // digital signal at BD = 1, TBD
constexpr int SERVO_US_ABS_MIN          = 500;   // hard safety rails
constexpr int SERVO_US_ABS_MAX          = 2500;

//Minimum demand that moves the pads considering noise
constexpr float BRAKE_STICTION_FLOOR_DEFAULT = 0.05f; //TBD

constexpr float BRAKE_RPM_SPAN_DEFAULT = 1000.0f; //TBD

//Control Gains: Error = RPM, Output = RPM
constexpr float KP_DEFAULT = 1.0f; // TBD
constexpr float KI_DEFAULT = 1.0f; // TBD
constexpr float KD_DEFAULT = 0.0f;  // TBD

// How much of the braking request is dropped each tick once the torque limit
// is reached. NOTE the unwind time scales with brake_rpm_span, so it is not the
// same on every motor: ticks to unwind = span / 25, i.e. 2.0 s at a span of
// 1000 but only 0.4 s at a span of 200. Erring fast is the safe direction for a
// protection feature, but lower this if you see the brake lurching.
constexpr float TORQUE_RELIEF_RPM = 25.0f; // rpm removed from the request per tick

constexpr uint16_t BRAKE_RAMP_TICKS = 40; // ticks to reach full servo travel

//EMA Filtering Constants (only for HX711)
constexpr float RPM_ALPHA = 0.4f;
constexpr float TORQUE_ALPHA = 0.3f;

//INA226
constexpr uint8_t INA226_ADDR = 0x40; //INA226 address for using A0,A1 -> GND
constexpr float INA_SHUNT_OHMS = 0.015f; //TBD
constexpr float INA_MAX_CURRENT_A = 5.0f; //TBD

//Averaging sample size index
constexpr uint8_t INA_AVG_INDEX = 2; //index 2 = 16 samples
constexpr uint8_t INA_CT_INDEX = 4; //index 4 = 1.1ms conversion time

constexpr uint8_t HX711_GAIN = 128; // amplifies strain gauge readings by 128 times
