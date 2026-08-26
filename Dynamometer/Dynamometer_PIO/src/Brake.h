#pragma once
#include <Arduino.h>

float brakeRpmToDemand(float rpm_shed);

void  brakeBegin();
void  brakeSetDemand(float demand01);
void  brakeSetMicroseconds(int us);
void  brakeRelease();
int   brakeMicroseconds();
float brakeDemand(); 