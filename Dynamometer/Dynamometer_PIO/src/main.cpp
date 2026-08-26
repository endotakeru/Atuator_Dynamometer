#include "config.h"
#include "Calibration.h"
#include "Torque.h"
#include "Tach.h"
#include "PowerMeter.h"
#include "Brake.h"
#include "Controller.h"

static uint32_t last_tick_ms = 0; // (?)

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(PIN_LED, OUTPUT);

  calLoadDefaults(); // (?)
  brakeBegin(); // (?)
  calLoad(); // (?)

  torqueBegin();
  tachBegin();
  controlBegin();
  brakeRelease();

  Serial.println(F("# Dynamometer constant-speed (rpm sweep) ready"));
  Serial.println(F("t_ms,mode,setpoint_rpm,rpm,torque_Nm,voltage_V,current_A,elec_W,brake_W,eff,servo_us,demand"));
  calPrint(); // (?)
}

void handleSerial(){
  static char buf[64]; // (?)
  static uint8_t n = 0; // (?)

  while (Serial.available()){ // (?)
    const char ch = Serial.read(); // (?)
    if (ch != '\n' && ch != '\r'){ // (?)
      if (n < sizeof(buf) - 1) buf[n++] = ch; // (?)
      continue;
    }
    if (n == 0) continue; // (?)
    buf[n] = '\0'; // (?)
    n = 0; // (?)

    const char cmd = buf[0]; // (?)
    char *arg = buf + 1;

    switch(cmd){
      case 'X':
        cal.torque_limit_Nm = constrain((float)atof(arg), 0.0f, TORQUE_LIMIT_ABS_NM); // (?)
        Serial.print(F("# torque_limit_Nm=")); Serial.println(cal.torque_limit_Nm, 3); // (?)
      case 'S':{
        const float sp = atof(arg); // (?)
        if (sp < 0.0f){controlSetMode(MODE_IDLE); controlSetSpeed(0);} // (?)
        else {controlSetSpeed(sp); controlSetMode(MODE_SPEED);} // (?)
        break;
      }
      case 'M': controlSetMode(MODE_MANUAL); brakeRelease(); break; // (?)
      case 'B': controlSetMode(MODE_MANUAL); brakeSetMicroseconds(atoi(arg)); // (?)
        Serial.print(F("# released")); break; // (?)
      case 'R': controlSetMode(MODE_IDLE); controlSetSpeed(0); brakeRelease(); // (?)
        Serial.println(F("# released")); break; // (?)
      case 'T': torqueTare(10); Serial.println(F("# tared")); break; // (?)
      case 'K': cal.counts_per_Nm = atof(arg); // (?)
        if(cal.counts_per_Nm == 0.0f) cal.counts_per_Nm = 1.0f; // (?)
        Serial.print(F("# counts_per_Nm=")); Serial.println(cal.counts_per_Nm, 4); // (?)
        break; // (?)
      case 'D': cal.drag_tare_Nm = atof(arg); break; // (?)
      case 'Z': cal.drag_tare_Nm = torqueRawFiltered(); // (?)
        Serial.print(F("# drag_tare_Nm=")); Serial.println(cal.drag_tare_Nm, 5); // (?)
        break; // (?)
      case 'G':{ // (?)
        char *p1 = strtok(arg, " "), *p2 = strtok(NULL, " "), *p3 = strtok(NULL, " ");
        if (p1 && p2 && p3){cal.kp = atof(p1); cal.ki = atof(p2); cal.kd = atof(p3);}
        calPrint(); // (?)
        break;
      }
      case 'L':{ // (?)
        char *p1 = strtok(arg, " "), *p2 = strtok(NULL, " ");
        if(p1 && p2){
          cal.servo_released_us = constrain(atoi(p1), SERVO_US_ABS_MIN, SERVO_US_ABS_MAX);
          cal.servo_full_us = constrain(atoi(p2), SERVO_US_ABS_MIN, SERVO_US_ABS_MAX);
        }
      }
      case 'F': cal.stiction_floor = constrain((float)atof(arg), 0.0f, 0.5f); break; // (?)
      case 'N': cal.brake_rpm_span = max((float)atof(arg), 1.0f); // (?)
        Serial.print(F("# brake_rpm_span=")); Serial.println(cal.brake_rpm_span, 1); // (?)
        break;
      case 'P': Serial.print(F("# tach_pulses=")); Serial.println(tachTotalPulses()); break; // (?)
      case 'W': calSave(); break; // (?)
      case '?': calPrint(); break; // (?)
      default: Serial.print(F("# unknown cmd: ")); Serial.println(buf); break;
    }
  }
}

void loop() {
  handleSerial();

  const uint32_t now = millis();
  if (now-last_tick_ms < CONTROL_DT_MS) return;
  const float dt = (now-last_tick_ms) / 1000.0f; // (?)
  last_tick_ms = now;

  torqueUpdate();
  tachUpdate(dt);
  powerUpdate();

  const float torque_Nm = torqueRead();
  const float rpm = tachRPM();
  const float voltage = powerVoltage();
  const float current = powerCurrent();
  const float elec_W = powerElectricalW();

  controlUpdate(torque_Nm, rpm, dt);

  const float brake_W = (2.0f * PI * rpm * torque_Nm) / 60.0f; // Output
  const float eff = (elec_W > 0.5f) ? (brake_W / elec_W) : 0.0f; // (?) Input

  Serial.print(now); Serial.print(',');
  Serial.print((int)controlMode()); Serial.print(','); // (?)
  Serial.print(controlSpeedSetpoint(), 1); Serial.print(',');
  Serial.print(rpm, 1); Serial.print(',');
  Serial.print(torque_Nm, 4); Serial.print(',');
  Serial.print(voltage, 3); Serial.print(',');
  Serial.print(current, 4); Serial.print(',');
  Serial.print(elec_W, 3); Serial.print(',');
  Serial.print(brake_W, 3); Serial.print(',');
  Serial.print(eff, 4); Serial.print(',');
  Serial.print(brakeMicroseconds()); // (?) 
  Serial.println(brakeDemand(), 4); // (?)

  digitalWrite(LED_PIN, controlMode() == MODE_SPEED); // (?)
}