#include "Calibration.h"
#include "config.h"
#include <EEPROM.h>

static constexpr uint32_t CAL_MAGIC = 0x44594E45; //"DYNE"
static constexpr int CAL_ADDR = 0; // (?) TBD

Calibration cal; // (?)

void calLoadDefaults(){
    cal.magic = CAL_MAGIC;
    cal.counts_per_Nm = 1.0f; //TBD
    cal.drag_tare_Nm = 0.0f; //TBD
    cal.kp = KP_DEFAULT; //TBD
    cal.ki = KI_DEFAULT; //TBD
    cal.kd = KD_DEFAULT; //TBD
    cal.servo_released_us = SERVO_RELEASED_US_DEFAULT;
    cal.servo_full_us = SERVO_FULL_US_DEFAULT;
    cal.torque_limit_Nm = TORQUE_LIMIT_DEFAULT_NM;
    cal.stiction_floor = BRAKE_STICTION_FLOOR_DEFAULT; // (?)
    cal.brake_rpm_span = BRAKE_RPM_SPAN_DEFAULT;
}

void calLoad(){
    Calibration tmp; // (?)
    EEPROM.get(CAL_ADDR, tmp); // (?)
    if (tmp.magic == CAL_MAGIC){
        cal = tmp; // (?)
        cal.torque_limit_Nm = constrain(cal.torque_limit_Nm, 0.0f, TORQUE_LIMIT_ABS_NM);
        if (cal.brake_rpm_span <= 0.0f) cal.brake_rpm_span = BRAKE_RPM_SPAN_DEFAULT; // (?)
        Serial.println(F("# cal loaded from EEPROM"));
    }
    else{
        calLoadDefaults(); // (?)
        Serial.println(F("# no valid EEPROM cal - using defaults"));
    }
}

void calSave(){
    cal.magic = CAL_MAGIC;
    EEPROM.put(CAL_ADDR, cal); // (?)
    #if defined(ESP32) || defined(ESP8266)
        EEPROM.commit(); // (?)
    #endif // (?)
        Serial.println(F("# cal saved"));
}

void calPrint(){
    Serial.print(F("# cfg counts_per_Nm = ")); Serial.print(cal.counts_per_Nm, 4);
    Serial.print(F(" drag_tare_Nm="));       Serial.print(cal.drag_tare_Nm, 5);
    Serial.print(F(" torque_limit_Nm="));    Serial.print(cal.torque_limit_Nm, 3);
    Serial.print(F(" Kp="));                 Serial.print(cal.kp, 4);
    Serial.print(F(" Ki="));                 Serial.print(cal.ki, 4);
    Serial.print(F(" Kd="));                 Serial.print(cal.kd, 4);
    Serial.print(F(" stiction="));           Serial.print(cal.stiction_floor, 3);
    Serial.print(F(" rpm_span="));           Serial.print(cal.brake_rpm_span, 1);
    Serial.print(F(" servo=["));             Serial.print(cal.servo_released_us);
    Serial.print(',');                       Serial.print(cal.servo_full_us);
    Serial.print(F("] slots="));             Serial.print(ENCODER_SLOTS);
    Serial.print(F(" shunt="));              Serial.print(INA_SHUNT_OHMS, 4);
    Serial.print(F(" Imax="));               Serial.println(INA_MAX_CURRENT_A, 2);
}