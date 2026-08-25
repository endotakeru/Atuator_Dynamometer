#include "Tach.h"
#include "config.h"

static constexpr uint32_t MIN_EDGE_INTERVAL_US = 50; //TBD

static volatile uint32_t pulse_count = 0;
static volatile uint32_t total_pulses = 0;
static volatile uint32_t last_edge_us = 0;
static float rpm_filt = 0.0f;

static void tachISR(){
    const uint32_t now = micros();
    if(now-last_edge_us < MIN_EDGE_INTERVAL_US) return; //noise filter
    last_edge_us = now;
    pulse_count++;
    total_pulses++;
}

void tachBegin(){
    pinMode(PIN_TACH, INPUT_PULLUP); // (?)
    attachInterrupt(digitalPinToInterrupt(PIN_TACH), tachISR, RISING); // (?)
}

void tachUpdate(float dt_s){
    noInterrupts();
    const uint32_t pulses = pulse_count; // (?)
    pulse_count = 0;
    interrupts();

    const float revs = (float)pulses / (float)ENCODER_SLOTS;
    const float rpm = (dt_s > 0.0f) ? (revs/dt_s)*60.0f : 0.0f; // (?)
    rpm_filt = RPM_ALPHA*rpm + (1.0f - RPM_ALPHA) * rpm_filt;
}

float tachRPM(){
    return rpm_filt;
}

uint32_t tachTotalPulses(){
    noInterrupts(); // (?)
    const uint32_t t = total_pulses; // (?)
    interrupts(); // (?)
    return t;
}