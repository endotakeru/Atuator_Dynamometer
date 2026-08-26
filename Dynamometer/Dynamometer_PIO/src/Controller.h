#pragma once
#include <Arduino.h>

enum Mode: uint8_t{MODE_IDLE = 0, MODE_SPEED = 1, MODE_MANUAL = 2}; // (?)

void controlBegin();
void controlSetMode(Mode m);
Mode controlMode();

void vontrolSetSpeed(float rpm);
float controlSpeedSetpoint();

void controlReset();
void controlUpdate(float torque, float rpm, float dt);