//  SMART PUMP — Deteksi & Pengendalian Kavitasi (ESP32-S3)  [Tugas Akhir]

// Library 
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <esp_wifi.h>
#include "esp_task_wdt.h"
#include "esp_pm.h"
#include "driver/temperature_sensor.h"

// Konfigurasi & Tipe (harus sebelum globals)
#include "config.h"
#include "types.h"

// VARIABEL GLOBAL
WiFiServer    server(80);
Adafruit_MPU6050 mpu;

bool       sensorEnable[SENSOR_COUNT] = {
  CFG_DEFAULT_EN_P, CFG_DEFAULT_EN_V, CFG_DEFAULT_EN_I
};
const char* SENSOR_NAME[SENSOR_COUNT] = {"PRESSURE","VIBRATION","CURRENT"};

// Relay state machine
int           relayState   = 0;
unsigned long relayTimer   = 0;
bool          relayPending = false;
bool          relayDir     = false;

// Thermal
float         chipTemp       = 0.0f;
bool          tempSensorOK   = false;
temperature_sensor_handle_t temp_handle = NULL;
unsigned long lastTempRead   = 0;
bool          thermalThrottle = false;

// Sensor offsets
float acs_offset = 2.5f;
float ax_off = 0, ay_off = 0, az_off = 0;

// Moving average tekanan
float ma_buf[CFG_MA_SIZE];
int   ma_idx = 0;
float ma_sum = 0;
bool  ma_full = false;

// Error counters
int vib_error_count = 0;
int cur_neg_count   = 0;

// Filter vibrasi
float vib_baseline = CFG_VIB_GRAVITY;
float vib_med_buf[CFG_VIB_MEDIAN_N] = {0,0,0,0,0};
int   vib_med_idx = 0;

// Kalibrasi NVS
Preferences nvs;
float p_zero_offset_kPa  = CFG_PCAL_ZERO_DEFAULT;
float p_gain             = CFG_PCAL_GAIN_DEFAULT;
bool  p_cal_done         = false;
unsigned long p_cal_timestamp = 0;
char  p_cal_method[16]   = "default";

bool  acs_cal_done       = false;
unsigned long acs_cal_timestamp = 0;
float acs_cal_ref_A      = 0.0f;
float acs_cal_gain       = 1.0f;

bool  mpu_cal_done       = false;
unsigned long mpu_cal_timestamp = 0;
volatile bool mpu_cal_running  = false;
volatile bool acs_cal_running  = false;
volatile int  cal_progress     = 0;

// Throughput monitor
unsigned long loop_count_window = 0;
unsigned long loop_window_start = 0;
float         loops_per_sec     = 0;
unsigned long bytes_sent_total  = 0;
unsigned long bytes_window_last = 0;
unsigned long bytes_window_at   = 0;
float         mbps_value        = 0.0f;

// Mode & kontrol
String pidActiveMode     = "V";
char   ctrlPrimary       = CFG_CTRL_PRIMARY;
bool   testFixedMode     = (CFG_TEST_FIXED_PWM != 0);
int    cavPwmCap         = CFG_PWM_MAX;
int    testHoldPwm       = 50;
int    curSpeedTargetPct = CFG_I_SPEED_TARGET_DEFAULT;
int    ispEffPct         = CFG_I_SPEED_TARGET_DEFAULT; // setpoin efektif setelah reduksi kavitasi
float  pressureSetpoint  = CFG_PRESSURE_REF;
float  pBaseline = 0.0f;
bool   pBaselineReady    = false;
float  iSlopeLearned     = CFG_I_MODEL_SLOPE;
unsigned long pLearnStart = 0;
float  pLearnAccum = 0;
long   pLearnN = 0;
float  pidSetpoint = 0;
float  pidPV       = 0;
float  fw_P = 0.10f, fw_V = 0.65f, fw_I = 0.25f;

// Logger CSV
LogRow        csvBuf[CFG_CSV_BUF_SIZE];
int           csvHead = 0, csvCount = 0;
bool          logging = false;
unsigned long lastLog = 0;
unsigned long logInterval = CFG_LOG_INTERVAL_MS;

