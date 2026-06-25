#pragma once

// fuzzy.h — MF, Mamdani 27-aturan, Fuzzy Gain Scheduling

// Batas defuzzifikasi CoG
#define CAV_OUT_MIN   0.0f
#define CAV_OUT_MAX   1.5f
#define CAV_OUT_STEP  0.01f
// Bentuk MF keluaran — dirancang agar CoG tiap state bersih:
//   NORMAL pure  → CoG ≈ 0.083  (NORM_R/3)
//   EARLY  pure  → CoG ≈ 0.70   ((L+PK+R)/3)
//   SEVERE pure  → CoG ≈ 1.367  ((L+PK+R)/3)
// NORMAL: spike sempit di y=0 agar normal state ≈ 0
#define CAV_NORM_L    0.00f
#define CAV_NORM_PK   0.00f
#define CAV_NORM_R    0.25f   // was 0.50 — dipersempit, CoG_normal = 0.083
// EARLY: centroid di 0.70, overlap dengan NORM di [0.20,0.25] dan SEVERE di [1.10,1.20]
#define CAV_EARLY_L   0.20f   // was 0.30 — geser kiri agar overlap NORM
#define CAV_EARLY_PK  0.70f
#define CAV_EARLY_R   1.20f   // was 1.10 — geser kanan agar overlap SEVERE
// SEVERE: ramp ke puncak 1.50, geser kanan untuk CoG lebih tinggi
#define CAV_SEV_L     1.10f   // was 0.90 — dipersempit, CoG_severe ≈ 1.367
#define CAV_SEV_PK    1.50f
#define CAV_SEV_R     1.50f

// MF Tekanan 
// CATATAN: mf_P_low/norm/high tidak dipanggil dalam inferensi Mamdani.
float mf_P_low(float P) {
  if (P <= CFG_MFP_LOW_FULL) return 1.0f;
  if (P <  CFG_MFP_LOW_ZERO) return (CFG_MFP_LOW_ZERO-P)/(CFG_MFP_LOW_ZERO-CFG_MFP_LOW_FULL);
  return 0.0f;
}
float mf_P_norm(float P) {
  if (P <= CFG_MFP_NORM_L0 || P >= CFG_MFP_NORM_RPEAK) return 0.0f;
  if (P <  CFG_MFP_NORM_LPEAK) return (P-CFG_MFP_NORM_L0)/(CFG_MFP_NORM_LPEAK-CFG_MFP_NORM_L0);
  return (CFG_MFP_NORM_RPEAK-P)/(CFG_MFP_NORM_RPEAK-CFG_MFP_NORM_LPEAK);
}
float mf_P_high(float P) {
  if (P <= CFG_MFP_HIGH_ZERO) return 0.0f;
  if (P <  CFG_MFP_HIGH_FULL) return (P-CFG_MFP_HIGH_ZERO)/(CFG_MFP_HIGH_FULL-CFG_MFP_HIGH_ZERO);
  return 1.0f;
}

// MF Vibrasi 
float mf_V_low(float V) {
  if (V <= CFG_MFV_LOW_FULL) return 1.0f;
  if (V <  CFG_MFV_LOW_ZERO) return (CFG_MFV_LOW_ZERO-V)/(CFG_MFV_LOW_ZERO-CFG_MFV_LOW_FULL);
  return 0.0f;
}
float mf_V_med(float V) {
  if (V <= CFG_MFV_MED_L0 || V >= CFG_MFV_MED_RPEAK) return 0.0f;
  if (V <  CFG_MFV_MED_LPEAK) return (V-CFG_MFV_MED_L0)/(CFG_MFV_MED_LPEAK-CFG_MFV_MED_L0);
  return (CFG_MFV_MED_RPEAK-V)/(CFG_MFV_MED_RPEAK-CFG_MFV_MED_LPEAK);
}
float mf_V_high(float V) {
  if (V <= CFG_MFV_HIGH_ZERO) return 0.0f;
  if (V <  CFG_MFV_HIGH_FULL) return (V-CFG_MFV_HIGH_ZERO)/(CFG_MFV_HIGH_FULL-CFG_MFV_HIGH_ZERO);
  return 1.0f;
}

