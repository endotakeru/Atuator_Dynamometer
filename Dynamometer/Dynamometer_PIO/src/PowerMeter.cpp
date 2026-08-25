#include "PowerMeter.h"
#include "config.h"
#include <Wire.h> // (?)
#include <INA226.h>

static INA226 ina(INA226_ADDR); // (?)
static float v = 0.0f, i = 0.0f;
static bool present = false;

bool powerBegin(){
    Wire.begin();
    present = ina.begin();
    if(!present){
        Serial.println(F("# WARN: INA226 not found on I2c - check SDA/SCL and address"));
        return false;
    }
    ina.setMaxCurrentShunt(INA_MAX_CURRENT_A, INA_SHUNT_OHMS);

    ina.setAverage(INA_AVG_INDEX);
    ina.setBusVoltageConversionTime(INA_CT_INDEX);
    ina.setShuntVoltageConversionTime(INA_CT_INDEX);
    return true; //INA226 present
}

void powerUpdate(){
    if (!present) return;
    v = ina.getBusVoltage();
    i = ina.getCurrent();
}

//return v,i,power
float powerVoltage(){return v;}
float powerCurrent(){return i;}
float powerElectricalW(){return v*i;}