// Status flags
bool manualStop    = true;   // true saat boot — pompa hanya jalan setelah /start ditekan
bool startupMode   = false;
bool stabilizing   = false;
bool emergencyStop = false;
bool sensorOK      = true;
bool mpuOK         = true;
bool systemReady   = false;
bool pressureOK    = true;
bool currentOK     = true;
bool vibrationOK   = true;

// PWM & aktuator
int           pwmTarget       = 0;
int           pwmActual       = 0;
unsigned long lastActuator    = 0;
unsigned long startupDoneTime = 0;
int           loopCounter     = 0;
unsigned long lastLoopUs      = 0;

// Data publik (dibaca web)
String sensorStatus = "NORMAL";
String fuzzyMode    = "VPI";
float g_vibration = 0, g_current = 0, g_pressure_kPa = 0;
float g_cavitation = 0, g_pid_output = 0;
int   g_cavState   = 0;

// Filter state
float s_ax=0, s_ay=0, s_az=0, s_i=0, s_p=0;
float ax_raw=0, ay_raw=0, az_raw=0;
float current_cached = 0;
float pres_cached    = 0;

// PID state
float         pid_integral   = 0;
float         pid_prev_error = 0;
float         pid_deriv_filt = 0;
unsigned long pid_last_ms    = 0;

// Cavitation state
int           cavState          = 0;
unsigned long cavStateEnterTime = 0;
int           cav_pending       = 0;
unsigned long cavPendSince      = 0;
int           cavStatePrev      = 0;   // state sebelumnya untuk deteksi transisi → recovery
unsigned long cavRecoveryUntil  = 0;   // waktu akhir recovery hold pasca-kavitasi
unsigned long cavDetectEnableAt = 0;   // deteksi diaktifkan setelah settle pasca-stabilisasi

// Web server state
WiFiClient    activeClient;
String        clientRequest = "";
bool          clientActive  = false;
unsigned long clientTimeout = 0;
bool          csvStreaming  = false;
int           csvStreamIdx = 0, csvStreamTotal = 0, csvStreamStart = 0;

//  FUNGSI SISTEM (urutan include = urutan dependensi)
#include "system_utils.h"
#include "actuator.h"
#include "sensor_pressure.h"
#include "sensor_current.h"
#include "sensor_vibration.h"
#include "fuzzy.h"
#include "pid_control.h"
#include "web_html.h"
#include "web_server.h"

