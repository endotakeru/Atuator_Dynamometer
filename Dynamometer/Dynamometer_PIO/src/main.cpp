#include <Arduino.h>
#include "config.h"
//....

static uint32_t last_tick_ms = 0; // (?)
// put function declarations here:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(SERIAL_BAUD);
  pinMode(PIN_LED, OUTPUT);

  Serial.println(F("# Dynamometer constant-speed (rpm sweep) ready"));
  Serial.println(F("t_ms,mode,setpoint_rpm,rpm,torque_Nm,voltage_V,current_A,elec_W,brake_W,eff,servo_us,demand"));
}

void handleSerial(){
}

void loop() {
  // put your main code here, to run repeatedly:
  const uint32_t now = millis();
  if (now-last_tick_ms < CONTROL_DT_MS) return;
  const float dt = (now-last_tick_ms) / 1000.0f;
  last_tick_ms = now;
}

// put function definitions here: