#pragma once
#include <Arduino.h>

struct Calibration{
    uint32_t magic;
    float counts_per_Nm; //ADC counts per Nm
    float drag_tare_Nm; //no-load reading (offset)
    float kp, ki, kd; //gains
    int servo_released_us; //digital signal pads off
    int servo_full_us; //digital signal pads fully on
    float torque_limit_Nm; //working torque limit
    float stiction_floor; //minimum demand (w/ offset)
    float brake_rpm_span; //full clamp 'speed reduction value' (?)
};

extern Calibration cal; // (?)

void calLoadDefaults();
void calLoad();
void calSave();
void calPrint();