#pragma once

//  config.h — Konfigurasi Smart Pump (ESP32-S3)

// [A] WiFi 
#define CFG_WIFI_SSID            "NERO"
#define CFG_WIFI_PASSWORD        "12345678"

// [B] Pin Hardware 
#define CFG_PIN_SDA              7
#define CFG_PIN_SCL              6
#define CFG_PIN_ACS              5
#define CFG_PIN_PRESSURE         4
#define CFG_PIN_RELAY_UP         16
#define CFG_PIN_RELAY_DOWN       17

// [C] Sensor Tekanan WPT-83G 
#define CFG_P_VMIN               0.5f     // V tegangan sensor saat P=0
#define CFG_P_VMAX               4.5f     // V tegangan sensor saat P=Fullscale
#define CFG_P_FULLSCALE         12.0f     // bar full scale sensor
#define CFG_VDIV_FACTOR          2.0f     // faktor pembagi tegangan ADC
#define CFG_P_MIN_KPA           -100.0f   // batas validasi bawah (kPa)
#define CFG_P_MAX_KPA           1250.0f   // batas validasi atas (kPa)
#define CFG_P_OVERSAMPLE         8        // rata-rata ADC oversampling
#define CFG_P_OPERATIONAL_MAX   30.0f     // kPa max tekanan isap operasi normal (referensi MF)

// [C2] Kalibrasi Tekanan 
#define CFG_PCAL_ZERO_DEFAULT    0.0f
#define CFG_PCAL_GAIN_DEFAULT    1.0f
#define CFG_PCAL_SAMPLES         100
#define CFG_PCAL_SAMPLE_DELAY    20      // ms antar sampel kalibrasi
#define CFG_PCAL_ZERO_MIN       -20.0f   // batas aman offset nol (kPa)
#define CFG_PCAL_ZERO_MAX        20.0f
#define CFG_PCAL_GAIN_MIN        0.5f
#define CFG_PCAL_GAIN_MAX        2.0f

// [D] Sensor Arus ACS712 
#define CFG_ACS_SENSITIVITY      0.185f   // V/A (ACS712-5A)
#define CFG_I_MIN_VALID         -1.0f     // batas validasi arus (A)
#define CFG_I_MAX_VALID          6.0f
#define CFG_I_CLAMP_LOW          0.0f     // klem bawah arus terukur
#define CFG_I_CLAMP_HIGH         5.0f     // klem atas arus terukur
#define CFG_I_DRIFT_THRESH       5        // iterasi negatif sebelum auto-koreksi
#define CFG_I_DEADZONE           0.05f    // A deadzone arus
// Model linear arus vs PWM: I_exp = SLOPE * PWM% + OFFSET
#define CFG_I_MODEL_SLOPE        0.01438f
#define CFG_I_MODEL_OFFSET       0.002f
#define CFG_I_DEFICIT_EARLY      0.12f    // 12% defisit → onset kavitasi
#define CFG_I_DEFICIT_SEVERE     0.25f    // 25% defisit → kavitasi berat

// [E] Sensor Vibrasi MPU6050 
#define CFG_MPU_ACCEL_RANGE      MPU6050_RANGE_4_G
#define CFG_MPU_GYRO_RANGE       MPU6050_RANGE_500_DEG
#define CFG_MPU_DLPF             MPU6050_BAND_21_HZ
#define CFG_MPU_CALIB_SAMPLES    500
#define CFG_VIB_GRAVITY          9.81f    // m/s² gravitasi
#define CFG_VIB_MAX_VALID        40.0f    // m/s² batas spike valid
#define CFG_VIB_PERSIST_N        3        // iterasi spike sebelum error
#define CFG_VIB_HAMPEL_N         7        // jendela filter Hampel
#define CFG_VIB_HAMPEL_K         3.0f     // threshold Hampel (×MAD)
#define CFG_VIB_MEDIAN_N         5        // jendela filter median

// [F] Filter EMA 
#define CFG_EMA_ALPHA_ACCEL      0.25f    // alpha EMA akselerometer
#define CFG_EMA_ALPHA_CURR       0.12f    // alpha EMA arus
#define CFG_EMA_ALPHA_PRES       0.08f    // alpha EMA tekanan
#define CFG_EMA_ALPHA_VIB_BASE   0.005f   // alpha EMA baseline vibrasi (saat pompa mati)

// [G] Moving Average Tekanan 
#define CFG_MA_SIZE              20       // ukuran jendela moving average

