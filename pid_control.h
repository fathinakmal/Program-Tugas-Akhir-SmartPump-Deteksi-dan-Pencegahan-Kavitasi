#pragma once

// pid_control.h — PID FGS position-form, state kavitasi, mode, rate-limit

String determinePIDMode() {
  bool okV = sensorEnable[SENSOR_V] && vibrationOK && mpuOK;
  bool okP = sensorEnable[SENSOR_P] && pressureOK;
  bool okI = sensorEnable[SENSOR_I] && currentOK;
  if      (ctrlPrimary == 'V') { if (okV) return "V"; if (okP) return "P"; if (okI) return "I"; }
  else if (ctrlPrimary == 'P') { if (okP) return "P"; if (okV) return "V"; if (okI) return "I"; }
  else                         { if (okI) return "I"; if (okV) return "V"; if (okP) return "P"; }
  return "-";
}

float computePID_FGS(float sp, float pv, float cavIdx,
                     char mode, float deadband, float dt) {
  float error = sp - pv;
  if (fabsf(error) <= deadband) { pid_integral *= 0.98f; pid_prev_error = error; return 0.0f; }
  DynGains g = fuzzyGainSchedule(cavIdx, mode);
  if (g.Kp < 0.001f && g.Ki < 0.001f && g.Kd < 0.001f) {
    pid_integral = 0; pid_prev_error = error; return 0.0f;
  }
  if (dt < CFG_DT_MIN) dt = CFG_DT_MIN;
  bool satHi  = (pwmActual >= CFG_PWM_MAX && error > 0);
  bool satLo  = (pwmActual <= CFG_PWM_MIN && error < 0);
  // Cegah windup saat cavPwmCap menahan kenaikan: error positif tapi cap
  // memblokir PWM naik → integral akan terus tumbuh tanpa guna.
  bool satCap = (!testFixedMode && pwmActual >= cavPwmCap && error > 0);
  if (g.Ki > 0.0f && !satHi && !satLo && !satCap)
    pid_integral = constrain(pid_integral + error * dt, -CFG_INTEGRAL_MAX, CFG_INTEGRAL_MAX);
  float raw_d = (error - pid_prev_error) / dt;
  pid_deriv_filt = smooth(pid_deriv_filt, raw_d, CFG_DERIV_ALPHA);
  pid_prev_error = error;
  return g.Kp * error + g.Ki * pid_integral + g.Kd * pid_deriv_filt;
}

char modeChar() {
  if (pidActiveMode.length() == 0) return '-';
  return pidActiveMode.charAt(0);
}

int updateCavState(float idx) {
  int target = cavState;
  switch (cavState) {
    case 0: if (idx > CFG_CAV_0_TO_1) target=1; break;
    case 1:
      if      (idx < CFG_CAV_1_TO_0) target=0;
      else if (idx > CFG_CAV_1_TO_2) target=2;
      break;
    case 2: if (idx < CFG_CAV_2_TO_1) target=1; break;
  }
  unsigned long now = millis();
  if (target == cavState) { cav_pending = cavState; cavPendSince = now; return cavState; }
  if (target != cav_pending) { cav_pending = target; cavPendSince = now; return cavState; }
  unsigned long needMs = (target > cavState) ? CFG_CAV_CONFIRM_UP_MS : CFG_CAV_CONFIRM_DOWN_MS;
  if ((now - cavPendSince) >= needMs && (now - cavStateEnterTime) >= CFG_CAV_HOLD_MS) {
    Serial.printf("[CAV] %d->%d  K=%.3f\n", cavState, target, idx);
    cavState = target; cavStateEnterTime = now; cavPendSince = now;
  }
  return cavState;
}

int pidToDelta(float du, int state) {
  int mx = (state == 0) ? CFG_DELTA_MAX_NORMAL
         : (state == 1) ? CFG_DELTA_MAX_EARLY
         :                CFG_DELTA_MAX_SEVERE;
  return constrain((int)roundf(du), -mx, mx);
}

int applyRateLimit(int t, int a) {
  int d = t - a;
  if (d >  CFG_PWM_RATE_UP)   return a + CFG_PWM_RATE_UP;
  if (d < -CFG_PWM_RATE_DOWN) return a - CFG_PWM_RATE_DOWN;
  return t;
}
