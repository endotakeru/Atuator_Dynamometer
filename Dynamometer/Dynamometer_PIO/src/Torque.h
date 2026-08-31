#pragma once
#include <Arduino.h>

void torqueBegin();
void torqueUpdate();
float torqueRead();
float torqueRawFiltered();
void torqueTare(uint8_t n = 10); //default n = 10 samples for avgs
bool torqueReady();