// [H] Kontrol Pompa & PWM 
#define CFG_PRESSURE_REF         0.25f    // kPa setpoint tekanan isap (was 0.12 — dinaikkan sesuai data)
#define CFG_P_AUTO               0        // 1=setpoint tekanan otomatis (belajar baseline)
#define CFG_P_BASE_LEARN_S       3.0f     // detik jendela belajar baseline tekanan
#define CFG_P_BASE_EMA           0.02f    // laju re-learn baseline tekanan
#define CFG_P_AUTO_FACTOR        0.70f    // ambang = baseline × faktor
#define CFG_P_AUTO_MARGIN        0.30f    // ambang = baseline − margin (bila FACTOR≤0)
#define CFG_PWM_MIN              0
#define CFG_PWM_MAX              100
#define CFG_PWM_STARTUP          20       // PWM fase startup ramp
#define CFG_PWM_RATE_UP          1        // langkah maksimal naik per aktuasi
#define CFG_PWM_RATE_DOWN        2        // langkah maksimal turun per aktuasi
#define CFG_PID_DEADBAND         0.05f    // kPa deadband PID tekanan
#define CFG_STABILIZE_MS         4000     // ms fase stabilisasi pasca startup
#define CFG_ACTUATOR_INTERVAL    400      // ms periode aktuasi relay (harus > PULSE+SETTLE_UP agar relay selalu selesai sebelum cek berikutnya)
#define CFG_PWM_FLOW_MIN         20       // PWM minimum agar dianggap ada aliran

// [I] PID Gains — Mode Tekanan 
#define CFG_KP_NORMAL            5.0f
#define CFG_KI_NORMAL            0.5f
#define CFG_KD_NORMAL            0.2f
#define CFG_KP_EARLY             2.0f
#define CFG_KI_EARLY             0.0f
#define CFG_KD_EARLY             0.5f

// [I1] PID Gains — Mode Vibrasi 
#define CFG_VIB_SETPOINT         1.0f     // m/s² setpoint vibrasi (was 2.0 — disesuaikan range MFV baru)
#define CFG_KP_VIB               8.0f
#define CFG_DEADBAND_VIB         0.15f    // m/s² deadband vibrasi

// [I2] PID Gains — Mode Arus 
#define CFG_CUR_SETPOINT         0.30f    // A (referensi; aktifnya adaptif)
#define CFG_I_SPEED_TARGET_DEFAULT 50     // % kecepatan target default
#define CFG_I_SPEED_MIN            20     // % lantai kecepatan
// Faktor reduksi kecepatan saat kavitasi terdeteksi.
#define CFG_CAV_EARLY_SCALE      0.70f    // reduksi kecepatan saat early kavitasi (was hardcoded 0.80)
#define CFG_CAV_SEVERE_SCALE     0.50f    // reduksi kecepatan saat severe kavitasi (was hardcoded 0.60)
#define CFG_I_AUTO_BASE          0        // 1=slope arus belajar otomatis
#define CFG_I_BASE_EMA           0.003f   // laju adaptasi slope arus
#define CFG_I_SLOPE_MIN          0.008f   // batas aman slope hasil belajar
#define CFG_I_SLOPE_MAX          0.025f
#define CFG_KP_CUR               100.0f
#define CFG_DEADBAND_CUR         0.02f    // A deadband arus

// [I3] Anti-windup & Filter PID 
#define CFG_INTEGRAL_MAX         2.0f
#define CFG_DERIV_ALPHA          0.15f
#define CFG_DT_MIN               0.001f   // s dt minimum
#define CFG_DT_MAX               0.5f     // s dt maksimum

// [I4] Plafon PWM Kavitasi (supervisori) 
#define CFG_CAV_CAP_SEVERE       4        // PWM cap turun per aktuasi saat Severe
#define CFG_CAV_CAP_EARLY        3        // PWM cap turun per aktuasi saat Early (was 1 — respons lebih cepat)
#define CFG_CAV_CAP_RECOVER      2        // PWM cap naik per aktuasi saat Normal (was 1 — dinaikkan agar kecepatan pulih lebih cepat)
#define CFG_DELTA_MAX_NORMAL     2        // langkah PID max saat Normal
#define CFG_DELTA_MAX_EARLY      1        // langkah PID max saat Early
#define CFG_DELTA_MAX_SEVERE     4        // langkah PID max saat Severe
#define CFG_CAV_RECOVERY_HOLD_MS 4000     // ms tahan kecepatan rendah setelah kavitasi hilang (was 8000 — dikurangi agar kecepatan pulih lebih cepat)

