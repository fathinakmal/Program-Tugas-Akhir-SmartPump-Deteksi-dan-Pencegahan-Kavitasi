#pragma once

// web_server.h — HTTP server: routing, kalibr. perintah, streaming CSV

// Forward declarations
void sendHTML(WiFiClient& c);
void sendData(WiFiClient& c);
void sendSensorStatus(WiFiClient& c);
void sendMFData(WiFiClient& c);
void sendLogStatus(WiFiClient& c);
void sendCSVHeader(WiFiClient& c);

// Routing utama 
void handleClient() {
  if (csvStreaming) {
    if (!activeClient.connected()) { activeClient.stop(); csvStreaming=false; return; }
    int sent=0;
    while (csvStreamIdx<csvStreamTotal && sent<CFG_CSV_CHUNK) {
      int idx=(csvStreamStart+csvStreamIdx)%CFG_CSV_BUF_SIZE;
      LogRow& r=csvBuf[idx];
      char line[200];
      snprintf(line,sizeof(line),
        "%lu,%.4f,%.4f,%.2f,%.5f,%d,%.4f,%d,%.4f,%s,%s,%s,%u\r\n",
        r.time_ms,r.vibration,r.current,
        r.pressure_kPa,r.pressure_kPa/100.0f,
        r.pwm,r.cavIdx,r.cavState,r.pidOut,
        r.sensorSt,r.fuzzyMode,r.pidMode,(unsigned)r.enableMask);
      activeClient.print(line);
      csvStreamIdx++; sent++;
    }
    esp_task_wdt_reset();
    if (csvStreamIdx>=csvStreamTotal) { activeClient.stop(); csvStreaming=false; }
    return;
  }

  if (!clientActive) {
    WiFiClient c=server.available();
    if (!c) return;
    activeClient=c; clientRequest=""; clientActive=true; clientTimeout=millis();
    return;
  }
  if (!activeClient.connected()) { activeClient.stop(); clientActive=false; return; }
  if (millis()-clientTimeout>500) { activeClient.stop(); clientActive=false; return; }

  while (activeClient.available()) {
    char c=activeClient.read();
    if (c=='\r'||c=='\n') break;
    clientRequest+=c;
    if (clientRequest.length()>250) break;
  }
  if (activeClient.available()==0&&clientRequest.indexOf("GET")==-1) {
    if (millis()-clientTimeout<500) return;
  }

  if (clientRequest.indexOf("/start")!=-1) {
    manualStop=false; startupMode=true;
    pid_integral=0; pid_prev_error=0; pid_deriv_filt=0;
    cavState=0; cavStatePrev=0; cav_pending=0; cavStateEnterTime=millis(); cavPendSince=millis(); pwmTarget=0;
    cavPwmCap=CFG_PWM_MAX; cavRecoveryUntil=0;
    cavDetectEnableAt=0;  // akan diset ulang saat stabilisasi selesai
    Serial.println("[CMD] START");
  }
  if (clientRequest.indexOf("/stop")!=-1) { manualStop=true; cavRecoveryUntil=0; cavPendSince=millis(); Serial.println("[CMD] STOP"); }

  if (clientRequest.indexOf("/sensor?")!=-1) {
    int idPos=clientRequest.indexOf("id=");
    int enPos=clientRequest.indexOf("en=");
    if (idPos!=-1&&enPos!=-1) {
      char id=clientRequest.charAt(idPos+3);
      int en=clientRequest.substring(enPos+3,enPos+4).toInt();
      int sid=-1;
      if      (id=='P'||id=='p') sid=SENSOR_P;
      else if (id=='V'||id=='v') sid=SENSOR_V;
      else if (id=='I'||id=='i') sid=SENSOR_I;
      if (sid!=-1) {
        sensorEnable[sid]=(en!=0);
        updateFuzzyMode();
        if (sid==SENSOR_P) { s_p=0; pres_cached=0; initMovingAvg(); }
        if (sid==SENSOR_I) { s_i=0; current_cached=0; cur_neg_count=0; }
        if (sid==SENSOR_V) {
          s_ax=s_ay=0; s_az=CFG_VIB_GRAVITY;
          vib_baseline=CFG_VIB_GRAVITY; vib_error_count=0;
        }
        pid_integral=0; pid_prev_error=0; pid_deriv_filt=0;
        String newMode=determinePIDMode();
        Serial.printf("[SENSOR] %s→%s | Mode:%s→%s | w:V=%.2f P=%.2f I=%.2f\n",
                      SENSOR_NAME[sid],sensorEnable[sid]?"ON":"OFF",
                      pidActiveMode.c_str(),newMode.c_str(),fw_V,fw_P,fw_I);
      }
    }
    sendSensorStatus(activeClient); activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/sstatus")!=-1) {
    sendSensorStatus(activeClient); activeClient.stop(); clientActive=false; return;
  }
  if (clientRequest.indexOf("/mfdata")!=-1) {
    sendMFData(activeClient); activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/mpucal")!=-1) {
    String resp="{\"ok\":true,";
    if (clientRequest.indexOf("cmd=start")!=-1) {
      if (pwmActual>0) resp="{\"ok\":false,\"err\":\"Matikan pompa (PWM=0)\",";
      else if (!mpuOK) resp="{\"ok\":false,\"err\":\"MPU6050 tidak ditemukan\",";
      else { bool ok=calibrateMPUWeb(); resp+=ok?"\"action\":\"calibrated\",":"{\"ok\":false,\"err\":\"Kalibrasi gagal\","; }
    } else if (clientRequest.indexOf("cmd=reset")!=-1) {
      resetMPUCalibration(); resp+="\"action\":\"reset\",";
    } else resp+="\"action\":\"status\",";
    resp+="\"done\":"  +String(mpu_cal_done?"true":"false")+",";
    resp+="\"ax\":"    +String(ax_off,5)+",";
    resp+="\"ay\":"    +String(ay_off,5)+",";
    resp+="\"az\":"    +String(az_off,5)+",";
    resp+="\"mpuOK\":"+String(mpuOK?"true":"false")+",";
    resp+="\"pwm\":"   +String(pwmActual)+"}";
    activeClient.println("HTTP/1.1 200 OK");
    activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *");
    activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(resp);
    activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/acscal")!=-1) {
    String resp="{\"ok\":true,";
    if (clientRequest.indexOf("cmd=zero")!=-1) {
      if (pwmActual>0) resp="{\"ok\":false,\"err\":\"Matikan pompa (arus=0)\",";
      else { bool ok=calibrateACSZero(); resp+=ok?"\"action\":\"zero\",":"{\"ok\":false,\"err\":\"Zero cal gagal\","; }
    } else if (clientRequest.indexOf("cmd=span")!=-1) {
      int ri=clientRequest.indexOf("ref=");
      if (ri==-1) resp="{\"ok\":false,\"err\":\"Parameter ref= tidak ada\",";
      else {
        float ref=clientRequest.substring(ri+4).toFloat();
        if (ref<0.05f||ref>10.0f) resp="{\"ok\":false,\"err\":\"ref tidak valid (0.05-10 A)\",";
        else {
          bool ok=calibrateACSSpan(ref);
          resp+=ok?("\"action\":\"span\",\"ref\":"+String(ref,3)+","):"{\"ok\":false,\"err\":\"Span gagal (gain di luar range)\",";
        }
      }
    } else if (clientRequest.indexOf("cmd=reset")!=-1) {
      resetACSCalibration(); resp+="\"action\":\"reset\",";
    } else resp+="\"action\":\"status\",";
    resp+="\"done\":"   +String(acs_cal_done?"true":"false")+",";
    resp+="\"offset\":"+String(acs_offset,5)+",";
    resp+="\"gain\":"  +String(acs_cal_gain,5)+",";
    resp+="\"iNow\":"  +String(g_current,4)+",";
    resp+="\"pwm\":"   +String(pwmActual)+"}";
    activeClient.println("HTTP/1.1 200 OK");
    activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *");
    activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(resp);
    activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/pcal")!=-1) {
    String resp="{\"ok\":true,";
    if (clientRequest.indexOf("cmd=zero")!=-1) {
      if (pwmActual>0&&!manualStop) resp="{\"ok\":false,\"err\":\"Stop pompa (PWM=0)\",";
      else { calibratePressureZero(); resp+="\"action\":\"zero\","; }
    } else if (clientRequest.indexOf("cmd=span")!=-1) {
      int ri=clientRequest.indexOf("ref=");
      if (ri==-1) resp="{\"ok\":false,\"err\":\"missing ref= parameter\",";
      else {
        float ref=clientRequest.substring(ri+4).toFloat();
        if (ref<1.0f||ref>CFG_P_MAX_KPA) resp="{\"ok\":false,\"err\":\"ref out of range\",";
        else { calibratePressureSpan(ref); resp+="\"action\":\"span\",\"ref\":"+String(ref,2)+","; }
      }
    } else if (clientRequest.indexOf("cmd=reset")!=-1) {
      resetPressureCalibration(); resp+="\"action\":\"reset\",";
    } else resp+="\"action\":\"status\",";
    resp+="\"zero\":"   +String(p_zero_offset_kPa,3)+",";
    resp+="\"gain\":"   +String(p_gain,4)+",";
    resp+="\"done\":"   +String(p_cal_done?"true":"false")+",";
    resp+="\"method\":\""+String(p_cal_method)+"\",";
    resp+="\"pNow\":"   +String(g_pressure_kPa,2)+",";
    resp+="\"pwm\":"    +String(pwmActual)+"}";
    activeClient.println("HTTP/1.1 200 OK");
    activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *");
    activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(resp);
    activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/log")!=-1&&
      clientRequest.indexOf("/logstatus")==-1&&
      clientRequest.indexOf("/logcsv")==-1) {
    if (clientRequest.indexOf("cmd=start")!=-1) {
      int ii=clientRequest.indexOf("interval=");
      if (ii!=-1) { unsigned long nv=clientRequest.substring(ii+9).toInt(); if (nv>=100&&nv<=60000) logInterval=nv; }
      logging=true; lastLog=millis();
      Serial.printf("[LOG] Start interval=%lu ms\n",logInterval);
    }
    if (clientRequest.indexOf("cmd=stop") !=-1) { logging=false; Serial.println("[LOG] Stop"); }
    if (clientRequest.indexOf("cmd=clear")!=-1) { logging=false; csvHead=0; csvCount=0; Serial.println("[LOG] Clear"); }
    activeClient.println("HTTP/1.1 200 OK"); activeClient.println("Content-Type: text/plain");
    activeClient.println("Access-Control-Allow-Origin: *"); activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(logging?"LOGGING":"STOPPED");
    activeClient.stop(); clientActive=false; return;
  }
  if (clientRequest.indexOf("/logstatus")!=-1) { sendLogStatus(activeClient); activeClient.stop(); clientActive=false; return; }
  if (clientRequest.indexOf("/logcsv")!=-1) {
    if (csvCount==0) {
      activeClient.println("HTTP/1.1 200 OK"); activeClient.println("Content-Type: text/plain");
      activeClient.println("Connection: close"); activeClient.println();
      activeClient.print("# Tidak ada data log."); activeClient.stop(); clientActive=false; return;
    }
    sendCSVHeader(activeClient);
    csvStreamStart=(csvCount<CFG_CSV_BUF_SIZE)?0:csvHead;
    csvStreamTotal=csvCount; csvStreamIdx=0; csvStreaming=true;
    clientActive=false; return;
  }

  if (clientRequest.indexOf("/prelearn")!=-1) {
    pBaselineReady=false; pLearnStart=0; pLearnAccum=0; pLearnN=0;
    pid_integral=0; pid_prev_error=0; pid_deriv_filt=0;
    Serial.println("[AUTO-P] belajar ulang baseline tekanan...");
    String j="{\"ok\":true,\"relearn\":true}";
    activeClient.println("HTTP/1.1 200 OK"); activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *"); activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(j); activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/isp")!=-1) {
    int qi=clientRequest.indexOf("pct=");
    if (qi!=-1) {
      int v=constrain(clientRequest.substring(qi+4).toInt(), CFG_I_SPEED_MIN, 100);
      curSpeedTargetPct=v;
      nvs.begin("smartpump", false); nvs.putInt("ispd", v); nvs.end();
      pid_integral=0; pid_prev_error=0; pid_deriv_filt=0;
      Serial.printf("[ISP] target kecepatan Mode Arus -> %d%% (sp=%.3fA)\n",
                    v, iSlopeLearned*v + CFG_I_MODEL_OFFSET);
    }
    float spA = iSlopeLearned*curSpeedTargetPct + CFG_I_MODEL_OFFSET;
    String j="{\"ok\":true,\"pct\":"+String(curSpeedTargetPct)+",\"spA\":"+String(spA,3)+"}";
    activeClient.println("HTTP/1.1 200 OK"); activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *"); activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(j); activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/vtest")!=-1) {
    int ei=clientRequest.indexOf("en=");
    if (ei!=-1) testFixedMode = (clientRequest.charAt(ei+3)=='1');
    int pi=clientRequest.indexOf("pwm=");
    if (pi!=-1) testHoldPwm = constrain(clientRequest.substring(pi+4).toInt(), CFG_PWM_MIN, CFG_PWM_MAX);
    if (testFixedMode) { pwmTarget=testHoldPwm; pid_integral=0; pid_prev_error=0; pid_deriv_filt=0; }
    else { pid_integral=0; pid_prev_error=0; pid_deriv_filt=0; }
    Serial.printf("[UJI] tutup-katup: %s PWM=%d\n", testFixedMode?"AKTIF":"OFF", testHoldPwm);
    String j="{\"ok\":true,\"en\":"+String(testFixedMode?1:0)+",\"pwm\":"+String(testHoldPwm)+"}";
    activeClient.println("HTTP/1.1 200 OK"); activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *"); activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(j); activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/ctrl")!=-1) {
    int pi=clientRequest.indexOf("primary=");
    if (pi!=-1) {
      char p=clientRequest.charAt(pi+8);
      if (p=='v') p='V'; if (p=='p') p='P'; if (p=='i') p='I';
      if (p=='V'||p=='P'||p=='I') {
        ctrlPrimary=p;
        // ctrlPrimary hardcoded di setup() dari CFG_CTRL_PRIMARY — tidak perlu NVS
        pid_integral=0; pid_prev_error=0; pid_deriv_filt=0;
        Serial.printf("[CTRL] prioritas -> %c\n", ctrlPrimary);
      }
    }
    String j="{\"ok\":true,\"primary\":\""+String(ctrlPrimary)+"\"}";
    activeClient.println("HTTP/1.1 200 OK"); activeClient.println("Content-Type: application/json");
    activeClient.println("Access-Control-Allow-Origin: *"); activeClient.println("Connection: close"); activeClient.println();
    activeClient.print(j); activeClient.stop(); clientActive=false; return;
  }

  if (clientRequest.indexOf("/data")!=-1) { sendData(activeClient); activeClient.stop(); clientActive=false; return; }
  if (clientRequest.indexOf("GET")  !=-1) sendHTML(activeClient);
  activeClient.stop(); clientActive=false;
}

