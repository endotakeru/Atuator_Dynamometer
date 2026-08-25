#pragma once
#include <Arduino.h>

void tachBegin();
void tachUpdate(float dt_s);
float tachRPM();
uint32_t tachTotalPulses();