// [I5] Fuzzy Gain Scheduling (FGS) — batas CavIdx 
#define FGS_CAVIDX_A             0.00f    // CavIdx = Normal
#define FGS_CAVIDX_B             0.70f    // CavIdx = batas Early
#define FGS_CAVIDX_C             1.15f    // CavIdx = batas Severe
#define FGS_CAVIDX_D             1.50f    // CavIdx = Severe penuh

// FGS — Gain Mode Vibrasi
#define FGS_V_KP_A               0.8f
#define FGS_V_KI_A               0.10f
#define FGS_V_KD_A               0.0f
#define FGS_V_KP_B               0.4f
#define FGS_V_KI_B               0.05f
#define FGS_V_KD_B               0.0f
#define FGS_V_KP_C               0.0f
#define FGS_V_KI_C               0.0f
#define FGS_V_KD_C               0.0f

// FGS — Gain Mode Tekanan
#define FGS_P_KP_A               3.0f
#define FGS_P_KI_A               0.30f
#define FGS_P_KD_A               0.0f
#define FGS_P_KP_B               1.5f
#define FGS_P_KI_B               0.10f
#define FGS_P_KD_B               0.0f
#define FGS_P_KP_C               0.0f
#define FGS_P_KI_C               0.0f
#define FGS_P_KD_C               0.0f

// FGS — Gain Mode Arus
#define FGS_I_KP_A               6.0f
#define FGS_I_KI_A               0.80f
#define FGS_I_KD_A               0.10f
#define FGS_I_KP_B               3.0f
#define FGS_I_KI_B               0.30f
#define FGS_I_KD_B               0.05f
#define FGS_I_KP_C               0.0f
#define FGS_I_KI_C               0.0f
#define FGS_I_KD_C               0.0f

// [J] Fuzzy MF — Tekanan P (kPa) 
#define CFG_MFP_LOW_FULL         0.10f   // P ≤ SEVERE → LOW=1.0  (was 0.0)
#define CFG_MFP_LOW_ZERO         0.40f   // P ≥ EARLY  → LOW=0.0  (was 8.0)
#define CFG_MFP_NORM_L0          0.10f   // NORM mulai dari SEVERE (was 5.0)
#define CFG_MFP_NORM_LPEAK       0.25f   // puncak = (SEVERE+EARLY)/2 (was 15.0)
#define CFG_MFP_NORM_RPEAK       0.40f   // NORM berakhir di EARLY  (was 25.0)
#define CFG_MFP_HIGH_ZERO        0.10f   // HIGH naik mulai SEVERE  (was 22.0)
#define CFG_MFP_HIGH_FULL        0.40f   // HIGH=1.0 sejak EARLY    (was 30.0)

// [K] Fuzzy MF — Vibrasi V (m/s²) 
// Dikalibrasi ke pompa riil: idle ~0.2-0.5 m/s², kavitasi onset ~0.8-1.2 m/s², kavitasi penuh ~2-3 m/s²
#define CFG_MFV_LOW_FULL         0.20f    // ≤ ini: LOW penuh   (was 0.30)
#define CFG_MFV_LOW_ZERO         0.60f    // ≥ ini: LOW nol     (was 0.80)
#define CFG_MFV_MED_L0           0.30f    // mulai MED          (was 0.50)
#define CFG_MFV_MED_LPEAK        0.80f    // puncak MED         (was 2.00)
#define CFG_MFV_MED_RPEAK        1.80f    // puncak MED kanan   (was 4.00)
#define CFG_MFV_HIGH_ZERO        1.20f    // mulai HIGH
#define CFG_MFV_HIGH_FULL        2.00f    // HIGH penuh (was 3.00 — terlalu tinggi, muV[2]≈0.17 saat kavitasi aktual 1.5 m/s²)

// [L] Fuzzy MF — Arus I (Ampere) 
#define CFG_MFI_LOW_FULL         0.50f
#define CFG_MFI_LOW_ZERO         0.90f
#define CFG_MFI_NORM_L0          0.70f
#define CFG_MFI_NORM_LPEAK       1.30f
#define CFG_MFI_NORM_RPEAK       1.70f
#define CFG_MFI_HIGH_ZERO        1.50f
#define CFG_MFI_HIGH_FULL        2.00f

// [M] Hysteresis State Kavitasi 
#define CFG_CAV_0_TO_1           0.65f   // CavIdx Normal→Early  
#define CFG_CAV_1_TO_0           0.45f   // CavIdx Early→Normal  
#define CFG_CAV_1_TO_2           1.15f   // CavIdx Early→Severe  
#define CFG_CAV_2_TO_1           0.95f   // CavIdx Severe→Early  
#define CFG_CAV_HOLD_MS          800     // ms minimum antar transisi
#define CFG_CAV_CONFIRM_UP_MS    1500    // ms konfirmasi naik (was 800 — diperpanjang agar tidak trip dari sinyal transien/noise)
#define CFG_CAV_CONFIRM_DOWN_MS  1500    // ms konfirmasi turun (lambat, hati-hati)
// Waktu tunggu setelah stabilisasi sebelum deteksi kavitasi diaktifkan.
// Selama waktu ini PID menaikkan kecepatan dari 20%→target. Saat deteksi
#define CFG_CAV_DETECT_SETTLE_MS 8000   // ms (≈ 20 langkah × 400 ms = PWM naik ~20%)

// [N] Relay Timing 
// Total UP   cycle = PULSE + SETTLE_UP   = 80+260 = 340ms < ACTUATOR_INTERVAL(400ms) ✓
// Total DOWN cycle = PULSE + SETTLE_DOWN = 80+200 = 280ms < ACTUATOR_INTERVAL(400ms) ✓
// Jaminan: relay selalu selesai sebelum aktuasi berikutnya → tidak ada pulsa yang tertinggal
#define CFG_RELAY_PULSE_MS       80      // ms lebar pulsa relay
#define CFG_RELAY_SETTLE_UP_MS   260     // ms settle setelah naik (motor kontroler butuh waktu proses akselerasi)
#define CFG_RELAY_SETTLE_DOWN_MS 200     // ms settle setelah turun (sedikit lebih cepat untuk respons kavitasi)

// [O] Thermal Management ESP32 
#define CFG_TEMP_READ_MS         5000
#define CFG_TEMP_THROTTLE_C      80.0f   // °C mulai throttle
#define CFG_TEMP_RECOVER_C       75.0f   // °C pulih dari throttle
#define CFG_CPU_NORMAL_MHZ       160
#define CFG_CPU_THROTTLE_MHZ     80

// [P] Sistem & Watchdog 
#define CFG_LOOP_MIN_US          1000    // us periode loop minimum
#define CFG_WDT_TIMEOUT_MS       5000
#define CFG_SERIAL_BAUD          115200
#define CFG_I2C_CLOCK_HZ         400000
#define CFG_CALIB_SAMPLES        500
#define CFG_CSV_CHUNK            5

// [Q] CSV Logger 
#define CFG_CSV_BUF_SIZE         2000    // baris maksimal buffer log
#define CFG_LOG_INTERVAL_MS      500     // ms interval log default

// [R] Sensor Default Enable 
#define CFG_DEFAULT_EN_P         true
#define CFG_DEFAULT_EN_V         true
#define CFG_DEFAULT_EN_I         true

// [S] Konfigurasi Sensor Tekanan 
#define CFG_P_SENSOR_VACUUM      0        // 0=gauge WPT-83G; 1=compound/vacuum
#define CFG_PV_VMIN              0.5f
#define CFG_PV_VMAX              4.5f
#define CFG_PV_KPA_MIN          -100.0f
#define CFG_PV_KPA_MAX           0.0f
// Ambang deteksi kavitasi dari tekanan isap (gauge WPT-83G)
// Dikalibrasi dari data operasi:
#define CFG_P_SUCTION_EARLY      0.40f   // kPa — onset kavitasi  
#define CFG_P_SUCTION_SEVERE     0.10f   // kPa — kavitasi berat  
// Koroborasi: alarm butuh konfirmasi tekanan ATAU arus, bukan vibrasi saja.
// 0.25 → cukup 25% sinyal untuk lolos; lebih sensitif dari 0.40 sebelumnya.
#define CFG_CAV_CORROB_MIN       0.25f
// PWM referensi kalibrasi ambang tekanan (P_SUCTION_EARLY/SEVERE dikalibrasi pada kecepatan ini).
// Ambang diskalakan linier: p_eff = p_ref * (pwmActual / CFG_PWM_P_THRESH_REF).
// Pada kecepatan rendah tekanan isap secara alami lebih rendah — threshold mengikuti.
#define CFG_PWM_P_THRESH_REF     100
// Prioritas PID (ditetapkan program): I=arus (paling stabil untuk speed control) → V → P
// Prioritas DETEKSI (Fuzzy): P(0.40) > V(0.40) > I(0.20) — tekanan isap paling langsung
#define CFG_CTRL_PRIMARY         'I'
// Mode uji PWM tetap: 1=kunci PWM ke testHoldPwm, PID off
#define CFG_TEST_FIXED_PWM       0

// [T] Sensor IDs 
#define SENSOR_P    0
#define SENSOR_V    1
#define SENSOR_I    2
#define SENSOR_COUNT 3
