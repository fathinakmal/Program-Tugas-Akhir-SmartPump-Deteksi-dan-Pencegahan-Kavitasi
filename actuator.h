#pragma once

// actuator.h — State machine relay non-blocking (step PWM)

void scheduleRelay(bool goDown) {
  if (relayState != 0) return;
  relayDir = goDown; relayPending = true;
}

void tickRelay() {
  if (!relayPending && relayState == 0) return;
  switch (relayState) {
    case 0:
      if (!relayPending) return;
      relayPending = false;
      if (!relayDir) { digitalWrite(CFG_PIN_RELAY_UP, HIGH);   relayState = 1; }
      else           { digitalWrite(CFG_PIN_RELAY_DOWN, HIGH); relayState = 3; }
      relayTimer = millis(); break;
    case 1:
      if (millis()-relayTimer >= CFG_RELAY_PULSE_MS)
        { digitalWrite(CFG_PIN_RELAY_UP, LOW); relayState=2; relayTimer=millis(); }
      break;
    case 2: if (millis()-relayTimer >= CFG_RELAY_SETTLE_UP_MS)   relayState=0; break;
    case 3:
      if (millis()-relayTimer >= CFG_RELAY_PULSE_MS)
        { digitalWrite(CFG_PIN_RELAY_DOWN, LOW); relayState=4; relayTimer=millis(); }
      break;
    case 4: if (millis()-relayTimer >= CFG_RELAY_SETTLE_DOWN_MS) relayState=0; break;
  }
}

bool relayBusy() { return relayState != 0; }