//  SETUP
void setup() {
  Serial.begin(CFG_SERIAL_BAUD); delay(500);
  setCpuFrequencyMhz(CFG_CPU_NORMAL_MHZ);

  esp_task_wdt_deinit();
  esp_task_wdt_config_t wdt_cfg = {
    .timeout_ms=CFG_WDT_TIMEOUT_MS, .idle_core_mask=0, .trigger_panic=true
  };
  esp_task_wdt_init(&wdt_cfg);
  esp_task_wdt_add(NULL);

  WiFi.softAP(CFG_WIFI_SSID, CFG_WIFI_PASSWORD);
  WiFi.setTxPower(WIFI_POWER_17dBm);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  server.begin();
  Serial.printf("[WiFi] AP: %s  IP: %s\n",
                CFG_WIFI_SSID, WiFi.softAPIP().toString().c_str());

  Wire.begin(CFG_PIN_SDA, CFG_PIN_SCL);
  Wire.setClock(CFG_I2C_CLOCK_HZ);
  if (mpu.begin(0x68)) {
    mpuOK = true;
    mpu.setAccelerometerRange(CFG_MPU_ACCEL_RANGE);
    mpu.setGyroRange(CFG_MPU_GYRO_RANGE);
    mpu.setFilterBandwidth(CFG_MPU_DLPF);
    loadMPUCalibration();
    if (!mpu_cal_done) calibrateMPU();
    Serial.println("[MPU] OK");
  } else {
    mpuOK = false;
    Serial.println("[ERR] MPU6050 tidak ditemukan! Fallback ke P atau I.");
  }

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(CFG_PIN_RELAY_UP,   OUTPUT); digitalWrite(CFG_PIN_RELAY_UP,   LOW);
  pinMode(CFG_PIN_RELAY_DOWN, OUTPUT); digitalWrite(CFG_PIN_RELAY_DOWN, LOW);

  loadACSCalibration();
  if (!acs_cal_done) {
    acs_offset = calibrateACS(); acs_cal_gain = 1.0f; saveACSCalibration();
  }

  initMovingAvg();
  loadPressureCalibration();
  ctrlPrimary = CFG_CTRL_PRIMARY; // prioritas ditetapkan program, bukan NVS
  { nvs.begin("smartpump", true);
    int sp = nvs.getInt("ispd", CFG_I_SPEED_TARGET_DEFAULT);
    pressureSetpoint = nvs.getFloat("psp", CFG_PRESSURE_REF); nvs.end();
    curSpeedTargetPct = constrain(sp, CFG_I_SPEED_MIN, 100); }

  pid_last_ms = millis(); cavStateEnterTime = millis();
  initTempSensor(); lastLoopUs = micros();
  updateFuzzyMode();

  Serial.println("=============================================");
  Serial.println(" Smart Pump — Deteksi & Pengendalian Kavitasi");
  Serial.printf (" MPU6050 (V): %s\n", mpuOK?"OK":"TIDAK DITEMUKAN");
  Serial.printf (" PID priority (program): %c -> %s\n", ctrlPrimary, ctrlPrimary=='I'?"I>V>P":ctrlPrimary=='V'?"V>P>I":"P>V>I");
  Serial.printf (" SP_V=%.2f m/s²  SP_P=%.3f kPa  SP_I=adaptif\n",
                  CFG_VIB_SETPOINT, CFG_PRESSURE_REF);
  Serial.printf (" CAV thresh: 0→1=%.2f 1→0=%.2f 1→2=%.2f 2→1=%.2f\n",
                  CFG_CAV_0_TO_1, CFG_CAV_1_TO_0,
                  CFG_CAV_1_TO_2, CFG_CAV_2_TO_1);
  Serial.printf (" Bobot: wV=%.2f wP=%.2f wI=%.2f\n", fw_V, fw_P, fw_I);
  Serial.println("=============================================");
}

