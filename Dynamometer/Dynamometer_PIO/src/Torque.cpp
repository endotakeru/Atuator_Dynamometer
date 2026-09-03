#include "Torque.h"
#include "config.h"
#include "Calibration.h"
#include <HX711.h>

static HX711 scale;
static float filt = 0.0f; //filtered torque value

void torqueBegin(){
    scale.begin(PIN_HX711_DT, PIN_HX711_SCK);
    scale.set_gain(HX711_GAIN);
    // Wait for the module rather than hoping 300 ms is enough. A skipped tare
    // leaves the ADC offset at zero and makes every torque reading wrong, so
    // if it never answers we say so instead of failing silently.
    const uint32_t t0 = millis();
    while(!scale.is_ready() && millis() - t0 < 1000) delay(10);
    if(scale.is_ready()){
        scale.tare(10);
    } else {
        Serial.println(F("# WARN: HX711 not responding - torque readings are meaningless"));
    }
}

bool torqueReady(){return scale.is_ready();}

void torqueUpdate(){
    if(!scale.is_ready()) return;

    const long raw = scale.read(); //raw 24-bit reading
    // Never divide by zero here: the NaN it produces would fail every torque
    // comparison in the controller and silently disable both safeguards.
    const float scale_counts = (cal.counts_per_Nm != 0.0f) ? cal.counts_per_Nm : 1.0f;
    const float t = (float)(raw - scale.get_offset()) / scale_counts; //raw -> Torque (Nm)
    filt = TORQUE_ALPHA * t + (1.0f - TORQUE_ALPHA) * filt; //low-pass (filtered output (Nm))
}

float torqueRawFiltered(){return filt;}

float torqueRead(){return filt - cal.drag_tare_Nm;} //final output (Nm)

void torqueTare(uint8_t n){
    scale.tare(n);
    filt = 0.0f;
}