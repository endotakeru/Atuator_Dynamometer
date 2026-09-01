#include "Brake.h"
#include "config.h"
#include "Calibration.h"
#include <Servo.h>

static Servo servo1, servo2;
static int last_us = SERVO_RELEASED_US_DEFAULT;
static float last_demand = 0.0f;

static int clampUs(int us){
    const int lo = min(cal.servo_released_us, cal.servo_full_us);
    const int hi = max(cal.servo_released_us, cal.servo_full_us);
    us = constrain(us, lo, hi);
    return constrain(us, SERVO_US_ABS_MIN, SERVO_US_ABS_MAX);
}

float brakeRpmToDemand(float rpm_shed){
    if (rpm_shed <= 0.0f) return 0.0f;

    const float span = (cal.brake_rpm_span > 0.0f) ? cal.brake_rpm_span : BRAKE_RPM_SPAN_DEFAULT;
    const float frac = constrain(rpm_shed / span, 0.0f, 1.0f);
    return cal.stiction_floor + frac * (1.0f - cal.stiction_floor); //stiction_floor makes sure caliper dead-zones are avoided
}

void brakeBegin(){
    servo1.attach(PIN_SERVO1); //set mcu pins
    servo2.attach(PIN_SERVO2);
    brakeRelease();
}

void brakeSetDemand(float demand01){
    demand01 = constrain(demand01, 0.0f, 1.0f);
    last_demand = demand01;
    const int span = cal.servo_full_us - cal.servo_released_us;
    brakeSetMicroseconds(cal.servo_released_us + (int)(demand01 * span));
    // cal.servo_released_us + (int)(demand01 * span) = demand01 servo_us
}

void brakeSetMicroseconds(int us){
    last_us = clampUs(us);
    servo1.writeMicroseconds(last_us);
    servo2.writeMicroseconds(last_us);
}

void brakeRelease(){
    //set both last_us and demand to 0
    last_demand = 0.0f;
    last_us = clampUs(cal.servo_released_us);
    servo1.writeMicroseconds(last_us);
    servo2.writeMicroseconds(last_us);
}

int brakeMicroseconds(){return last_us;}
float brakeDemand(){return last_demand;}