// Response helpers 
void sendSensorStatus(WiFiClient& c) {
  String j = "{";
  j += "\"P\":" + String(sensorEnable[SENSOR_P] ? "true" : "false") + ",";
  j += "\"V\":" + String(sensorEnable[SENSOR_V] ? "true" : "false") + ",";
  j += "\"I\":" + String(sensorEnable[SENSOR_I] ? "true" : "false") + ",";
  j += "\"mode\":\"" + fuzzyMode + "\",";
  j += "\"pidMode\":\"" + pidActiveMode + "\",";
  j += "\"mask\":"   + String(getEnableMask()) + ",";
  j += "\"wV\":"     + String(fw_V, 2) + ",";
  j += "\"wP\":"     + String(fw_P, 2) + ",";
  j += "\"wI\":"     + String(fw_I, 2) + ",";
  j += "\"nActive\":"+ String((int)sensorEnable[SENSOR_P]+(int)sensorEnable[SENSOR_V]+(int)sensorEnable[SENSOR_I]);
  j += "}";
  c.println("HTTP/1.1 200 OK"); c.println("Content-Type: application/json");
  c.println("Access-Control-Allow-Origin: *"); c.println("Connection: close"); c.println();
  c.print(j);
}

void sendMFData(WiFiClient& c) {
  String j = "{";
  j += "\"P\":{";
  // Bentuk MF sesuai pdanger: LOW turun dari SEVERE→EARLY, HIGH naik dari SEVERE→EARLY (simetris).
  // max = EARLY*3 agar tekanan operasi normal (0.5-2.5 kPa) juga terlihat di grafik.
  j +=   "\"unit\":\"kPa\",\"min\":0,\"max\":" + String(CFG_P_SUCTION_EARLY*3.0f,2) + ",";
  j +=   "\"low\":["  + String(CFG_MFP_LOW_FULL,3)   + "," + String(CFG_MFP_LOW_ZERO,3)   + "],";
  j +=   "\"norm\":[" + String(CFG_MFP_NORM_L0,3)    + "," + String(CFG_MFP_NORM_LPEAK,3) + "," + String(CFG_MFP_NORM_RPEAK,3) + "],";
  j +=   "\"high\":[" + String(CFG_MFP_HIGH_ZERO,3)  + "," + String(CFG_MFP_HIGH_FULL,3)  + "],";
  j +=   "\"sp\":"    + String(CFG_P_SUCTION_EARLY,3) + ",\"w\":" + String(fw_P,2);
  j += "},";
  j += "\"V\":{";
  j +=   "\"unit\":\"m/s²\",\"min\":0,\"max\":" + String(CFG_MFV_HIGH_FULL,2) + ",";
  j +=   "\"low\":["  + String(CFG_MFV_LOW_FULL,2)  + "," + String(CFG_MFV_LOW_ZERO,2)  + "],";
  j +=   "\"norm\":[" + String(CFG_MFV_MED_L0,2)    + "," + String(CFG_MFV_MED_LPEAK,2) + "," + String(CFG_MFV_MED_RPEAK,2) + "],";
  j +=   "\"high\":[" + String(CFG_MFV_HIGH_ZERO,2) + "," + String(CFG_MFV_HIGH_FULL,2) + "],";
  j +=   "\"sp\":"    + String(CFG_VIB_SETPOINT,2)  + ",\"w\":" + String(fw_V,2);
  j += "},";
  j += "\"I\":{";
  j +=   "\"unit\":\"A\",\"min\":0,\"max\":2,";
  j +=   "\"low\":["  + String(CFG_MFI_LOW_FULL,2)  + "," + String(CFG_MFI_LOW_ZERO,2)  + "],";
  j +=   "\"norm\":[" + String(CFG_MFI_NORM_L0,2)   + "," + String(CFG_MFI_NORM_LPEAK,2)+ "," + String(CFG_MFI_NORM_RPEAK,2) + "],";
  j +=   "\"high\":[" + String(CFG_MFI_HIGH_ZERO,2) + "," + String(CFG_MFI_HIGH_FULL,2) + "],";
  j +=   "\"sp\":"    + String(iSlopeLearned*curSpeedTargetPct + CFG_I_MODEL_OFFSET,3) + ",\"w\":" + String(fw_I,2);
  j += "},";
  j += "\"weights\":{\"P\":" + String(fw_P,2) + ",\"V\":" + String(fw_V,2) + ",\"I\":" + String(fw_I,2) + "},";
  { String _o = (ctrlPrimary=='I') ? "I>V>P" : (ctrlPrimary=='V') ? "V>P>I" : "P>V>I";
    j += "\"pidPriority\":\"" + _o + "\","; }
  j += "\"pidMode\":\"" + pidActiveMode + "\"";
  j += "}";
  c.println("HTTP/1.1 200 OK"); c.println("Content-Type: application/json");
  c.println("Access-Control-Allow-Origin: *"); c.println("Connection: close"); c.println();
  c.print(j);
}