//  LOOP UTAMA
void loop() {
  if (micros()-lastLoopUs < (unsigned long)CFG_LOOP_MIN_US) vTaskDelay(1);
  lastLoopUs = micros();
  loopCounter++;
  loop_count_window++;

  unsigned long _now = millis();
  if (loop_window_start==0) loop_window_start=_now;
  if (_now-loop_window_start >= 1000) {
    loops_per_sec = (float)loop_count_window*1000.0f/(float)(_now-loop_window_start);
    loop_count_window=0; loop_window_start=_now;
  }
  if (bytes_window_at==0) bytes_window_at=_now;
  if (_now-bytes_window_at >= 1000) {
    unsigned long delta = bytes_sent_total - bytes_window_last;
    mbps_value = (float)delta/1024.0f/1024.0f*1000.0f/(float)(_now-bytes_window_at);
    bytes_window_last = bytes_sent_total; bytes_window_at = _now;
  }

  esp_task_wdt_reset();
  tickRelay();

  // ── BACA SENSOR 
  if (sensorEnable[SENSOR_V] && mpuOK) readMPU();
  else { ax_raw=0; ay_raw=0; az_raw=CFG_VIB_GRAVITY; }

  if ((loopCounter % 2) == 0) {
    if (sensorEnable[SENSOR_I]) {
      int adc_i = analogReadOversampled(CFG_PIN_ACS, CFG_P_OVERSAMPLE);
      float v_sense = (adc_i/4095.0f)*3.3f*CFG_VDIV_FACTOR;
      current_cached = (v_sense - acs_offset) / CFG_ACS_SENSITIVITY;
      currentOK = !(adc_i<5 || adc_i>4090 ||
                    current_cached<CFG_I_MIN_VALID ||
                    current_cached>CFG_I_MAX_VALID);
      if (fabsf(current_cached) < CFG_I_DEADZONE) current_cached=0;
      else current_cached *= acs_cal_gain;
      if (current_cached < 0) {
        cur_neg_count++;
        if (cur_neg_count>CFG_I_DRIFT_THRESH && pwmActual==0) {
          float noff = (analogReadOversampled(CFG_PIN_ACS,16)/4095.0f)*3.3f*CFG_VDIV_FACTOR;
          acs_offset = smooth(acs_offset, noff, 0.3f);
          cur_neg_count = 0;
          Serial.printf("[ACS] auto-koreksi offset: %.4fV\n", acs_offset);
        }
      } else { cur_neg_count = 0; }
      current_cached = constrain(current_cached, CFG_I_CLAMP_LOW, CFG_I_CLAMP_HIGH);
    } else { current_cached=0.0f; currentOK=true; }

    if (sensorEnable[SENSOR_P]) {
      int adc_p = analogReadOversampled(CFG_PIN_PRESSURE, CFG_P_OVERSAMPLE);
      float p_kPa = applyPressureCalibration(computePressureRaw_kPa(adc_p));
      pressureOK = !(adc_p<5 || adc_p>4090 ||
                     p_kPa<CFG_P_MIN_KPA || p_kPa>CFG_P_MAX_KPA);
      p_kPa = max(0.0f, p_kPa);
      pres_cached = movingAvgPressure(p_kPa);
    } else { pres_cached=0.0f; pressureOK=true; }
  }

  // EMA FILTER 
  if (sensorEnable[SENSOR_V]) {
    s_ax = smooth(s_ax, ax_raw-ax_off, CFG_EMA_ALPHA_ACCEL);
    s_ay = smooth(s_ay, ay_raw-ay_off, CFG_EMA_ALPHA_ACCEL);
    s_az = smooth(s_az, az_raw-az_off, CFG_EMA_ALPHA_ACCEL);
  } else { s_ax=s_ay=0; s_az=CFG_VIB_GRAVITY; }
  if (sensorEnable[SENSOR_I]) s_i = smooth(s_i, current_cached, CFG_EMA_ALPHA_CURR);
  else s_i = 0;
  if (sensorEnable[SENSOR_P]) s_p = smooth(s_p, pres_cached, CFG_EMA_ALPHA_PRES);
  else s_p = 0;

  // GRAVITY REMOVAL + FILTER 
  float vib_mag = sqrtf(s_ax*s_ax + s_ay*s_ay + s_az*s_az);
  if (pwmActual==0 && sensorEnable[SENSOR_V])
    vib_baseline = smooth(vib_baseline, vib_mag, CFG_EMA_ALPHA_VIB_BASE);
  float vib_pre  = fabsf(vib_mag - vib_baseline);
  float vibration = medianFilter5(hampelFilter(vib_pre));
  mpuOK = !(isnan(ax_raw)||isnan(ay_raw)||isnan(az_raw));

  if (sensorEnable[SENSOR_V]) {
    bool spike = (fabsf(ax_raw)>CFG_VIB_MAX_VALID ||
                  fabsf(ay_raw)>CFG_VIB_MAX_VALID ||
                  fabsf(az_raw)>CFG_VIB_MAX_VALID);
    if (spike) { vib_error_count++; vibrationOK=(vib_error_count<CFG_VIB_PERSIST_N); }
    else        { vib_error_count=0; vibrationOK=true; }
  } else { vibrationOK=true; vib_error_count=0; }

  // STATUS SENSOR 
  bool anyActive = sensorEnable[SENSOR_P] || sensorEnable[SENSOR_V] || sensorEnable[SENSOR_I];
  bool activeP_ok = !sensorEnable[SENSOR_P] || pressureOK;
  bool activeV_ok = !sensorEnable[SENSOR_V] || (vibrationOK && mpuOK);
  bool activeI_ok = !sensorEnable[SENSOR_I] || currentOK;
  bool noNaN      = !(isnan(vibration)||isnan(s_p)||isnan(s_i));
  sensorOK   = anyActive && activeP_ok && activeV_ok && activeI_ok && noNaN;
  systemReady = sensorOK;

  String parts = "";
  if (!sensorEnable[SENSOR_V]) parts += "[V-OFF]";
  if (!sensorEnable[SENSOR_P]) parts += "[P-OFF]";
  if (!sensorEnable[SENSOR_I]) parts += "[I-OFF]";
  if      (sensorEnable[SENSOR_V]&&!vibrationOK) sensorStatus = "VIBRATION ERROR"+parts;
  else if (sensorEnable[SENSOR_V]&&!mpuOK)       sensorStatus = "MPU6050 ERROR"  +parts;
  else if (sensorEnable[SENSOR_P]&&!pressureOK)  sensorStatus = "PRESSURE ERROR" +parts;
  else if (sensorEnable[SENSOR_I]&&!currentOK)   sensorStatus = "CURRENT ERROR"  +parts;
  else if (!anyActive)                            sensorStatus = "NO SENSOR ACTIVE";
  else if (parts.length()>0)                      sensorStatus = "NORMAL "+parts;
  else                                            sensorStatus = "NORMAL";

  // FUZZY MAMDANI 
  bool cavDetectActive = !startupMode && !stabilizing &&
                         (millis() >= cavDetectEnableAt);
  float cavIdx = 0.0f;
  if (cavDetectActive) {
    cavIdx = fuzzyMamdaniAdaptive(s_p, vibration, s_i, fuzzyMode);
  } else {
    cavState = 0; cavStatePrev = 0; cav_pending = 0;
    cavStateEnterTime = millis(); cavPendSince = millis();
  }
  int cs = cavDetectActive ? updateCavState(cavIdx) : 0;

  // AUTO-TEKANAN (jika CFG_P_AUTO=1) 
#if CFG_P_AUTO
  if (sensorEnable[SENSOR_P] && pressureOK && !manualStop && !emergencyStop && !startupMode) {
    if (!pBaselineReady) {
      if (pLearnStart==0) pLearnStart=millis();
      pLearnAccum += s_p; pLearnN++;
      if (millis()-pLearnStart >= (unsigned long)(CFG_P_BASE_LEARN_S*1000.0f)) {
        pBaseline = (pLearnN>0) ? pLearnAccum/pLearnN : s_p;
        pBaselineReady = true;
        Serial.printf("[AUTO-P] baseline = %.3f kPa\n", pBaseline);
      }
    } else if (cs==0 && s_p > pBaseline) {
      pBaseline += (s_p - pBaseline) * CFG_P_BASE_EMA;
    }
    if (pBaselineReady)
      pressureSetpoint = (CFG_P_AUTO_FACTOR > 0.0f) ? pBaseline * CFG_P_AUTO_FACTOR
                                                     : pBaseline - CFG_P_AUTO_MARGIN;
  }
#endif

  // AUTO-ARUS (jika CFG_I_AUTO_BASE=1) 
#if CFG_I_AUTO_BASE
  if (sensorEnable[SENSOR_I] && currentOK && !manualStop && !emergencyStop &&
      !startupMode && pwmActual >= CFG_I_SPEED_MIN && cs == 0) {
    float ratio = s_i / (float)pwmActual;
    if (ratio >= iSlopeLearned * 0.97f) {
      iSlopeLearned += (ratio - iSlopeLearned) * CFG_I_BASE_EMA;
      iSlopeLearned = constrain(iSlopeLearned, CFG_I_SLOPE_MIN, CFG_I_SLOPE_MAX);
    }
  }
#endif

  // PID FGS 
  unsigned long now_ms = millis();
  float dt = constrain((now_ms-pid_last_ms)/1000.0f, CFG_DT_MIN, CFG_DT_MAX);
  pid_last_ms = now_ms;

  pidActiveMode = determinePIDMode();
  char m = modeChar();

  float sp_used=0, pv_used=0, db_used=0;
  switch (m) {
    case 'V': sp_used=CFG_VIB_SETPOINT; pv_used=vibration; db_used=CFG_DEADBAND_VIB; break;
    case 'P': sp_used=pressureSetpoint; pv_used=s_p; db_used=CFG_PID_DEADBAND; break;
    case 'I': {
      // Setpoint I dinamis berbasis state kavitasi (diskrit, langsung berlaku saat state berubah)
      float stateScale = (cs == 2) ? CFG_CAV_SEVERE_SCALE : (cs == 1) ? CFG_CAV_EARLY_SCALE : 1.00f;
      int   nomPct     = constrain(curSpeedTargetPct, CFG_I_SPEED_MIN, 100);
      int   effPct     = max(CFG_I_SPEED_MIN, (int)roundf(nomPct * stateScale));
      ispEffPct  = effPct;
      sp_used    = iSlopeLearned * effPct + CFG_I_MODEL_OFFSET;
      pv_used = s_i; db_used = CFG_DEADBAND_CUR; break; }
    default: break;
  }
  if (m != 'I') ispEffPct = curSpeedTargetPct;
  pidSetpoint = sp_used; pidPV = pv_used;

  bool pid_can_run = (m != '-') && !manualStop && !emergencyStop &&
                     sensorOK && !startupMode && !stabilizing && !testFixedMode;

  float pidOut = 0;
  if (pid_can_run) {
    float sp_c = sp_used, pv_c = pv_used;
    if (m == 'P') { sp_c = pv_used; pv_c = sp_used; }
    pidOut = computePID_FGS(sp_c, pv_c, cavIdx, m, db_used, dt);
  }

  if (testFixedMode && !manualStop && !emergencyStop && !startupMode && !stabilizing)
    pwmTarget = constrain(testHoldPwm, CFG_PWM_MIN, CFG_PWM_MAX);

  if (manualStop||emergencyStop) {
    pwmTarget=CFG_PWM_MIN; pid_integral=0; pid_deriv_filt=0;
  }

  g_vibration=vibration; g_current=s_i; g_pressure_kPa=s_p;
  g_cavitation=cavIdx; g_cavState=cs; g_pid_output=pidOut;

  // LOGGING 
  unsigned long effInterval = thermalThrottle ? logInterval*2 : logInterval;
  if (logging && (millis()-lastLog >= effInterval)) {
    lastLog = millis();
    logRow(g_vibration, g_current, g_pressure_kPa, pwmActual,
           g_cavitation, g_cavState, g_pid_output, sensorStatus);
  }

  // WEB CLIENT 
  handleClient();

  // AKTUATOR (cadence) 
  if (millis()-lastActuator > CFG_ACTUATOR_INTERVAL) {
    lastActuator = millis();
    if (relayBusy()) { /* tunggu */ }
    else if (manualStop||emergencyStop) {
      if (pwmActual>0) { scheduleRelay(true); pwmActual--; }
    } else if (startupMode) {
      if (pwmActual < CFG_PWM_STARTUP) { scheduleRelay(false); pwmActual++; }
      else {
        startupMode=false; stabilizing=true;
        startupDoneTime=millis(); pwmTarget=CFG_PWM_STARTUP;
        pid_integral=0; pid_prev_error=0; pid_deriv_filt=0;
        cavState=0; cavStatePrev=0; cav_pending=0; cavPendSince=millis(); cavStateEnterTime=millis();
        Serial.printf("[OK] Startup selesai. Mode=%s SP=%.3f\n",
                      pidActiveMode.c_str(), pidSetpoint);
      }
    } else if (stabilizing) {
      pwmTarget = CFG_PWM_STARTUP;
      if (millis()-startupDoneTime > CFG_STABILIZE_MS) {
        stabilizing = false;
        // Set settle timer: deteksi kavitasi baru aktif setelah PID
        // menaikkan kecepatan dari startup ke kisaran operasi.
        cavDetectEnableAt = millis() + CFG_CAV_DETECT_SETTLE_MS;
        // Bersihkan state machine agar tidak carry-over dari transien startup
        cavState=0; cavStatePrev=0; cav_pending=0;
        cavStateEnterTime=millis(); cavPendSince=millis(); cavPwmCap=CFG_PWM_MAX; cavRecoveryUntil=0;
        pid_integral=0; pid_deriv_filt=0;
        Serial.printf("[OK] Stabilisasi selesai. Deteksi aktif dalam %ds. Mode=%s PV=%.3f\n",
                      CFG_CAV_DETECT_SETTLE_MS/1000, pidActiveMode.c_str(), pidPV);
      }
    } else {
      if (pid_can_run) {
        pwmTarget = constrain(pwmTarget + pidToDelta(pidOut, cs), CFG_PWM_MIN, CFG_PWM_MAX);
        int _antijump = (cs == 2) ? CFG_DELTA_MAX_SEVERE : (cs == 1) ? CFG_DELTA_MAX_EARLY : CFG_DELTA_MAX_NORMAL;
        pwmTarget = constrain(pwmTarget, pwmActual - _antijump, pwmActual + _antijump);
      }
      // Mode I + kavitasi aktif: drive langsung ke target kecepatan efektif.
      // FGS mengecilkan gain PID → PID tidak bisa mengoreksi kecepatan sendiri;
      // override ini memastikan pompa benar-benar turun ke effPct.
      // effPwm TIDAK dibatasi cavPwmCap di sini — line 539 (pwmTarget > cavPwmCap)
      // menangani capping, sehingga jika kavitasi masih aktif di effPwm, cavPwmCap
      // terus turun dan pompa menyusul ke bawah effPwm hingga kavitasi hilang.
      if (pid_can_run && m == 'I' && ispEffPct < curSpeedTargetPct) {
        int effPwm = constrain((int)roundf(CFG_PWM_MAX * ispEffPct / 100.0f),
                                CFG_PWM_FLOW_MIN, CFG_PWM_MAX);
        pwmTarget = effPwm;
      }
      // Floor cavPwmCap selalu di CFG_PWM_FLOW_MIN (bukan di effPwm).
      // Jika kavitasi tidak hilang di effPwm, cavPwmCap terus turun sehingga
      // pompa dapat turun lebih jauh — mencegah deadlock stuck-at-effPwm.
      // Hunting dicegah oleh CFG_CAV_RECOVERY_HOLD_MS (8 detik), bukan floor ini.
      if      (cs == 2) cavPwmCap = max(CFG_PWM_FLOW_MIN, cavPwmCap - CFG_CAV_CAP_SEVERE);
      else if (cs == 1) cavPwmCap = max(CFG_PWM_FLOW_MIN, cavPwmCap - CFG_CAV_CAP_EARLY);
      else              cavPwmCap = min(CFG_PWM_MAX, cavPwmCap + CFG_CAV_CAP_RECOVER);
      // Recovery hold: saat kavitasi baru hilang, tahan kecepatan dan reset integrator
      if (cs == 0 && cavStatePrev > 0) {
        cavRecoveryUntil = millis() + CFG_CAV_RECOVERY_HOLD_MS;
        pid_integral = 0; pid_deriv_filt = 0;
        Serial.printf("[REC] Recovery hold %ds. PWM=%d cap=%d\n",
                      CFG_CAV_RECOVERY_HOLD_MS/1000, pwmActual, cavPwmCap);
      }
      if (cs > cavStatePrev) { pid_integral = 0; pid_deriv_filt = 0; }
      cavStatePrev = cs;
      if (!testFixedMode && pwmTarget > cavPwmCap) pwmTarget = cavPwmCap;
      int lim = applyRateLimit(pwmTarget, pwmActual);
      // Blokir kenaikan kecepatan selama recovery hold
      if (millis() < cavRecoveryUntil && lim > pwmActual) lim = pwmActual;
      if      (lim > pwmActual) { scheduleRelay(false); pwmActual++; }
      else if (lim < pwmActual) { scheduleRelay(true);  pwmActual--; }
    }
  }

  readChipTemp();
  yield();
}
