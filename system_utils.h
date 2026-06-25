#pragma once

// system_utils.h — Utilitas umum, sensor suhu, dan logger CSV

// Bobot & mode fuzzy 
void updateFuzzyWeights() {
  bool eV = sensorEnable[SENSOR_V];
  bool eP = sensorEnable[SENSOR_P];
  bool eI = sensorEnable[SENSOR_I];
  int n = (eV?1:0) + (eP?1:0) + (eI?1:0);
  if (n == 0) { fw_V=0.0f; fw_P=0.0f; fw_I=0.0f; return; }
  // Rasio bobot berbasis prioritas P≥V>I, renormalisasi per sensor aktif
  float wV = eV ? 0.40f : 0.0f;
  float wP = eP ? 0.40f : 0.0f;
  float wI = eI ? 0.20f : 0.0f;
  float sum = wV + wP + wI;
  fw_V = wV / sum; fw_P = wP / sum; fw_I = wI / sum;
}

void updateFuzzyMode() {
  String m = "";
  if (sensorEnable[SENSOR_V]) m += "V";
  if (sensorEnable[SENSOR_P]) m += "P";
  if (sensorEnable[SENSOR_I]) m += "I";
  fuzzyMode = (m.length() == 0) ? "-" : m;
  updateFuzzyWeights();
}

// EMA smoothing 
float smooth(float prev, float val, float alpha) {
  return alpha * val + (1.0f - alpha) * prev;
}

// Bitmask sensor aktif 
uint8_t getEnableMask() {
  uint8_t m = 0;
  if (sensorEnable[SENSOR_P]) m |= 0x01;
  if (sensorEnable[SENSOR_V]) m |= 0x02;
  if (sensorEnable[SENSOR_I]) m |= 0x04;
  return m;
}

// ADC oversampling 
int analogReadOversampled(int pin, int n) {
  long sum = 0;
  for (int i = 0; i < n; i++) sum += analogRead(pin);
  return (int)(sum / n);
}

// Sensor suhu internal ESP32 
void initTempSensor() {
  temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
  tempSensorOK = (temperature_sensor_install(&cfg, &temp_handle) == ESP_OK &&
                  temperature_sensor_enable(temp_handle) == ESP_OK);
}

void readChipTemp() {
  if (!tempSensorOK || millis() - lastTempRead < CFG_TEMP_READ_MS) return;
  lastTempRead = millis();
  float t = 0;
  if (temperature_sensor_get_celsius(temp_handle, &t) != ESP_OK) return;
  chipTemp = t;
  if (chipTemp >= CFG_TEMP_THROTTLE_C && !thermalThrottle) {
    thermalThrottle = true;
    setCpuFrequencyMhz(CFG_CPU_THROTTLE_MHZ);
    WiFi.setTxPower(WIFI_POWER_11dBm);
    Serial.printf("[TEMP] THROTTLE: %.1f°C\n", chipTemp);
  } else if (chipTemp < CFG_TEMP_RECOVER_C && thermalThrottle) {
    thermalThrottle = false;
    setCpuFrequencyMhz(CFG_CPU_NORMAL_MHZ);
    WiFi.setTxPower(WIFI_POWER_17dBm);
    Serial.printf("[TEMP] recover: %.1f°C\n", chipTemp);
  }
}

// Logger CSV 
void logRow(float vib, float cur, float pres_kPa, int pwm,
            float cav, int cs, float pid, const String& sens) {
  LogRow& r = csvBuf[csvHead];
  r.time_ms = millis(); r.vibration = vib; r.current = cur;
  r.pressure_kPa = pres_kPa; r.pwm = pwm;
  r.cavIdx = cav; r.cavState = cs; r.pidOut = pid;
  strncpy(r.sensorSt, sens.c_str(), 23); r.sensorSt[23] = '\0';
  strncpy(r.fuzzyMode, fuzzyMode.c_str(), 7); r.fuzzyMode[7] = '\0';
  strncpy(r.pidMode, pidActiveMode.c_str(), 3); r.pidMode[3] = '\0';
  r.enableMask = getEnableMask();
  csvHead = (csvHead + 1) % CFG_CSV_BUF_SIZE;
  if (csvCount < CFG_CSV_BUF_SIZE) csvCount++;
}