void sendLogStatus(WiFiClient& c) {
  String j = "{";
  j += "\"logging\":"  + String(logging ? "true":"false") + ",";
  j += "\"count\":"    + String(csvCount) + ",";
  j += "\"maxRows\":"  + String(CFG_CSV_BUF_SIZE) + ",";
  j += "\"interval\":" + String(logInterval) + ",";
  j += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  j += "\"usedKB\":"   + String((csvCount * (int)sizeof(LogRow)) / 1024) + ",";
  j += "\"chipTemp\":" + String(chipTemp, 1) + ",";
  j += "\"throttle\":" + String(thermalThrottle ? "true":"false") + ",";
  j += "\"cpuMhz\":"   + String(getCpuFrequencyMhz());
  j += "}";
  c.println("HTTP/1.1 200 OK"); c.println("Content-Type: application/json");
  c.println("Access-Control-Allow-Origin: *"); c.println("Connection: close"); c.println();
  c.print(j);
}

void sendCSVHeader(WiFiClient& c) {
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: text/csv; charset=utf-8");
  c.println("Content-Disposition: attachment; filename=\"smart_pump_log.csv\"");
  c.println("Access-Control-Allow-Origin: *"); c.println("Connection: close"); c.println();
  c.println("Time_ms,Vibration_m/s2,Current_A,Pressure_kPa,Pressure_bar,PWM_%,CavIdx,CavState,PIDout,SensorStatus,FuzzyMode,PID_Mode,EnableMask");
}

