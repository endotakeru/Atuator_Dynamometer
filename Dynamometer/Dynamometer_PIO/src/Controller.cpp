#include "Controller.h"
#include "config.h"
#include "Calibration.h"
#include "Brake.h"

static Mode mode = MODE_IDLE; // (?)

static float rpm_target = 0.0f;
static float rpm_i = 0.0f; // I gain total
static float rpm_last = 0.0f;
static bool have_last = false;
static float shed_last = 0.0f; // rpm amount the brake shed last
static bool easing_off = false; // (?)

void controlBegin(){controlReset(); mode = MODE_IDLE;} // (?)

void controlReset(){
    rpm_i = 0.0f; rpm_last = 0.0f; have_last = false;
    shed_last = 0.0f; easing_off = false;
}

void controlSetMode(Mode m){ // (?)
    if(m != mode) controlReset();
    mode = m;
    if(mode != MODE_SPEED) brakeRelease();
}

Mode controlMode(){return mode;}

void controlSetSpeed(float rpm){rpm_target = max(rpm, 0.0f);} //make sure speed is not negative, 0 means stall

float controlSpeedSetpoint(){return rpm_target;}

static void updateSpeed(float torque, float rpm, float dt){
    if (torque >= TORQUE_LIMIT_ABS_NM){
        brakeRelease();
        rpm_i = 0.0f; shed_last = 0.0f; have_last = false;
        mode = MODE_IDLE;
        Serial.print(F("# E-STOP: torque ")); Serial.print(torque, 3); // (?)
        Serial.print(F(" >= abs limit ")); Serial.println(TORQUE_LIMIT_ABS_NM, 3);
        return;
    }

    const float err = rpm - rpm_target; //error = rpm

    rpm_i += cal.ki * err * dt; // (?)

    float slope = 0.0f;
    if (have_last) slope = (rpm - rpm_last) / dt;
    rpm_last = rpm; have_last = true;

    float rpm_shed = cal.kp * err + rpm_i + cal.kd * slope; // PID eq.

    const float span = (cal.brake_rpm_span > 0.0f) ? cal.brake_rpm_span :  BRAKE_RPM_SPAN_DEFAULT; // (?)

    const float limit = constrain(cal.torque_limit_Nm, 0.0f, TORQUE_LIMIT_ABS_NM); // (?)
    if(torque >= limit){
        rpm_shed = min(rpm_shed, shed_last) - TORQUE_RELIEF_RPM; // (?)
        rpm_i -= cal.ki * err * dt; // (?)
        rpm_i = min(rpm_i, max(rpm_shed, 0.0f)); // (?)
        if(!easing_off){ // (?)
            Serial.print(F("# torque limit ")); Serial.print(limit, 3);
            Serial.println(F(" Nm reached - easing the brake off"));
            easing_off = true;
        }
    } else {
        easing_off = false;
    }

    const float max_step = span / (float)BRAKE_RAMP_TICKS;
    if(rpm_shed > shed_last + max_step) rpm_shed = shed_last + max_step; // clamp to max_step

    rpm_shed = constrain(rpm_shed, 0.0f, span);
    rpm_i = constrain(rpm_i, 0.0f, span);

    const bool pinned_max = (rpm_shed >= span && err > 0.0f);
    const bool pinned_min = (rpm_shed <= 0.0f && err < 0.0f);
    if(pinned_max || pinned_min){ // (?)
        rpm_i -= cal.ki * err * dt;
        rpm_i = constrain(rpm_i, 0.0f, span);
    }

    shed_last = rpm_shed;

    brakeSetDemand(brakeRPMToDemand(rpm_shed)); // (?)
}

void controlUpdate(float torque, float rpm, float dt){
    if (dt <= 0.0f) return;

    switch(mode){
        case MODE_SPEED: updateSpeed(torque, rpm, dt); break; // (?)
        case MODE_IDLE: breakRelease(); rpm_i = 0.0f; shed_last = 0.0f; break; // (?)
        case MODE_MANUAL: default: break; // (?)
    }
}