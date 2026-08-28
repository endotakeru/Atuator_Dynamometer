#include "Tach.h"
#include "config.h"

static constexpr uint32_t MIN_EDGE_INTERVAL_US = 50; //TBD

static volatile uint32_t pulse_count = 0;
static volatile uint32_t total_pulses = 0;
static volatile uint32_t last_edge_us = 0; //last accepted pulse timestamp
static float rpm_filt = 0.0f; //filtered rpm reading

static void tachISR(){
    const uint32_t now = micros();
    if(now-last_edge_us < MIN_EDGE_INTERVAL_US) 
        return; //glitch/noise filter
    last_edge_us = now;
    pulse_count++;
    total_pulses++;
}

void tachBegin(){
    pinMode(PIN_TACH, INPUT_PULLUP); // use PULLUP as arduino has built-in PU Resistor
    attachInterrupt(digitalPinToInterrupt(PIN_TACH), tachISR, RISING); //Interrupt and run tachISR once when edge
}

void tachUpdate(float dt_s){
    noInterrupts(); //stop counting
    const uint32_t pulses = pulse_count;
    pulse_count = 0;
    interrupts(); //resume counting

    const float revs = (float)pulses / (float)ENCODER_SLOTS;
    const float rpm = (dt_s > 0.0f) ? (revs/dt_s)*60.0f : 0.0f; // If dt_s > 0 calculate rpm, else rpm = 0
    rpm_filt = RPM_ALPHA*rpm + (1.0f - RPM_ALPHA) * rpm_filt;
}

float tachRPM(){
    return rpm_filt;
}

uint32_t tachTotalPulses(){
    noInterrupts();
    const uint32_t t = total_pulses; //need for P command
    interrupts();
    return t;
}