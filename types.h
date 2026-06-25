#pragma once
#include <stdint.h>

struct DynGains { float Kp; float Ki; float Kd; };

struct LogRow {
  unsigned long time_ms;
  float vibration, current, pressure_kPa, cavIdx, pidOut;
  int   pwm, cavState;
  char  sensorSt[24];
  char  fuzzyMode[8];
  char  pidMode[4];
  uint8_t enableMask;
};

struct FuzzyRule {
  uint8_t p_idx;
  uint8_t v_idx;
  uint8_t i_idx;
  uint8_t out_idx;
};