// MF Arus 
float mf_I_low(float I) {
  if (I <= CFG_MFI_LOW_FULL) return 1.0f;
  if (I <  CFG_MFI_LOW_ZERO) return (CFG_MFI_LOW_ZERO-I)/(CFG_MFI_LOW_ZERO-CFG_MFI_LOW_FULL);
  return 0.0f;
}
float mf_I_norm(float I) {
  if (I <= CFG_MFI_NORM_L0 || I >= CFG_MFI_NORM_RPEAK) return 0.0f;
  if (I <  CFG_MFI_NORM_LPEAK) return (I-CFG_MFI_NORM_L0)/(CFG_MFI_NORM_LPEAK-CFG_MFI_NORM_L0);
  return (CFG_MFI_NORM_RPEAK-I)/(CFG_MFI_NORM_RPEAK-CFG_MFI_NORM_LPEAK);
}
float mf_I_high(float I) {
  if (I <= CFG_MFI_HIGH_ZERO) return 0.0f;
  if (I <  CFG_MFI_HIGH_FULL) return (I-CFG_MFI_HIGH_ZERO)/(CFG_MFI_HIGH_FULL-CFG_MFI_HIGH_ZERO);
  return 1.0f;
}

// MF Keluaran 
float mf_out_normal(float y) {
  if (y < CAV_NORM_L || y >= CAV_NORM_R) return 0.0f;  // '<' bukan '<=' agar puncak (y=0) dievaluasi
  return (CAV_NORM_R - y) / CAV_NORM_R;
}
float mf_out_early(float y) {
  if (y <= CAV_EARLY_L || y >= CAV_EARLY_R) return 0.0f;
  if (y < CAV_EARLY_PK) return (y - CAV_EARLY_L) / (CAV_EARLY_PK - CAV_EARLY_L);
  return (CAV_EARLY_R - y) / (CAV_EARLY_R - CAV_EARLY_PK);
}
float mf_out_severe(float y) {
  if (y <= CAV_SEV_L) return 0.0f;
  if (y >= CAV_SEV_PK) return 1.0f;
  return (y - CAV_SEV_L) / (CAV_SEV_PK - CAV_SEV_L);
}

// ── 27 Aturan Mamdani ─────────────────────────────────────────────
// Format: {P_idx, V_idx, I_idx, out_idx}
//   P: 0=P_LOW (tekanan rendah/berbahaya), 1=P_MED, 2=P_HIGH (tekanan tinggi/aman)
//   V: 0=V_LOW, 1=V_MED, 2=V_HIGH
//   I: 0=I_LOW (defisit arus), 1=I_MED, 2=I_HIGH (arus normal)
//   out: 0=NORMAL, 1=EARLY, 2=SEVERE
// Prinsip: Deteksi membutuhkan MINIMAL DUA INDIKATOR yang saling mengonfirmasi.
//   P_LOW + V_LOW + I_HIGH → NORMAL  (V dan I membantah bahaya — P saja tidak cukup)
//   P_LOW + V_LOW + I_MED  → EARLY   (P drop + I mulai turun = dua indikator)
//   P_LOW + V_LOW + I_LOW  → SEVERE  (P + I keduanya konfirmasi bahaya)
//   P_LOW + V_MED/HIGH     → EARLY/SEVERE (P + getaran = dua indikator)
//   P_HIGH + V_HIGH        → EARLY   (getaran mekanis tanpa pressure drop = onset ringan)
static const FuzzyRule RULES[27] = {
  { 0,0,0,2 }, { 0,0,1,1 }, { 0,0,2,1 },  // P_LOW+V_LOW:  SEVERE / EARLY / EARLY
  { 0,1,0,2 }, { 0,1,1,1 }, { 0,1,2,1 },  // P_LOW+V_MED:  SEVERE / EARLY / EARLY
  { 0,2,0,2 }, { 0,2,1,2 }, { 0,2,2,2 },  // P_LOW+V_HIGH: SEVERE / SEVERE / SEVERE
  { 1,0,0,1 }, { 1,0,1,0 }, { 1,0,2,0 },  // P_MED+V_LOW:  EARLY(I_LOW) / NORMAL / NORMAL
  { 1,1,0,1 }, { 1,1,1,0 }, { 1,1,2,0 },  // P_MED+V_MED:  EARLY / NORMAL / NORMAL
  { 1,2,0,2 }, { 1,2,1,1 }, { 1,2,2,1 },  // P_MED+V_HIGH: SEVERE / EARLY / EARLY
  { 2,0,0,0 }, { 2,0,1,0 }, { 2,0,2,0 },  // P_HIGH+V_LOW: NORMAL
  { 2,1,0,1 }, { 2,1,1,0 }, { 2,1,2,0 },  // P_HIGH+V_MED: EARLY(I_LOW) / NORMAL / NORMAL
  { 2,2,0,0 }, { 2,2,1,0 }, { 2,2,2,0 }   // P_HIGH+V_HIGH: NORMAL (getaran mekanis — P aman, bukan kavitasi)
};
// {0,0,2}: P_LOW+V_LOW+I_HIGH  NORMAL→EARLY
// Alasan: Mode I menjaga arus selalu di setpoin (I_HIGH), sehingga arus tidak bisa
// menjadi indikator kavitasi. Deteksi harus bergantung pada tekanan (P_LOW).
// Penurunan tekanan isap yang cukup (pdanger ≥ corrob_min) sudah merupakan 2 indikator
// (tekanan + speed-proportional threshold) untuk memicu Early.

