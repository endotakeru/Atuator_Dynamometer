#include "Torque.h"
#include "config.h"
#include "Calibration.h"
#include <HX711.h>

static HX711 scale;
static float filt = 0.0f; //TBD

void torqueBegin(){
    scale.begin(PIN_HX711_DT, PIN_HX711_SCK);
    scale.set_gain(HX711_GAIN);
    delay(300); //settle time
    if(scale.is_ready()) scale.tare(10); // (?)
}

bool torqueReady(){return scale.is_ready();}

void torqueUpdate(){
    if(!scale.is_ready()) return;

    const long raw = scale.read(); // (?)
    const float t = (float)(raw - scale.get_offset()) / cal.counts_per_Nm; //raw -> Torque (Nm)
    filt = TORQUE_ALPHA * t + (1.0f - TORQUE_ALPHA) * filt; //low-pass (filtered output (Nm))
}

float torqueRawFiltered(){return filt;}

float torqueRead(){return filt - cal.drag_tare_Nm;} //final output (Nm)

void torqueTare(uint8_t n){// (?)
    scale.tare(n);
    filt = 0.0f;
}