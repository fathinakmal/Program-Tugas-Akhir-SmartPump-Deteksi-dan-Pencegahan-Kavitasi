[BLOK-7-BAB-41-50-Pembedahan-Program-Firmware.md](https://github.com/user-attachments/files/29321578/BLOK-7-BAB-41-50-Pembedahan-Program-Firmware.md)
# BLOK VII — PEMBEDAHAN PROGRAM FIRMWARE (BAB 41–50)

> Buku Modul SmartPump — Blok paling teknis. Di sini Anda membedah setiap baris firmware C++ yang berjalan di ESP32-S3: dari `config.h` (konstanta), `types.h` (struktur data), tiga modul sensor, mesin fuzzy, kontroler PID, aktuator relay, hingga `loop()` utama yang menyatukan semuanya. Setelah blok ini, Anda dapat menunjuk baris kode mana pun di layar dan menjelaskan mengapa baris itu ada.

**Peta file dan ketergantungan:**
```
config.h ────────────────────────────────────────────────┐
types.h  ────────────────────────────────────────────────┤
system_utils.h  (EMA, ADC, suhu, CSV logger)             ├──► smart_pumpp.ino
actuator.h      (mesin relay 5-state)                    │      setup() + loop()
sensor_pressure.h  (WPT-83G, MA, kalibrasi)              │
sensor_current.h   (ACS712, defisit, kalibrasi)          │
sensor_vibration.h (MPU6050, Hampel, median, kalibrasi)  │
fuzzy.h         (MF, 27 aturan, centroid, FGS)           │
pid_control.h   (mode, PID FGS, state machine, limiter)  │
web_html.h      (string HTML dashboard)                  │
web_server.h    (HTTP handler non-blocking)             ─┘
```

---

# BAB 41 — ARSITEKTUR PROGRAM: SINGLE-CORE NON-BLOCKING STATE MACHINE

## 41.1 Filosofi Desain

Firmware SmartPump berprinsip **satu loop, tidak ada `delay()`**. Artinya:

- Semua tugas (baca sensor, hitung fuzzy, PID, web, relay) dijalankan bergantian dalam **satu iterasi loop**.
- Tidak ada satu pun tugas yang boleh "memblokir" (menunggu dengan cara yang menghentikan loop).
- Relay dikendalikan oleh **state machine non-blocking** — bukan `digitalWrite` + `delay`.
- Web server dilayani **satu permintaan per iterasi loop** — bukan `while(client connected)`.

**Mengapa?** ESP32 memiliki Watchdog Timer (WDT). Jika loop tidak memanggil `esp_task_wdt_reset()` dalam `CFG_WDT_TIMEOUT_MS` (5000 ms), ESP32 auto-reset. Sebelumnya pakai `delay()` → TWDT trip → restart berulang. Solusinya: arsitektur non-blocking.

**Akibat desain ini:** tidak ada variabel lokal besar, tidak ada loop tunggu-sampai-selesai, semua state tersimpan di variabel global agar bisa diteruskan antar iterasi.

## 41.2 Alur Setiap Iterasi Loop

```
loop() dipanggil ~1000×/detik
│
├─ 1. Rate limiter (paksa ≥1ms per iterasi, hindari busy-spin)
├─ 2. WDT reset (beri tahu watchdog "saya masih hidup")
├─ 3. Hitung throughput (loops/detik, MB/s)
├─ 4. tickRelay() — lanjutkan state machine relay jika sedang berjalan
├─ 5. readMPU() — baca akselerometer tiap iterasi (I²C cepat)
├─ 6. Baca ADC (arus + tekanan) — hanya tiap 2 iterasi (alternasi)
├─ 7. EMA filter — perhalus semua sinyal
├─ 8. Gravity removal + Hampel + median (untuk getaran dinamis)
├─ 9. Validasi sensor + set sensorStatus
├─ 10. fuzzyMamdaniAdaptive() — hitung indeks K
├─ 11. updateCavState() — mesin keadaan hysteresis
├─ 12. computePID_FGS() — hitung koreksi PWM
├─ 13. logRow() — catat ke buffer CSV
├─ 14. handleClient() — layani satu HTTP request
└─ 15. Aktuasi (tiap 400ms) — naikkan/turunkan PWM, update cavPwmCap
```

## 41.3 Fase Operasi Pompa

Pompa tidak langsung beroperasi penuh. Ada **tiga fase berurutan sebelum deteksi aktif**:

```
[BOOT] → [STARTUP] → [STABILIZING] → [NORMAL OPERATION]
           PWM 0→20     tunggu 4s       deteksi aktif
           (ramp)        (settle)        setelah 8s lagi
```

| Flag | Arti | Pengaruh |
|---|---|---|
| `manualStop=true` | pompa dimatikan manual | PWM diturunkan ke 0, PID off |
| `startupMode=true` | ramp naik ke PWM=20 | hanya naik 1 langkah per 400ms |
| `stabilizing=true` | tunggu 4000ms | PID dihitung tapi tidak diaktuasi |
| Normal | semua false | PID+fuzzy aktif, deteksi aktif setelah 8s |

**Mengapa ada settle timer 8 detik?** Saat pompa baru nyala di PWM=20, tekanan dan arus butuh beberapa detik untuk stabil. Jika deteksi langsung aktif, nilai transien akan memicu false positive.

---

# BAB 42 — `config.h`: SEMUA PARAMETER DALAM SATU FILE

## 42.1 Filosofi: Satu Tempat untuk Semua Angka

`config.h` adalah satu-satunya file yang boleh diubah untuk mengkonfigurasi sistem. Tidak ada "magic number" tersebar di dalam kode. Setiap `#define` memiliki nama deskriptif dan komentar penjelasan.

## 42.2 Grup Konfigurasi [A]–[T]

### [A] WiFi
```cpp
#define CFG_WIFI_SSID     "NERO"
#define CFG_WIFI_PASSWORD "12345678"
```
ESP32 beroperasi sebagai **Access Point (Soft-AP)** — ia adalah router Wi-Fi mandiri, tidak perlu jaringan luar. Semua perangkat terhubung langsung ke SSID "NERO".

### [B] Pin Hardware
```cpp
#define CFG_PIN_SDA     7   // I²C data  (MPU6050)
#define CFG_PIN_SCL     6   // I²C clock (MPU6050)
#define CFG_PIN_ACS     5   // ADC arus ACS712
#define CFG_PIN_PRESSURE 4  // ADC tekanan WPT-83G
#define CFG_PIN_RELAY_UP   16  // relay naik PWM
#define CFG_PIN_RELAY_DOWN 17  // relay turun PWM
```
**Mengapa GPIO 4–7?** GPIO 35/36/37 di ESP32-S3 terpakai internal untuk PSRAM (flash eksternal). Menggunakannya untuk ADC menyebabkan pembacaan acak. Ini adalah jebakan hardware yang umum.

### [C] Sensor Tekanan WPT-83G
```cpp
#define CFG_P_VMIN        0.5f   // V minimum sensor (tekanan nol)
#define CFG_P_VMAX        4.5f   // V maksimum sensor (tekanan penuh)
#define CFG_P_FULLSCALE  12.0f   // bar skala penuh
#define CFG_VDIV_FACTOR   2.0f   // pembagi tegangan R1=R2 → V_ADC = V_sensor/2
#define CFG_P_OVERSAMPLE  8      // rata-rata 8 pembacaan ADC
```
**Mengapa pembagi tegangan?** Sensor bertegangan 0.5–4.5V, ADC ESP32 hanya aman sampai 3.3V. Pembagi 1:1 (R1=R2=10kΩ) menghasilkan V_ADC = V_sensor / 2, sehingga maks 2.25V — aman.

### [F] EMA Filter
```cpp
#define CFG_EMA_ALPHA_ACCEL  0.25f  // akselerasi (respons cepat)
#define CFG_EMA_ALPHA_CURR   0.12f  // arus (sedang)
#define CFG_EMA_ALPHA_PRES   0.08f  // tekanan (lambat, isyarat kecil)
```
Semakin kecil alpha → semakin mulus tapi lebih lambat. Tekanan perlu alpha kecil karena isyaratnya sangat kecil (0–0.3 kPa) sehingga rawan gangguan elektronik.

### [H] Kontrol Pompa
```cpp
#define CFG_PRESSURE_REF     0.25f   // kPa setpoint tekanan isap
#define CFG_PWM_RATE_UP      1       // maks +1 PWM per aktuasi
#define CFG_PWM_RATE_DOWN    2       // maks -2 PWM per aktuasi
#define CFG_PID_DEADBAND     0.05f   // kPa — tidak koreksi jika error < ini
#define CFG_ACTUATOR_INTERVAL 400    // ms periode relay
```
**Rate limiter asimetris (+1/-2):** turun lebih cepat dari naik. Saat kavitasi, penurunan kecepatan harus segera; kenaikan kembali harus pelan agar kavitasi tidak kambuh.

### [I5] Fuzzy Gain Scheduling (FGS) Breakpoint
```cpp
#define FGS_CAVIDX_A  0.00f  // K = Normal
#define FGS_CAVIDX_B  0.70f  // K = batas Early
#define FGS_CAVIDX_C  1.15f  // K = batas Severe
#define FGS_CAVIDX_D  1.50f  // K = Severe penuh
```
Gain PID diinterpolasi linear antara titik-titik ini. Saat K ≥ 1.15, semua gain = 0 (PID mati, pompa dikendalikan supervisori).

### [M] Hysteresis State Kavitasi
```cpp
#define CFG_CAV_0_TO_1  0.65f  // Normal→Early: K harus melampaui ini
#define CFG_CAV_1_TO_0  0.45f  // Early→Normal: K harus turun di bawah ini
#define CFG_CAV_1_TO_2  1.15f  // Early→Severe
#define CFG_CAV_2_TO_1  0.95f  // Severe→Early
#define CFG_CAV_HOLD_MS       800   // ms minimum antar transisi
#define CFG_CAV_CONFIRM_UP_MS 1500  // ms K harus bertahan sebelum naik state
```
**Mengapa ambang masuk ≠ keluar?** Ini **hysteresis** — mencegah chattering. Jika K berfluktuasi di 0.65, tanpa hysteresis state akan berkedip Normal↔Early ratusan kali per detik.

### [N] Relay Timing
```cpp
#define CFG_RELAY_PULSE_MS       80   // lebar pulsa HIGH relay
#define CFG_RELAY_SETTLE_UP_MS  260   // tunggu setelah naik
#define CFG_RELAY_SETTLE_DOWN_MS 200  // tunggu setelah turun
```
Total siklus naik = 80+260 = 340ms < 400ms (ACTUATOR_INTERVAL) → relay **pasti selesai** sebelum aktuasi berikutnya. Ini jaminan formal yang harus dipenuhi.

---

# BAB 43 — `types.h`: STRUKTUR DATA

```cpp
struct DynGains { float Kp; float Ki; float Kd; };
```
Digunakan `fuzzyGainSchedule()` untuk mengembalikan tiga gain sekaligus dari satu panggilan fungsi.

```cpp
struct LogRow {
  unsigned long time_ms;
  float vibration, current, pressure_kPa, cavIdx, pidOut;
  int   pwm, cavState;
  char  sensorSt[24];
  char  fuzzyMode[8];
  char  pidMode[4];
  uint8_t enableMask;
};
```
Satu baris log CSV. Buffer `csvBuf[2000]` (2000 baris) disimpan di RAM ESP32-S3 yang memiliki PSRAM 8MB — tidak ada file system, tidak ada SD card. Data diunduh via HTTP `/log`.

```cpp
struct FuzzyRule {
  uint8_t p_idx;  // 0=LOW 1=MED 2=HIGH
  uint8_t v_idx;
  uint8_t i_idx;
  uint8_t out_idx; // 0=NORMAL 1=EARLY 2=SEVERE
};
```
Satu aturan fuzzy dikodekan dalam 4 byte. Array `RULES[27]` berisi semua 27 aturan.

---

# BAB 44 — `system_utils.h`: UTILITAS UMUM

## 44.1 Fungsi EMA (Exponential Moving Average)

```cpp
float smooth(float prev, float val, float alpha) {
  return alpha * val + (1.0f - alpha) * prev;
}
```
**EMA** adalah filter low-pass paling efisien: hanya 1 perkalian, 1 tambah, tidak butuh buffer. Formula: `s[k] = α·x[k] + (1-α)·s[k-1]`. Alpha besar → lebih mengikuti sinyal baru; alpha kecil → lebih mempertahankan nilai lama.

## 44.2 ADC Oversampling

```cpp
int analogReadOversampled(int pin, int n) {
  long sum = 0;
  for (int i = 0; i < n; i++) sum += analogRead(pin);
  return (int)(sum / n);
}
```
ADC 12-bit ESP32 punya noise ~1–2 bit. Rata-rata 8 sampel (`CFG_P_OVERSAMPLE=8`) mengurangi noise efektif karena noise acak saling menghapus. Resolusi efektif meningkat.

## 44.3 Thermal Management

```cpp
void readChipTemp() {
  if (!tempSensorOK || millis()-lastTempRead < CFG_TEMP_READ_MS) return;
  // ...
  if (chipTemp >= CFG_TEMP_THROTTLE_C && !thermalThrottle) {
    thermalThrottle = true;
    setCpuFrequencyMhz(80);        // 160→80 MHz
    WiFi.setTxPower(WIFI_POWER_11dBm);  // kurangi daya WiFi
  } else if (chipTemp < CFG_TEMP_RECOVER_C && thermalThrottle) {
    thermalThrottle = false;
    setCpuFrequencyMhz(160);
    WiFi.setTxPower(WIFI_POWER_17dBm);
  }
}
```
Dibaca tiap 5 detik (`CFG_TEMP_READ_MS=5000`). Hysteresis throttle/recover (80°C/75°C) mencegah toggling frekuensi yang berlebihan. Saat throttle, interval log juga digandakan agar beban CPU berkurang.

## 44.4 Logger CSV (Ring Buffer)

```cpp
void logRow(...) {
  LogRow& r = csvBuf[csvHead];
  r.time_ms = millis();
  // ... isi field ...
  csvHead = (csvHead + 1) % CFG_CSV_BUF_SIZE;  // pointer maju
  if (csvCount < CFG_CSV_BUF_SIZE) csvCount++;
}
```
**Ring buffer** (circular buffer): `csvHead` adalah pointer tulis yang berputar. Saat penuh (2000 baris), data terlama ditimpa. Tidak ada alokasi dinamis (`new`/`malloc`) → tidak ada fragmentasi heap. Inilah cara log dapat berjalan 2 jam tanpa memory leak.

## 44.5 updateFuzzyMode() dan Bobot Sensor

```cpp
void updateFuzzyWeights() {
  float wV = eV ? 0.40f : 0.0f;
  float wP = eP ? 0.40f : 0.0f;
  float wI = eI ? 0.20f : 0.0f;
  float sum = wV + wP + wI;
  fw_V = wV/sum; fw_P = wP/sum; fw_I = wI/sum;
}
```
Bobot direnormalisasi agar selalu berjumlah 1.0 meskipun ada sensor yang dinonaktifkan. Nilai bobot ini ditampilkan di dashboard web sebagai informasi, bukan dipakai langsung dalam komputasi fuzzy.

---

# BAB 45 — `sensor_pressure.h`: SENSOR TEKANAN WPT-83G

## 45.1 Konversi ADC → kPa

```cpp
float computePressureRaw_kPa(int adc_p) {
  float v_adc    = (adc_p / 4095.0f) * 3.3f;      // ADC → tegangan di pin
  float v_sensor = v_adc * CFG_VDIV_FACTOR;        // kompensasi pembagi (×2)
  float p_bar = (v_sensor - CFG_P_VMIN) /
                (CFG_P_VMAX - CFG_P_VMIN) * CFG_P_FULLSCALE;  // bar
  return p_bar * 100.0f;  // bar → kPa (×100)
}
```

**Penurunan langkah demi langkah:**
1. `v_adc = (adc/4095) × 3.3` → tegangan di pin ADC (0–3.3V)
2. `v_sensor = v_adc × 2` → tegangan asli sensor (0–4.5V, kompensasi pembagi)
3. `p_bar = (v_sensor - 0.5) / (4.5-0.5) × 12` → tekanan dalam bar
4. `p_kPa = p_bar × 100` → konversi ke kPa

**Contoh:** ADC=1000 → v_adc=0.807V → v_sensor=1.613V → p_bar=(1.613-0.5)/4×12=3.34 bar → p_kPa=334 kPa. (Nilai sangat tinggi karena sensor 12 bar dipakai di rentang 0–0.3 kPa; offset kalibrasi akan memperbaiki ini.)

## 45.2 Kalibrasi (Zero + Span)

```cpp
float applyPressureCalibration(float p_raw_kPa) {
  return (p_raw_kPa - p_zero_offset_kPa) * p_gain;
}
```
- `p_zero_offset_kPa`: nilai ADC saat tekanan 0 (dikalibrasi dengan pompa mati, sensor terbuka atmosfer)
- `p_gain`: faktor skala jika ada referensi tekanan yang diketahui

Kalibrasi disimpan di **NVS (Non-Volatile Storage)** — memori flash yang bertahan restart. Jadi kalibrasi hanya perlu dilakukan sekali, lalu tersimpan permanen.

## 45.3 Moving Average 20 Sampel

```cpp
float movingAvgPressure(float v) {
  ma_sum -= ma_buf[ma_idx];   // hapus nilai lama dari sum
  ma_buf[ma_idx] = v;         // simpan nilai baru
  ma_sum += v;                // tambahkan ke sum
  ma_idx = (ma_idx+1) % CFG_MA_SIZE;
  int n = ma_full ? CFG_MA_SIZE : ma_idx;
  return ma_sum / n;          // rata-rata efisien O(1)
}
```
**Moving average efisien:** tidak menghitung ulang semua 20 nilai. Cukup kurangi nilai yang keluar, tambahkan nilai baru, bagi. O(1) per iterasi. Ini penting karena loop jalan 1000×/detik.

Mengapa moving average untuk tekanan tapi EMA untuk getaran? Tekanan lebih stabil (sinyal DC) → MA memberikan rata-rata yang jelas. Getaran lebih dinamis → EMA lebih responsif.

---

# BAB 46 — `sensor_current.h`: SENSOR ARUS ACS712

## 46.1 Prinsip ACS712

ACS712-5A menggunakan efek Hall: arus yang melewati konduktor internal menciptakan medan magnet yang diukur sebagai tegangan. Sensitivitas: **185 mV/A**. Tegangan offset saat arus 0 A: **2.5V**.

Konversi (di `smart_pumpp.ino`):
```cpp
float v_sense = (adc_i / 4095.0f) * 3.3f * CFG_VDIV_FACTOR;
current_cached = (v_sense - acs_offset) / CFG_ACS_SENSITIVITY;
```
- `v_sense`: tegangan output sensor (setelah pembagi tegangan)
- `acs_offset`: tegangan offset yang dikalibrasi (≈2.48V bukan persis 2.5V)
- Dibagi 0.185 V/A → arus dalam Ampere

## 46.2 Auto-Koreksi Offset Drift

```cpp
if (current_cached < 0) {
  cur_neg_count++;
  if (cur_neg_count > CFG_I_DRIFT_THRESH && pwmActual == 0) {
    float noff = (analogReadOversampled(CFG_PIN_ACS,16)/4095.0f)*3.3f*CFG_VDIV_FACTOR;
    acs_offset = smooth(acs_offset, noff, 0.3f);
    cur_neg_count = 0;
  }
}
```
Arus tidak boleh negatif pada pompa DC. Jika terbaca negatif >5 iterasi berturut-turut DAN pompa mati (pwmActual=0), kemungkinan offset bergeser. Sistem mengambil ulang nilai referensi (nol arus) dan memperbarui `acs_offset` secara perlahan (α=0.3). Ini **auto-calibration online** yang mencegah drift akumulatif.

## 46.3 Current Deficit — Indikator Tidak Langsung Kavitasi

```cpp
float currentDeficit(float i_meas, int pwm) {
  if (pwm < CFG_PWM_FLOW_MIN) return 0.0f;
  float i_exp = iSlopeLearned * pwm + CFG_I_MODEL_OFFSET;
  float d = (i_exp - i_meas) / i_exp;
  return constrain(d, 0.0f, 1.0f);
}
```
**Model linear:** `I_exp = slope × PWM + offset`. Pada pompa normal, arus berbanding lurus dengan kecepatan. Saat kavitasi, impeler memompa uap bukan air → beban berkurang → arus turun di bawah ekspektasi.

`deficit = (I_exp - I_meas) / I_exp` → [0, 1]:
- 0 = arus persis sesuai model (normal)
- 0.12 = arus 12% di bawah ekspektasi → onset kavitasi (`CFG_I_DEFICIT_EARLY`)
- 0.25 = arus 25% di bawah ekspektasi → kavitasi berat (`CFG_I_DEFICIT_SEVERE`)

**Slope adaptif (`iSlopeLearned`):** secara default 0.01438 A/%. Jika `CFG_I_AUTO_BASE=1`, slope diperbarui otomatis saat kondisi Normal, sehingga model mengikuti karakteristik pompa yang berubah seiring waktu.

---

# BAB 47 — `sensor_vibration.h`: SENSOR VIBRASI MPU6050

## 47.1 Pipeline Lengkap Vibrasi

```
MPU6050 (akselerasi raw ax,ay,az)
  → EMA filter (α=0.25) → s_ax, s_ay, s_az
  → magnitude = √(s_ax²+s_ay²+s_az²)
  → gravity removal: vib_pre = |magnitude - vib_baseline|
  → hampelFilter(vib_pre) → outlier dihapus
  → medianFilter5(...) → mulus
  = vibration (m/s² dinamis)
```

## 47.2 Gravity Removal — Menghapus Komponen Gravitasi

Akselerometer selalu "merasakan" gravitasi ≈9.81 m/s² di sumbu vertikal. Ini komponen DC yang tidak relevan untuk kavitasi. Cara menghapusnya:

```cpp
float vib_mag = sqrtf(s_ax*s_ax + s_ay*s_ay + s_az*s_az);

// Saat pompa mati, pelajari baseline (nilai gravitasi statik)
if (pwmActual == 0 && sensorEnable[SENSOR_V])
  vib_baseline = smooth(vib_baseline, vib_mag, CFG_EMA_ALPHA_VIB_BASE);

float vib_pre = fabsf(vib_mag - vib_baseline);  // komponen dinamis
```

`vib_baseline` awalnya ≈9.81 m/s². Diperbarui sangat lambat (α=0.005) hanya saat pompa mati — ini baseline gravitasi statik. Selisih antara magnitude aktual dan baseline adalah **komponen dinamis** yang disebabkan getaran kavitasi.

**Mengapa nilai raw MPU ≈9.99 m/s²?** Karena gravitasi. `vib_baseline ≈ 9.81` m/s². Setelah dikurangi, komponen dinamis yang kecil (0.1–2 m/s²) menjadi terlihat. (Ini adalah pertanyaan jebakan penguji J9.)

## 47.3 Hampel Filter — Deteksi Outlier Statistik

```cpp
float hampelFilter(float x) {
  // 1. Simpan x ke buffer circular 7 elemen
  // 2. Urutkan buffer → temukan median
  // 3. Hitung MAD (Median Absolute Deviation)
  // 4. MAD × 1.4826 = estimasi standar deviasi robust
  // 5. Jika |x - median| > 3 × MAD: ganti x dengan median
  return x_filtered;
}
```

MAD adalah ukuran dispersi yang robust terhadap outlier (berbeda dari standar deviasi yang sensitif). Faktor 1.4826 mengonversi MAD ke estimasi simpangan baku distribusi normal. Jendela 7 → median posisi ke-3 (0-indexed).

**Contoh:** buffer = [0.3, 0.2, 0.4, 5.0, 0.3, 0.2, 0.3]. Nilai 5.0 adalah spike dari komutasi motor. Median ≈ 0.3, MAD kecil. |5.0-0.3| >> 3×MAD → diganti median 0.3. Spike tereliminasi.

## 47.4 Median Filter 5-Elemen

```cpp
float medianFilter5(float new_val) {
  vib_med_buf[vib_med_idx] = new_val;
  vib_med_idx = (vib_med_idx+1) % 5;
  float s[5]; // salin buffer
  // bubble sort 5 elemen
  return s[2]; // nilai tengah (indeks 2 dari 5)
}
```

Filter median 5 elemen dijalankan **setelah** Hampel. Kombinasi ini memberikan perlindungan berlapis: Hampel menghapus spike ekstrem, median menghaluskan sisa fluktuasi kecil.

## 47.5 Kalibrasi MPU

```cpp
void calibrateMPU() {
  // Ambil 500 sampel saat pompa mati, diam
  ax_off = rata-rata ax;
  ay_off = rata-rata ay;
  az_off = rata-rata az - CFG_VIB_GRAVITY;  // offset sumbu Z dikurangi gravitasi
}
```
Offset ini dikurangkan dari setiap pembacaan: `s_ax = smooth(s_ax, ax_raw - ax_off, α)`. Hasilnya: ketika pompa diam, s_ax ≈ s_ay ≈ 0 dan s_az ≈ 9.81. Kalibrasi disimpan di NVS.

---

# BAB 48 — `fuzzy.h`: LOGIKA FUZZY MAMDANI

## 48.1 MF Input: Tekanan (P)

Tekanan tidak menggunakan MF konvensional secara langsung. Sebagai gantinya dipakai **`pdanger`** — indeks bahaya terstandarisasi:

```cpp
float sf = constrain((float)pwmActual / CFG_PWM_P_THRESH_REF, 0.0f, 1.0f);
float p_early_eff  = CFG_P_SUCTION_EARLY  * sf;   // 0.40 × sf
float p_severe_eff = CFG_P_SUCTION_SEVERE * sf;   // 0.10 × sf
float pdanger = constrain((p_early_eff - P) / (p_early_eff - p_severe_eff), 0.0f, 1.0f);
```

- `sf` = faktor skala berdasarkan PWM aktual. Pada PWM=100% → sf=1, ambang penuh. Pada PWM=50% → sf=0.5, ambang dikurangi 50%.
- `pdanger` = 1.0 saat P ≤ p_severe_eff (sangat berbahaya), 0.0 saat P ≥ p_early_eff (aman)

Lalu dipetakan ke muP[]:
```cpp
muP[0] = pdanger;          // P_LOW (berbahaya)
muP[2] = 1.0f - pdanger;  // P_HIGH (aman)
muP[1] = 1.0f - |muP[0]-muP[2]|;  // P_MED (transisi)
```

**Mengapa diskalakan terhadap PWM?** Pada kecepatan rendah, tekanan isap secara fisik lebih rendah — bukan karena kavitasi, tapi karena pompa belum cukup kuat menghisap. Tanpa skala, setiap kecepatan rendah akan menghasilkan alarm palsu.

## 48.2 MF Input: Getaran (V)

```cpp
float mf_V_low(float V) {
  if (V <= CFG_MFV_LOW_FULL) return 1.0f;     // ≤0.20 m/s²: LOW=1
  if (V <  CFG_MFV_LOW_ZERO) return (0.60-V)/(0.60-0.20);  // turun linear ke 0
  return 0.0f;                                 // ≥0.60: LOW=0
}
float mf_V_med(float V) {
  // Segitiga puncak di 0.80 m/s², rentang 0.30–1.80
}
float mf_V_high(float V) {
  if (V <= CFG_MFV_HIGH_ZERO) return 0.0f;    // ≤1.20: HIGH=0
  if (V <  CFG_MFV_HIGH_FULL) return (V-1.20)/(2.00-1.20);  // naik linear
  return 1.0f;                                 // ≥2.00: HIGH=1
}
```

**Tabel nilai khas:**

| Kondisi | V (m/s²) | LOW | MED | HIGH |
|---|---|---|---|---|
| Normal (idle) | 0.20 | 1.00 | 0.00 | 0.00 |
| Early (onset) | 1.00 | 0.00 | 0.80 | 0.00 |
| Severe (kuat) | 1.45 | 0.00 | 0.28 | 0.31 |

## 48.3 MF Input: Arus (I) via Deficit

```cpp
float def = currentDeficit(I, pwmActual);  // [0,1]
muI[0] = constrain((def-0.12)/(0.25-0.12), 0.0f, 1.0f);  // I_LOW
muI[2] = constrain(1.0f - def/0.12, 0.0f, 1.0f);          // I_HIGH
muI[1] = constrain(1.0f - muI[0] - muI[2], 0.0f, 1.0f);   // I_MED
```

Bukan arus absolut, melainkan **defisit relatif** terhadap ekspektasi model. `muI[0]` (LOW=arus defisit besar=kavitasi) tinggi saat `deficit > 12%`.

## 48.4 27 Aturan dan Evaluasi

```cpp
static const FuzzyRule RULES[27] = {
  { 0,0,0,2 }, { 0,0,1,1 }, { 0,0,2,1 },  // P_LOW+V_LOW
  { 0,1,0,2 }, { 0,1,1,1 }, { 0,1,2,1 },  // P_LOW+V_MED
  { 0,2,0,2 }, { 0,2,1,2 }, { 0,2,2,2 },  // P_LOW+V_HIGH  ← semua SEVERE!
  // ...27 kombinasi lengkap...
};
```

Evaluasi semua 27 aturan sekaligus:
```cpp
float alpha_out[3] = {0, 0, 0};  // kekuatan per kelas output
for (int k = 0; k < 27; k++) {
  // Kekuatan aturan = min dari tiga anteseden (operator AND = min)
  float alpha = min(muP[RULES[k].p_idx],
                min(muV[RULES[k].v_idx],
                    muI[RULES[k].i_idx]));
  // Agregasi: ambil MAX dari semua aturan ke kelas yang sama (OR = max)
  if (alpha > alpha_out[RULES[k].out_idx])
    alpha_out[RULES[k].out_idx] = alpha;
}
```

**Logika desain korroborasi:** SEVERE hanya muncul saat:
- P_LOW + V_HIGH (R7, R8, R9): tekanan drop + getaran tinggi
- P_LOW + V_MED + I_LOW (R4): tekanan + arus keduanya konfirmasi

Tidak ada satu sensor pun yang dapat memicu SEVERE sendirian. Ini menekan false positive.

## 48.5 Defuzzifikasi Centroid (CoG)

```cpp
float numer = 0.0f, denom = 0.0f;
const int N_STEPS = 150;  // 0.00–1.50 dengan step 0.01
for (int yi = 0; yi <= N_STEPS; yi++) {
  float y = yi * 0.01f;  // indeks integer → tidak ada float drift
  float mu = max(min(alpha_out[0], mf_out_normal(y)),
             max(min(alpha_out[1], mf_out_early(y)),
                 min(alpha_out[2], mf_out_severe(y))));
  numer += y * mu;
  denom += mu;
}
float K = (denom < 1e-6f) ? 0.0f : numer/denom;
```

**Langkah demi langkah:**
1. Pindai domain y dari 0 sampai 1.5 (151 titik)
2. Untuk setiap y, hitung `mu(y)` teragregasi = max dari ketiga himpunan terpotong
3. Akumulasi `numer += y*mu`, `denom += mu`
4. `K = numer/denom` = titik berat

**Mengapa indeks integer (`yi`) bukan float `y += 0.01`?** Float 0.01 tidak dapat direpresentasikan persis dalam biner (0.01 = 0.0000001010001... berulang di basis 2). Setelah 150 langkah, `0.00 + 0.01×150 ≠ 1.50` persis — ada galat akumulatif. Dengan `yi * 0.01f`, setiap langkah dihitung dari nol → tidak ada drift.

## 48.6 Gate Korroborasi

```cpp
if (eP || eI) {
  float corrob = 0.0f;
  if (eI) corrob = max(corrob, currentDeficit(I, pwmActual)/CFG_I_DEFICIT_EARLY);
  if (eP) corrob = max(corrob, muP[0]);  // pdanger
  if (corrob < CFG_CAV_CORROB_MIN)      // < 0.25
    cavOut *= (corrob / CFG_CAV_CORROB_MIN);  // skala bawah K
}
```

Setelah defuzzifikasi, jika sinyal konfirmasi dari P atau I lemah (<25%), indeks K diskalakan turun. Tujuannya: sinyal getaran tinggi (V) saja — mungkin dari sumber mekanis bukan kavitasi — tidak cukup memicu alarm. Harus ada "jangkar" dari P atau I.

## 48.7 FGS: Fuzzy Gain Scheduling

```cpp
float interpolateGain(float K, float yA, float yB, float yC) {
  if (K <= 0.00) return yA;
  if (K <= 0.70) return lerp(K, 0.00, 0.70, yA, yB);  // A→B
  if (K <= 1.15) return lerp(K, 0.70, 1.15, yB, yC);  // B→C
  if (K <= 1.50) return lerp(K, 1.15, 1.50, yC, 0.0); // C→D (→0)
  return 0.0f;
}
```

**Contoh Mode I, K=0.35:**
- K berada antara A (0.00) dan B (0.70)
- Kp = lerp(0.35, 0.00, 0.70, 6.0, 3.0) = 6.0 + (3.0-6.0)×(0.35/0.70) = 6.0 - 1.5 = **4.50**
- Ki = lerp(0.35, 0.00, 0.70, 0.80, 0.30) = 0.80 - 0.25 = **0.55**

Gain menurun halus (bukan lompatan) seiring K naik. Saat K=1.15 (Severe), semua gain = 0 → PID mati → pompa dikendalikan supervisori (cavPwmCap).

---

# BAB 49 — `pid_control.h`: KONTROLER PID ADAPTIF

## 49.1 Pemilihan Mode PID

```cpp
String determinePIDMode() {
  bool okV = sensorEnable[SENSOR_V] && vibrationOK && mpuOK;
  bool okP = sensorEnable[SENSOR_P] && pressureOK;
  bool okI = sensorEnable[SENSOR_I] && currentOK;
  // Prioritas: I → V → P (ditetapkan oleh CFG_CTRL_PRIMARY='I')
  if (okI) return "I";
  if (okV) return "V";
  if (okP) return "P";
  return "-";
}
```

Mode I (arus) adalah mode utama. Mode V (getaran) adalah fallback pertama. Mode P (tekanan) adalah fallback terakhir. Fallback otomatis terjadi jika sensor bermasalah — sistem tidak pernah berhenti karena satu sensor mati.

**Mengapa arus sebagai mode utama?** Arus paling stabil dan tidak langsung dipengaruhi kavitasi (dibanding tekanan yang berosilasi). Kontroler kecepatan berbasis arus (speed control) memberikan respons yang paling konsisten.

## 49.2 PID FGS Position-Form

```cpp
float computePID_FGS(float sp, float pv, float cavIdx,
                     char mode, float deadband, float dt) {
  float error = sp - pv;

  // Deadband: error kecil diabaikan
  if (fabsf(error) <= deadband) {
    pid_integral *= 0.98f;  // integral diredam perlahan
    pid_prev_error = error;
    return 0.0f;
  }

  // Ambil gain dari FGS
  DynGains g = fuzzyGainSchedule(cavIdx, mode);

  // Jika semua gain ≈ 0 (K ≥ 1.15), reset dan keluar
  if (g.Kp < 0.001f && g.Ki < 0.001f && g.Kd < 0.001f) {
    pid_integral = 0; return 0.0f;
  }

  // Anti-windup: integrator hanya aktif jika tidak saturasi
  bool satHi  = (pwmActual >= 100 && error > 0);  // PWM maks, error positif
  bool satLo  = (pwmActual <= 0   && error < 0);  // PWM min, error negatif
  bool satCap = (pwmActual >= cavPwmCap && error > 0);  // cap kavitasi
  if (g.Ki > 0.0f && !satHi && !satLo && !satCap)
    pid_integral = constrain(pid_integral + error*dt,
                             -CFG_INTEGRAL_MAX, CFG_INTEGRAL_MAX);  // ±2

  // Filtered derivative (EMA α=0.15)
  float raw_d = (error - pid_prev_error) / dt;
  pid_deriv_filt = smooth(pid_deriv_filt, raw_d, CFG_DERIV_ALPHA);
  pid_prev_error = error;

  // PID output
  return g.Kp * error + g.Ki * pid_integral + g.Kd * pid_deriv_filt;
}
```

**Position-form vs velocity-form:** versi sebelumnya pakai velocity-form (`Δu[k] = Kp·Δe + Ki·e + Kd·Δ²e`) yang menyebabkan PWM drift/stall. Position-form (`u[k] = Kp·e + Ki·Σe + Kd·Δe`) lebih stabil untuk aktuator diskret UP/DOWN.

**Deadband:** jika |error| < batas, output = 0 dan integrator diredam 2% per iterasi (bukan dimatikan mendadak). Ini mencegah akumulasi integrator saat pompa sudah stabil.

**Anti-windup clamping ±2:** integrator tidak boleh melampaui ±2. Ini mencegah overshoot besar saat kondisi berubah. Nilai 2 adalah dalam satuan `error × waktu` (unit abstrak PID, bukan bar/kPa).

**Mengapa `satCap`?** Saat kavitasi, `cavPwmCap` menahan PWM tidak boleh naik. Jika setpoint lebih tinggi dari cavPwmCap, error selalu positif, tapi PWM tidak bisa naik → integrator terus tumbuh tanpa guna (windup). `satCap` memutus ini.

## 49.3 Mesin Keadaan Hysteresis Kavitasi

```cpp
int updateCavState(float idx) {
  int target = cavState;
  // Tentukan target berdasarkan state saat ini
  switch (cavState) {
    case 0: if (idx > CFG_CAV_0_TO_1) target=1; break;  // Normal→Early?
    case 1:
      if      (idx < CFG_CAV_1_TO_0) target=0;          // Early→Normal?
      else if (idx > CFG_CAV_1_TO_2) target=2;          // Early→Severe?
      break;
    case 2: if (idx < CFG_CAV_2_TO_1) target=1; break;  // Severe→Early?
  }

  // Jika target sama → tetap di state ini
  if (target == cavState) { cav_pending = cavState; return cavState; }

  // Jika target berubah, mulai hitung waktu konfirmasi
  if (target != cav_pending) { cav_pending = target; cavPendSince = now; return cavState; }

  // Jika target konsisten cukup lama → transisi
  unsigned long needMs = (target > cavState) ? 1500 : 1500;
  if ((now-cavPendSince) >= needMs && (now-cavStateEnterTime) >= 800) {
    cavState = target;
    cavStateEnterTime = now;
  }
  return cavState;
}
```

**Alur logika:**
1. Hitung `target` yang diinginkan dari nilai K
2. Jika target berbeda dari `cav_pending` (target berubah-ubah) → reset timer konfirmasi
3. Jika target konsisten selama 1500ms AND sudah 800ms dari transisi terakhir → pindah state

**Contoh:** K naik ke 0.70 selama 200ms lalu turun → tidak ada transisi (belum 1500ms). K harus bertahan di atas 0.65 selama 1500ms berturut-turut untuk berpindah ke Early.

## 49.4 Rate Limiter dan Konversi PID→PWM Delta

```cpp
int pidToDelta(float du, int state) {
  int mx = (state==0) ? 2 : (state==1) ? 1 : 4;  // max langkah per aktuasi
  return constrain((int)roundf(du), -mx, mx);
}

int applyRateLimit(int target, int actual) {
  int d = target - actual;
  if (d >  CFG_PWM_RATE_UP)   return actual + 1;  // maks naik 1
  if (d < -CFG_PWM_RATE_DOWN) return actual - 2;  // maks turun 2
  return target;
}
```

`pidToDelta` mengubah output PID float ke integer langkah PWM, dengan batas berbeda per state. `applyRateLimit` adalah batas akhir: **tidak peduli seberapa besar PID menginginkan perubahan**, PWM hanya bisa bergerak ±1/−2 per 400ms.

---

# BAB 50 — `smart_pumpp.ino`: SETUP DAN LOOP UTAMA

## 50.1 setup() — Inisialisasi Sistem

```cpp
void setup() {
  setCpuFrequencyMhz(160);    // set CPU 160 MHz
  esp_task_wdt_init(...);     // inisialisasi watchdog 5 detik
  WiFi.softAP("NERO", ...);   // buat Access Point
  server.begin();              // HTTP server aktif

  Wire.begin(7, 6);            // I²C di GPIO 7 (SDA), 6 (SCL)
  mpu.begin(0x68);             // alamat I²C MPU6050

  calibrateMPU();              // kalibrasi gravitasi baseline
  calibrateACS();              // kalibrasi offset arus (pompa mati)
  loadPressureCalibration();   // muat kalibrasi dari NVS
}
```

**Urutan penting:** kalibrasi ACS dilakukan saat pompa mati (PWM=0, `manualStop=true`). Jika kalibrasi dilakukan saat pompa hidup, offset akan salah karena ada arus motor.

## 50.2 Loop() — Pembacaan Sensor

### Siklus Alternasi ADC
```cpp
if ((loopCounter % 2) == 0) {
  // baca ACS dan tekanan
}
// MPU6050 dibaca setiap iterasi (tanpa alternasi)
```
ADC dibaca tiap 2 iterasi (~2ms) bukan tiap iterasi. Alasannya: ADC membutuhkan waktu settling. Membaca terlalu sering tidak memberikan data baru yang bermakna. MPU6050 via I²C dibaca tiap iterasi karena library menangani timing internal.

### Validasi Sensor
```cpp
currentOK = !(adc_i<5 || adc_i>4090 ||
              current_cached<CFG_I_MIN_VALID ||
              current_cached>CFG_I_MAX_VALID);
```
- ADC < 5 atau > 4090: kabel putus atau short circuit
- Arus di luar -1A hingga 6A: tidak masuk akal secara fisik

## 50.3 Loop() — Logika Kavitasi

```cpp
bool cavDetectActive = !startupMode && !stabilizing &&
                       (millis() >= cavDetectEnableAt);
float cavIdx = 0.0f;
if (cavDetectActive) {
  cavIdx = fuzzyMamdaniAdaptive(s_p, vibration, s_i, fuzzyMode);
} else {
  cavState = 0; cav_pending = 0;  // reset state machine
}
int cs = cavDetectActive ? updateCavState(cavIdx) : 0;
```

Selama startup/stabilisasi/settle timer: `cavIdx = 0`, state machine di-reset. Ini mencegah nilai sensor transien (saat kecepatan naik dari 0) memicu alarm palsu.

## 50.4 Loop() — Setpoint Mode I (Adaptif per State)

```cpp
case 'I': {
  float stateScale = (cs==2) ? 0.50f : (cs==1) ? 0.70f : 1.00f;
  int effPct = max(20, roundf(curSpeedTargetPct * stateScale));
  ispEffPct  = effPct;
  sp_used = iSlopeLearned * effPct + CFG_I_MODEL_OFFSET;
  pv_used = s_i;
  db_used = CFG_DEADBAND_CUR;  // ±0.02 A
}
```

Setpoint arus bukan nilai tetap. Ia **berubah diskrit saat state berubah**:
- Normal: target = 100% kecepatan yang diinginkan
- Early: target = 70% kecepatan
- Severe: target = 50% kecepatan

PID kemudian mengatur arus menuju setpoint baru ini. Saat state kembali Normal, setpoint naik ke 100% lagi.

**Mengapa bukan langsung set PWM?** PID memberikan respons yang smooth dan mempertimbangkan error aktual. Setpoint ke 70% tidak selalu berarti PWM 70% — bergantung karakteristik pompa dan kondisi saat itu.

## 50.5 Loop() — Supervisori cavPwmCap

```cpp
if      (cs == 2) cavPwmCap = max(20, cavPwmCap - 4);  // Severe: turun 4/400ms
else if (cs == 1) cavPwmCap = max(20, cavPwmCap - 3);  // Early:  turun 3/400ms
else              cavPwmCap = min(100, cavPwmCap + 2); // Normal: naik  2/400ms

// PWM tidak boleh melebihi cap
if (pwmTarget > cavPwmCap) pwmTarget = cavPwmCap;
```

`cavPwmCap` adalah batas atas PWM yang diizinkan — lapisan supervisori di atas PID. Saat kavitasi aktif, cap terus turun (memaksa pompa melambat) dengan kecepatan 3–4%/400ms = 7.5–10%/detik. Saat Normal, cap naik perlahan 5%/detik.

**Mengapa dua mekanisme (setpoint + cap)?** Setpoint menggerakkan PID menuju kecepatan target. Cap adalah jaring pengaman: jika PID karena alasan apapun masih mendorong PWM naik, cap mencegahnya.

## 50.6 Loop() — Recovery Hold

```cpp
if (cs == 0 && cavStatePrev > 0) {
  cavRecoveryUntil = millis() + CFG_CAV_RECOVERY_HOLD_MS;  // 4 detik
  pid_integral = 0; pid_deriv_filt = 0;  // reset PID
}
// Blokir kenaikan kecepatan selama recovery
if (millis() < cavRecoveryUntil && lim > pwmActual) lim = pwmActual;
```

Saat kavitasi baru saja hilang (state turun dari 1/2 ke 0), sistem menunggu 4 detik sebelum memperbolehkan kecepatan naik lagi. Ini mencegah siklus kavitasi–pemulihan–kavitasi yang cepat (hunting). Integrator PID juga di-reset agar tidak ada "sisa" dari kondisi sebelumnya.

## 50.7 Loop() — Aktuasi Relay Cadence

```cpp
if (millis()-lastActuator > CFG_ACTUATOR_INTERVAL) {  // tiap 400ms
  lastActuator = millis();
  if (relayBusy()) { /* tunggu */ }
  else if (manualStop || emergencyStop) {
    if (pwmActual > 0) { scheduleRelay(true); pwmActual--; }
  } else if (startupMode) {
    if (pwmActual < 20) { scheduleRelay(false); pwmActual++; }
    else { /* startup selesai, masuk stabilizing */ }
  } else if (stabilizing) {
    // tunggu stabilizing selesai
  } else {
    // Operasi normal: hitung target dari PID, terapkan cap, rate limit, aktuasi
    int lim = applyRateLimit(pwmTarget, pwmActual);
    if (lim > pwmActual) { scheduleRelay(false); pwmActual++; }
    else if (lim < pwmActual) { scheduleRelay(true);  pwmActual--; }
  }
}
```

Relay hanya diaktuasi **tiap 400ms**. Dalam 400ms, bisa naik paling banyak 1 langkah atau turun 2 langkah (rate limiter). PWM berubah 1% per 400ms = 2.5%/detik naik, 5%/detik turun.

**Hitung total**: dari 0% ke 100% butuh 100 × 400ms = 40 detik. Ini lambat secara sengaja — mencegah water hammer dan kavitasi kambuh.

---

## PENUTUP BLOK VII

Anda telah membedah seluruh 12 file firmware, dari konstanta terkecil hingga logika loop utama. Alur sinyal lengkap:

```
Sensor fisik (P, V, I)
   → ADC / I²C → oversampling
   → EMA filter
   → [V] gravity removal → Hampel → median
   → MF Fuzzy (pdanger, mf_V, deficit)
   → 27 aturan Mamdani (AND=min, OR=max)
   → Centroid CoG (151 titik) → K ∈ [0,1.5]
   → Gate korroborasi → K tervalidasi
   → Mesin keadaan hysteresis (1500ms confirm, 800ms hold)
   → State kavitasi (0/1/2)
   → FGS: interpolasi gain (Kp,Ki,Kd) dari K
   → PID position-form (deadband, anti-windup ±2, filtered deriv α=0.15)
   → pidToDelta → rate limiter (+1/-2 per 400ms)
   → cavPwmCap (supervisori, -4/-3/+2 per 400ms)
   → Recovery hold (4 detik setelah kavitasi hilang)
   → scheduleRelay(up/down)
   → tickRelay: IDLE→PULSE(80ms)→SETTLE(260/200ms)→IDLE
   → Motor kontroler → kecepatan pompa
```

Setiap elemen dalam rantai ini ada alasannya. Tidak ada kode yang tidak perlu.

---
*Akhir Blok VII (BAB 41–50). — Pembedahan Firmware SmartPump.*