// Fuzzy Mamdani Adaptif 
float fuzzyMamdaniAdaptive(float P, float V, float I, String& modeOut) {
  bool eP = sensorEnable[SENSOR_P];
  bool eV = sensorEnable[SENSOR_V];
  bool eI = sensorEnable[SENSOR_I];
  int n = (eP?1:0) + (eV?1:0) + (eI?1:0);
  if (n == 0) { modeOut = "-"; return 0.0f; }

  // Hanya cek PWM minimum — cukup untuk memastikan pompa berputar.
  // Kondisi sekunder (V rendah + I kecil) dihapus karena saat sensor I off
  // s_i=0 selalu memenuhi syarat, memblokir deteksi P saat kavitasi awal.
  bool noFlow = (pwmActual < CFG_PWM_FLOW_MIN);
  if (noFlow) {
    String mm = "";
    if (eV) mm += "V"; if (eP) mm += "P"; if (eI) mm += "I";
    modeOut = mm; return 0.0f;
  }

  float muP[3], muV[3], muI[3];

  if (eP) {
    float sf = constrain((float)pwmActual / (float)CFG_PWM_P_THRESH_REF, 0.0f, 1.0f);
    float p_early_eff  = CFG_P_SUCTION_EARLY  * sf;
    float p_severe_eff = CFG_P_SUCTION_SEVERE * sf;
    float span = p_early_eff - p_severe_eff;

    float pdanger = (span > 0.005f && pwmActual > CFG_PWM_FLOW_MIN && cavState == 0)
        ? constrain((p_early_eff - P) / span, 0.0f, 1.0f)
        : 0.0f;
    muP[0] = pdanger;
    muP[2] = 1.0f - pdanger;
    muP[1] = 1.0f - fabsf(muP[0] - muP[2]);
  } else { muP[0]=1.0f; muP[1]=1.0f; muP[2]=1.0f; }

  if (eV) { muV[0]=mf_V_low(V); muV[1]=mf_V_med(V); muV[2]=mf_V_high(V); }
  else    { muV[0]=1.0f; muV[1]=1.0f; muV[2]=1.0f; }

  if (eI) {
    float def = currentDeficit(I, pwmActual);
    float lowDef = constrain((def-CFG_I_DEFICIT_EARLY)/(CFG_I_DEFICIT_SEVERE-CFG_I_DEFICIT_EARLY), 0.0f, 1.0f);
    muI[0] = lowDef;
    muI[2] = constrain(1.0f - def/CFG_I_DEFICIT_EARLY, 0.0f, 1.0f);
    muI[1] = constrain(1.0f - muI[0] - muI[2], 0.0f, 1.0f);
  } else { muI[0]=1.0f; muI[1]=1.0f; muI[2]=1.0f; }

  float alpha_out[3] = {0.0f, 0.0f, 0.0f};
  for (int k = 0; k < 27; k++) {
    float alpha = min(muP[RULES[k].p_idx], min(muV[RULES[k].v_idx], muI[RULES[k].i_idx]));
    uint8_t o = RULES[k].out_idx;
    if (alpha > alpha_out[o]) alpha_out[o] = alpha;
  }

  // Gunakan counter integer agar tidak ada float drift (0.01 × 150 ≠ 1.50 persis di float32).
  float numer = 0.0f, denom = 0.0f;
  const int N_STEPS = (int)roundf((CAV_OUT_MAX - CAV_OUT_MIN) / CAV_OUT_STEP);
  for (int yi = 0; yi <= N_STEPS; yi++) {
    float y = CAV_OUT_MIN + yi * CAV_OUT_STEP;
    float mu = max(min(alpha_out[0], mf_out_normal(y)),
               max(min(alpha_out[1], mf_out_early(y)),
                   min(alpha_out[2], mf_out_severe(y))));
    numer += y * mu; denom += mu;
  }

  String mm = "";
  if (eV) mm += "V"; if (eP) mm += "P"; if (eI) mm += "I";
  modeOut = mm;

  if (denom < 1e-6f) return 0.0f;
  float cavOut = constrain(numer / denom, 0.0f, CAV_OUT_MAX);

  // Gate koroborasi: hanya aktif jika minimal satu sensor anchor (P atau I) tersedia.
  // Tujuan: mencegah V semata (getaran mekanis) memicu alarm tanpa konfirmasi.
  // Jika P dan I keduanya off (V-only), bypass gate — tidak ada anchor untuk koroborasi.
  if (eP || eI) {
    float corrob = 0.0f;
    if (eI) corrob = max(corrob,
              constrain(currentDeficit(I, pwmActual)/CFG_I_DEFICIT_EARLY, 0.0f, 1.0f));
    if (eP) corrob = max(corrob, muP[0]);
    if (corrob < CFG_CAV_CORROB_MIN)
      cavOut *= (corrob / CFG_CAV_CORROB_MIN);
  }
  return cavOut;
}

