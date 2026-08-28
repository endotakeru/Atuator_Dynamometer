#pragma once
#include <Arduino.h>

struct Calibration{
    uint32_t magic;
    float counts_per_Nm; //ADC counts per Nm
    float drag_tare_Nm; //no-load reading (offset)
    float kp, ki, kd; //PID gains
    int servo_released_us; //live digital signal at BD = 0, TBD
    int servo_full_us; //live digital signal at BD = 1, TBD
    float torque_limit_Nm; //working torque limit = 5 Nm
    float stiction_floor; //minimum servo demand to start move brake (w/ offset)
    float brake_rpm_span;
};

extern Calibration cal; // (?)

void calLoadDefaults();
void calLoad();
void calSave();
void calPrint();