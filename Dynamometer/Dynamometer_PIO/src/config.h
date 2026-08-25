#pragma once //only define config once
#include <Arduino.h>

//MCU Pins:
constexpr uint8_t PIN_TACH = 2;
constexpr uint8_t PIN_HX711_DT = 4; // (?)
constexpr uint8_t PIN_HX711_SCK = 5; // (?)
constexpr uint8_t PIN_SERVO1 = 9;
constexpr uint8_t PIN_SERVO2 = 10;
constexpr uint8_t LED_PIN = LED_BUILTIN; // (?)

//Tachometer Slot Count:
constexpr uint16_t ENCODER_SLOTS = 30; //TBD

//Control Frequency and Serial Baud
constexpr uint8_t CONTROL_DT_MS = 50; //20 Hz control (?)
constexpr uint8_t SERIAL_BAUD = 115200;

//Torque Limits:
constexpr float TORQUE_LIMIT_DEFAULT_NM = 5.0f;
constexpr float TORQUE_LIMIT_ABS_NM = 15.0f;

//Brake/Servo digital signals limits
constexpr int SERVO_RELEASED_US_DEFAULT = 1000;  // brake demand 0.0 (pads off) TBD
constexpr int SERVO_FULL_US_DEFAULT     = 2000;  // brake demand 1.0 (firm clamp) TBD
constexpr int SERVO_US_ABS_MIN          = 800;   // hard safety rails (from Servo Data Sheet) TBD
constexpr int SERVO_US_ABS_MAX          = 2200;

//Minimum demand that moves the pads considering noise
constexpr float BRAKE_STICTION_FLOOR_DEFAULT = 0.05f;

//Control Gains: Error = RPM, Output = RPM
constexpr float KP_DEFAULT = 1.0f;
constexpr float KI_DEFAULT = 1.0f;
constexpr float KD_DEFAULT = 0.0f;  //Usually 0 is good because tach is noisy (?)

// (?)
constexpr float TORQUE_RELIEF_RPM = 25.0f;

// (?)
constexpr uint16_t BRAKE_RAMP_TICKS = 40;

//Filtering Constants
constexpr float RPM_ALPHA = 0.4f; //EMA (?)
constexpr float TORQUE_ALPHA = 0.3f; // (?)

//INA226
constexpr uint8_t INA226_ADDR = 0x40; //A0,A1 -> GND (?)
constexpr float INA_SHUNT_OHMS = 0.015f; //TBD
constexpr float INA_MAX_CURRENT_A = 5.0f; //TBD

//Averaging sample size index
constexpr uint8_t INA_AVG_INDEX = 2; //index 2 = 16 samples -> ~35 ms < CONTROL_DT_MS
constexpr uint8_t INA_CT_INDEX = 4; //index 4 = 1.1ms conversion (?)

//HX711
constexpr uint8_t HEX711_GAIN = 128; // (?)