void sendData(WiFiClient& c) {
  String j = "{";
  j += "\"v\":"        + String(g_vibration, 3) + ",";
  j += "\"i\":"        + String(g_current, 3) + ",";
  j += "\"p\":"        + String(g_pressure_kPa, 2) + ",";
  j += "\"p_bar\":"    + String(g_pressure_kPa / 100.0f, 4) + ",";
  j += "\"pwm\":"      + String(pwmActual) + ",";
  j += "\"stop\":"     + String(manualStop ? "true":"false") + ",";
  j += "\"cav\":"      + String(g_cavitation, 3) + ",";
  j += "\"cs\":"       + String(g_cavState) + ",";
  j += "\"pid\":"      + String(g_pid_output, 3) + ",";
  j += "\"rdy\":"      + String(systemReady ? "true":"false") + ",";
  j += "\"sens\":\""   + sensorStatus + "\",";
  j += "\"temp\":"     + String(chipTemp, 1) + ",";
  j += "\"thr\":"      + String(thermalThrottle ? "true":"false") + ",";
  j += "\"cpu\":"      + String(getCpuFrequencyMhz()) + ",";
  j += "\"up\":"       + String(millis()) + ",";
  j += "\"enP\":"      + String(sensorEnable[SENSOR_P] ? "true":"false") + ",";
  j += "\"enV\":"      + String(sensorEnable[SENSOR_V] ? "true":"false") + ",";
  j += "\"enI\":"      + String(sensorEnable[SENSOR_I] ? "true":"false") + ",";
  j += "\"fmode\":\"" + fuzzyMode + "\",";
  j += "\"ctrlPri\":\"" + String(ctrlPrimary) + "\",";
  j += "\"vtEn\":" + String(testFixedMode?1:0) + ",";
  j += "\"vtPwm\":" + String(testHoldPwm) + ",";
  j += "\"iSpd\":"    + String(curSpeedTargetPct) + ",";
  j += "\"iSpA\":"    + String(iSlopeLearned*curSpeedTargetPct + CFG_I_MODEL_OFFSET, 3) + ",";
  j += "\"iSpdEff\":" + String(ispEffPct) + ",";
  j += "\"iSlope\":"  + String(iSlopeLearned, 4) + ",";
  j += "\"pSp\":" + String(pressureSetpoint, 2) + ",";
  j += "\"pBase\":" + String(pBaseline, 2) + ",";
  j += "\"pRdy\":" + String(pBaselineReady ? "true":"false") + ",";
  j += "\"vbase\":"    + String(vib_baseline, 3) + ",";
  j += "\"pcalZero\":" + String(p_zero_offset_kPa, 3) + ",";
  j += "\"pcalGain\":" + String(p_gain, 4) + ",";
  j += "\"pcalDone\":" + String(p_cal_done ? "true" : "false") + ",";
  j += "\"pcalMethod\":\"" + String(p_cal_method) + "\",";
  j += "\"pidMode\":\"" + pidActiveMode + "\",";
  j += "\"pidSp\":"    + String(pidSetpoint, 3) + ",";
  j += "\"pidPv\":"    + String(pidPV, 3) + ",";
  j += "\"wV\":"       + String(fw_V, 2) + ",";
  j += "\"wP\":"       + String(fw_P, 2) + ",";
  j += "\"wI\":"       + String(fw_I, 2) + ",";
  j += "\"lps\":"      + String(loops_per_sec, 0) + ",";
  j += "\"mbps\":"     + String(mbps_value, 4) + ",";
  j += "\"heap\":"     + String(ESP.getFreeHeap()) + ",";
  j += "\"acsDone\":"  + String(acs_cal_done ? "true":"false") + ",";
  j += "\"acsOff\":"   + String(acs_offset, 4) + ",";
  j += "\"acsGain\":"  + String(acs_cal_gain, 4) + ",";
  j += "\"mpuDone\":"  + String(mpu_cal_done ? "true":"false") + ",";
  j += "\"mpuAx\":"    + String(ax_off, 4) + ",";
  j += "\"mpuAy\":"    + String(ay_off, 4) + ",";
  j += "\"mpuAz\":"    + String(az_off, 4) + ",";
  j += "\"spVcfg\":"   + String(CFG_VIB_SETPOINT, 2) + ",";
  j += "\"spPcfg\":"   + String(CFG_PRESSURE_REF, 3);
  j += "}";
  c.println("HTTP/1.1 200 OK"); c.println("Content-Type: application/json");
  c.println("Access-Control-Allow-Origin: *"); c.println("Connection: close"); c.println();
  c.print(j);
  bytes_sent_total += j.length();
}

void sendHTML(WiFiClient& c) {
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: text/html; charset=utf-8");
  c.println("Connection: close");
  c.println();
  c.println(HTML_PAGE);
}