// Fuzzy Gain Scheduling 
float lerpFGS(float x, float x0, float x1, float y0, float y1) {
  if (x1 <= x0) return y0;
  float t = constrain((x - x0) / (x1 - x0), 0.0f, 1.0f);
  return y0 + t * (y1 - y0);
}

float interpolateGain(float cavIdx, float yA, float yB, float yC) {
  if (cavIdx <= FGS_CAVIDX_A) return yA;
  if (cavIdx <= FGS_CAVIDX_B) return lerpFGS(cavIdx, FGS_CAVIDX_A, FGS_CAVIDX_B, yA, yB);
  if (cavIdx <= FGS_CAVIDX_C) return lerpFGS(cavIdx, FGS_CAVIDX_B, FGS_CAVIDX_C, yB, yC);
  if (cavIdx <= FGS_CAVIDX_D) return lerpFGS(cavIdx, FGS_CAVIDX_C, FGS_CAVIDX_D, yC, 0.0f);
  return 0.0f;
}

DynGains fuzzyGainSchedule(float cavIdx, char mode) {
  DynGains g = {0.0f, 0.0f, 0.0f};
  if (mode == 'V') {
    g.Kp=interpolateGain(cavIdx,FGS_V_KP_A,FGS_V_KP_B,FGS_V_KP_C);
    g.Ki=interpolateGain(cavIdx,FGS_V_KI_A,FGS_V_KI_B,FGS_V_KI_C);
    g.Kd=interpolateGain(cavIdx,FGS_V_KD_A,FGS_V_KD_B,FGS_V_KD_C);
  } else if (mode == 'P') {
    g.Kp=interpolateGain(cavIdx,FGS_P_KP_A,FGS_P_KP_B,FGS_P_KP_C);
    g.Ki=interpolateGain(cavIdx,FGS_P_KI_A,FGS_P_KI_B,FGS_P_KI_C);
    g.Kd=interpolateGain(cavIdx,FGS_P_KD_A,FGS_P_KD_B,FGS_P_KD_C);
  } else if (mode == 'I') {
    g.Kp=interpolateGain(cavIdx,FGS_I_KP_A,FGS_I_KP_B,FGS_I_KP_C);
    g.Ki=interpolateGain(cavIdx,FGS_I_KI_A,FGS_I_KI_B,FGS_I_KI_C);
    g.Kd=interpolateGain(cavIdx,FGS_I_KD_A,FGS_I_KD_B,FGS_I_KD_C);
  }
  return g;
}
