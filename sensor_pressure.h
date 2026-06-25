#pragma once

// sensor_pressure.h — Sensor tekanan WPT-83G + kalibrasi + moving average

float computePressureRaw_kPa(int adc_p) {
  float v_adc    = (adc_p / 4095.0f) * 3.3f;
  float v_sensor = v_adc * CFG_VDIV_FACTOR;
#if CFG_P_SENSOR_VACUUM
  return CFG_PV_KPA_MIN + (v_sensor - CFG_PV_VMIN) /
         (CFG_PV_VMAX - CFG_PV_VMIN) * (CFG_PV_KPA_MAX - CFG_PV_KPA_MIN);
#else
  float p_bar = (v_sensor - CFG_P_VMIN) / (CFG_P_VMAX - CFG_P_VMIN) * CFG_P_FULLSCALE;
  return p_bar * 100.0f;
#endif
}

float applyPressureCalibration(float p_raw_kPa) {
  return (p_raw_kPa - p_zero_offset_kPa) * p_gain;
}

void loadPressureCalibration() {
  nvs.begin("smartpump", true);
  p_zero_offset_kPa = nvs.getFloat("pcal_zero", CFG_PCAL_ZERO_DEFAULT);
  p_gain            = nvs.getFloat("pcal_gain", CFG_PCAL_GAIN_DEFAULT);
  p_cal_done        = nvs.getBool ("pcal_done", false);
  String method = nvs.getString("pcal_method", "default");
  strncpy(p_cal_method, method.c_str(), 15); p_cal_method[15] = '\0';
  nvs.end();
  if (p_zero_offset_kPa < CFG_PCAL_ZERO_MIN || p_zero_offset_kPa > CFG_PCAL_ZERO_MAX)
    p_zero_offset_kPa = CFG_PCAL_ZERO_DEFAULT;
  if (p_gain < CFG_PCAL_GAIN_MIN || p_gain > CFG_PCAL_GAIN_MAX)
    p_gain = CFG_PCAL_GAIN_DEFAULT;
  Serial.printf("[CAL-P] zero=%.3f kPa gain=%.4f done=%d method=%s\n",
                p_zero_offset_kPa, p_gain, p_cal_done, p_cal_method);
}

void savePressureCalibration(const char* method) {
  nvs.begin("smartpump", false);
  nvs.putFloat ("pcal_zero", p_zero_offset_kPa);
  nvs.putFloat ("pcal_gain", p_gain);
  nvs.putBool  ("pcal_done", true);
  nvs.putString("pcal_method", method);
  nvs.end();
  p_cal_done = true;
  strncpy(p_cal_method, method, 15); p_cal_method[15] = '\0';
  p_cal_timestamp = millis();
}

float calibratePressureZero() {
  float prev_zero = p_zero_offset_kPa, prev_gain = p_gain;
  p_zero_offset_kPa = 0.0f; p_gain = 1.0f;
  double sum = 0; int valid = 0;
  for (int i = 0; i < CFG_PCAL_SAMPLES; i++) {
    int adc_p = analogReadOversampled(CFG_PIN_PRESSURE, CFG_P_OVERSAMPLE);
    if (adc_p < 5 || adc_p > 4090) continue;
    sum += computePressureRaw_kPa(adc_p); valid++;
    delay(CFG_PCAL_SAMPLE_DELAY); esp_task_wdt_reset();
  }
  if (valid < CFG_PCAL_SAMPLES/2) {
    p_zero_offset_kPa = prev_zero; p_gain = prev_gain; return prev_zero;
  }
  float mean_raw = (float)(sum/valid);
  if (mean_raw < CFG_PCAL_ZERO_MIN || mean_raw > CFG_PCAL_ZERO_MAX) {
    p_zero_offset_kPa = prev_zero; p_gain = prev_gain; return prev_zero;
  }
  p_zero_offset_kPa = mean_raw; p_gain = prev_gain;
  const char* m = (fabsf(prev_gain-1.0f)>0.001f) ? "zero+span" : "zero";
  savePressureCalibration(m);
  Serial.printf("[CAL-P] zero done: offset=%.3f kPa\n", p_zero_offset_kPa);
  return p_zero_offset_kPa;
}

float calibratePressureSpan(float p_ref_kPa) {
  if (p_ref_kPa < 1.0f) return p_gain;
  float prev_gain = p_gain; p_gain = 1.0f;
  double sum = 0; int valid = 0;
  for (int i = 0; i < CFG_PCAL_SAMPLES; i++) {
    int adc_p = analogReadOversampled(CFG_PIN_PRESSURE, CFG_P_OVERSAMPLE);
    if (adc_p < 5 || adc_p > 4090) continue;
    sum += (computePressureRaw_kPa(adc_p) - p_zero_offset_kPa); valid++;
    delay(CFG_PCAL_SAMPLE_DELAY); esp_task_wdt_reset();
  }
  if (valid < CFG_PCAL_SAMPLES/2) { p_gain = prev_gain; return prev_gain; }
  float mean_z = (float)(sum/valid);
  if (fabsf(mean_z) < 1.0f) { p_gain = prev_gain; return prev_gain; }
  float new_gain = p_ref_kPa / mean_z;
  if (new_gain < CFG_PCAL_GAIN_MIN || new_gain > CFG_PCAL_GAIN_MAX) {
    p_gain = prev_gain; return prev_gain;
  }
  p_gain = new_gain;
  savePressureCalibration("zero+span");
  Serial.printf("[CAL-P] span done: gain=%.4f\n", p_gain);
  return p_gain;
}

void resetPressureCalibration() {
  p_zero_offset_kPa = CFG_PCAL_ZERO_DEFAULT;
  p_gain = CFG_PCAL_GAIN_DEFAULT;
  p_cal_done = false;
  strncpy(p_cal_method, "default", 15);
  nvs.begin("smartpump", false);
  nvs.remove("pcal_zero"); nvs.remove("pcal_gain");
  nvs.remove("pcal_done"); nvs.remove("pcal_method");
  nvs.end();
}

float movingAvgPressure(float v) {
  ma_sum -= ma_buf[ma_idx];
  ma_buf[ma_idx] = v; ma_sum += v;
  ma_idx = (ma_idx+1) % CFG_MA_SIZE;
  if (ma_idx==0) ma_full = true;
  int n = ma_full ? CFG_MA_SIZE : (ma_idx==0 ? CFG_MA_SIZE : ma_idx);
  return ma_sum / n;
}

void initMovingAvg() {
  for (int i=0; i<CFG_MA_SIZE; i++) ma_buf[i]=0;
  ma_sum=0; ma_idx=0; ma_full=false;
}
