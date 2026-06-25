#pragma once

// sensor_current.h — Sensor arus ACS712 + kalibrasi + deficit

void saveACSCalibration() {
  nvs.begin("smartpump", false);
  nvs.putFloat("acs_offset", acs_offset);
  nvs.putFloat("acs_gain",   acs_cal_gain);
  nvs.putBool ("acs_done",   true);
  nvs.end();
  acs_cal_done=true; acs_cal_timestamp=millis();
  Serial.printf("[CAL-A] saved: offset=%.4fV gain=%.4f\n", acs_offset, acs_cal_gain);
}

void loadACSCalibration() {
  nvs.begin("smartpump", true);
  bool done = nvs.getBool("acs_done", false);
  if (done) {
    acs_offset   = nvs.getFloat("acs_offset", 2.5f);
    acs_cal_gain = nvs.getFloat("acs_gain",   1.0f);
    acs_cal_done = true;
    Serial.printf("[CAL-A] loaded: offset=%.4fV gain=%.4f\n", acs_offset, acs_cal_gain);
  } else {
    Serial.println("[CAL-A] belum dikalibrasi, pakai default");
    acs_cal_done = false;
  }
  nvs.end();
}

void resetACSCalibration() {
  acs_offset=2.5f; acs_cal_gain=1.0f; acs_cal_done=false;
  s_i=0; current_cached=0; cur_neg_count=0;
  nvs.begin("smartpump", false);
  nvs.remove("acs_offset"); nvs.remove("acs_gain"); nvs.remove("acs_done");
  nvs.end();
  Serial.println("[CAL-A] reset ke default");
}

bool calibrateACSZero() {
  if (pwmActual > 0) { Serial.println("[CAL-A] pompa harus mati"); return false; }
  acs_cal_running=true; cal_progress=0;
  double sum=0; int n=CFG_CALIB_SAMPLES;
  for (int i=0; i<n; i++) {
    sum += (analogRead(CFG_PIN_ACS)/4095.0f)*3.3f*CFG_VDIV_FACTOR;
    cal_progress=(i+1)*100/n;
    delay(2); esp_task_wdt_reset();
  }
  acs_offset=(float)(sum/n);
  saveACSCalibration();
  s_i=0; current_cached=0; cur_neg_count=0;
  acs_cal_running=false;
  Serial.printf("[CAL-A] zero done: offset=%.4fV\n", acs_offset);
  return true;
}

bool calibrateACSSpan(float i_ref_A) {
  if (i_ref_A < 0.05f) return false;
  acs_cal_running=true; cal_progress=0;
  double sum=0; int n=CFG_CALIB_SAMPLES;
  for (int i=0; i<n; i++) {
    float v=(analogRead(CFG_PIN_ACS)/4095.0f)*3.3f*CFG_VDIV_FACTOR;
    sum += (v-acs_offset)/CFG_ACS_SENSITIVITY;
    cal_progress=(i+1)*100/n;
    delay(2); esp_task_wdt_reset();
  }
  float i_raw=(float)(sum/n);
  if (fabsf(i_raw)<0.01f) { acs_cal_running=false; return false; }
  float ng = i_ref_A/i_raw;
  if (ng<0.3f || ng>3.0f) {
    Serial.printf("[CAL-A] gain=%.4f di luar range\n",ng);
    acs_cal_running=false; return false;
  }
  acs_cal_gain=ng; acs_cal_ref_A=i_ref_A;
  saveACSCalibration();
  s_i=0; current_cached=0;
  acs_cal_running=false;
  Serial.printf("[CAL-A] span done: gain=%.4f (raw=%.3fA ref=%.3fA)\n",
                acs_cal_gain, i_raw, i_ref_A);
  return true;
}

float calibrateACS() {
  Serial.println("[CAL-A] kalibrasi startup zero...");
  float sum=0;
  for (int i=0; i<CFG_CALIB_SAMPLES; i++) {
    sum += (analogRead(CFG_PIN_ACS)/4095.0f)*3.3f*CFG_VDIV_FACTOR;
    delay(2); esp_task_wdt_reset();
  }
  return sum/CFG_CALIB_SAMPLES;
}

float currentDeficit(float i_meas, int pwm) {
  if (pwm < CFG_PWM_FLOW_MIN) return 0.0f;
  float i_exp = iSlopeLearned * pwm + CFG_I_MODEL_OFFSET;
  if (i_exp < 0.05f) return 0.0f;
  float d = (i_exp - i_meas) / i_exp;
  return constrain(d, 0.0f, 1.0f);
}
