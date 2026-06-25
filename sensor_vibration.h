#pragma once

// sensor_vibration.h — MPU6050, filter Hampel & median, kalibrasi

// Buffer filter (bukan global agar enkapsulasi bersih)
static float vib_hampel_buf[CFG_VIB_HAMPEL_N] = {0};
static int   vib_hampel_idx = 0;

float hampelFilter(float x) {
  vib_hampel_buf[vib_hampel_idx] = x;
  vib_hampel_idx = (vib_hampel_idx + 1) % CFG_VIB_HAMPEL_N;
  float s[CFG_VIB_HAMPEL_N];
  for (int i=0;i<CFG_VIB_HAMPEL_N;i++) s[i]=vib_hampel_buf[i];
  for (int i=0;i<CFG_VIB_HAMPEL_N-1;i++)
    for (int j=0;j<CFG_VIB_HAMPEL_N-1-i;j++)
      if (s[j]>s[j+1]){float t=s[j];s[j]=s[j+1];s[j+1]=t;}
  float med=s[CFG_VIB_HAMPEL_N/2];
  float dev[CFG_VIB_HAMPEL_N];
  for (int i=0;i<CFG_VIB_HAMPEL_N;i++) dev[i]=fabsf(s[i]-med);
  for (int i=0;i<CFG_VIB_HAMPEL_N-1;i++)
    for (int j=0;j<CFG_VIB_HAMPEL_N-1-i;j++)
      if (dev[j]>dev[j+1]){float t=dev[j];dev[j]=dev[j+1];dev[j+1]=t;}
  float mad=dev[CFG_VIB_HAMPEL_N/2]*1.4826f;
  if (mad<1e-6f) return x;
  if (fabsf(x-med) > CFG_VIB_HAMPEL_K*mad) return med;
  return x;
}

float medianFilter5(float new_val) {
  vib_med_buf[vib_med_idx] = new_val;
  vib_med_idx = (vib_med_idx + 1) % CFG_VIB_MEDIAN_N;
  float s[CFG_VIB_MEDIAN_N];
  for (int i = 0; i < CFG_VIB_MEDIAN_N; i++) s[i] = vib_med_buf[i];
  for (int i = 0; i < CFG_VIB_MEDIAN_N-1; i++)
    for (int j = 0; j < CFG_VIB_MEDIAN_N-1-i; j++)
      if (s[j] > s[j+1]) { float t=s[j]; s[j]=s[j+1]; s[j+1]=t; }
  return s[CFG_VIB_MEDIAN_N/2];
}

void readMPU() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  ax_raw = a.acceleration.x;
  ay_raw = a.acceleration.y;
  az_raw = a.acceleration.z;
}

void saveMPUCalibration() {
  nvs.begin("smartpump", false);
  nvs.putFloat("mpu_ax", ax_off);
  nvs.putFloat("mpu_ay", ay_off);
  nvs.putFloat("mpu_az", az_off);
  nvs.putBool ("mpu_done", true);
  nvs.end();
  mpu_cal_done = true; mpu_cal_timestamp = millis();
  Serial.printf("[CAL-M] saved: ax=%.4f ay=%.4f az=%.4f\n", ax_off, ay_off, az_off);
}

void loadMPUCalibration() {
  nvs.begin("smartpump", true);
  bool done = nvs.getBool("mpu_done", false);
  if (done) {
    ax_off = nvs.getFloat("mpu_ax", 0.0f);
    ay_off = nvs.getFloat("mpu_ay", 0.0f);
    az_off = nvs.getFloat("mpu_az", 0.0f);
    mpu_cal_done = true;
    Serial.printf("[CAL-M] loaded: ax=%.4f ay=%.4f az=%.4f\n", ax_off, ay_off, az_off);
  } else {
    Serial.println("[CAL-M] belum dikalibrasi, pakai default 0");
    mpu_cal_done = false;
  }
  nvs.end();
}

void resetMPUCalibration() {
  ax_off=0; ay_off=0; az_off=0; mpu_cal_done=false;
  nvs.begin("smartpump", false);
  nvs.remove("mpu_ax"); nvs.remove("mpu_ay");
  nvs.remove("mpu_az"); nvs.remove("mpu_done");
  nvs.end();
  s_ax=s_ay=0; s_az=CFG_VIB_GRAVITY;
  vib_baseline=CFG_VIB_GRAVITY; vib_error_count=0;
  for (int i=0; i<CFG_VIB_MEDIAN_N; i++) vib_med_buf[i]=0;
  for (int i=0; i<CFG_VIB_HAMPEL_N; i++) vib_hampel_buf[i]=0;
  vib_hampel_idx=0;
  Serial.println("[CAL-M] reset ke default");
}

void calibrateMPU() {
  Serial.println("[CAL-M] kalibrasi startup...");
  ax_off=0; ay_off=0; az_off=0;
  double sx=0, sy=0, sz=0;
  for (int i=0; i<CFG_MPU_CALIB_SAMPLES; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sx += a.acceleration.x;
    sy += a.acceleration.y;
    sz += (a.acceleration.z - CFG_VIB_GRAVITY);
    delay(2); esp_task_wdt_reset();
  }
  ax_off = (float)(sx/CFG_MPU_CALIB_SAMPLES);
  ay_off = (float)(sy/CFG_MPU_CALIB_SAMPLES);
  az_off = (float)(sz/CFG_MPU_CALIB_SAMPLES);
  saveMPUCalibration();
}

bool calibrateMPUWeb() {
  if (!mpuOK || pwmActual > 0) return false;
  mpu_cal_running=true; cal_progress=0;
  ax_off=0; ay_off=0; az_off=0;
  double sx=0, sy=0, sz=0;
  int n = CFG_MPU_CALIB_SAMPLES;
  for (int i=0; i<n; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sx += a.acceleration.x;
    sy += a.acceleration.y;
    sz += (a.acceleration.z - CFG_VIB_GRAVITY);
    cal_progress = (i+1)*100/n;
    delay(2); esp_task_wdt_reset();
  }
  ax_off=(float)(sx/n); ay_off=(float)(sy/n); az_off=(float)(sz/n);
  s_ax=s_ay=0; s_az=CFG_VIB_GRAVITY;
  vib_baseline=CFG_VIB_GRAVITY; vib_error_count=0;
  for (int i=0; i<CFG_VIB_MEDIAN_N; i++) vib_med_buf[i]=0;
  for (int i=0; i<CFG_VIB_HAMPEL_N; i++) vib_hampel_buf[i]=0;
  vib_hampel_idx=0;
  saveMPUCalibration();
  mpu_cal_running=false;
  return true;
}
