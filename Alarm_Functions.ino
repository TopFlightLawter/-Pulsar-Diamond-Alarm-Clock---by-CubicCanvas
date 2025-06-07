// ======================================================================================
// ALARM FUNCTIONS
// Alarm logic, test modes, and system control
// ======================================================================================

void setAlarmsEnabled(bool enabled) {
  alarmsEnabled = enabled;
  digitalWrite(Config::LED_PIN, enabled ? HIGH : LOW);
  Serial.println("Alarms " + String(enabled ? "ENABLED" : "DISABLED"));

  // If alarms disabled and currently active, turn everything off
  if (!enabled && (relayState || pulseMode)) {
    stopPulseMode();
    deactivateRelay();
    alarmTriggered = false;
    alarmStage = 0;
  }
}

void handleAlarmLogic() {
  // Get current time for alarm logic
  struct tm timeInfo = { 0 };
  if (!getLocalTime(&timeInfo)) {
    // Use RTC fallback
    unsigned long currentMillis = millis();
    unsigned long elapsedMillis = currentMillis - rtcMillis;
    timeInfo.tm_sec += elapsedMillis / 1000;
    rtcMillis = currentMillis;
    mktime(&timeInfo);
  }

  int currentHour = timeInfo.tm_hour;
  int currentMinute = timeInfo.tm_min;

  // Pre-connect WiFi at 9:29 PM for alarm window sync
  if (currentHour == Config::ALARM_START_HOUR - 1 && currentMinute == 59 && WiFi.status() != WL_CONNECTED) {
    if (connectToWiFi()) {
      synchronizeTime();
      Serial.println("Pre-alarm WiFi sync complete.");
    } else {
      Serial.println("Pre-alarm WiFi sync failed.");
    }
  }

  // Check if we're in alarm window
  bool inAlarmWindow = ((currentHour == Config::ALARM_START_HOUR && currentMinute >= Config::ALARM_START_MINUTE) || (currentHour == Config::ALARM_END_HOUR && currentMinute <= Config::ALARM_END_MINUTE));

  // Alarm triggering logic (minute % 4 == 2)
  if (alarmsEnabled && inAlarmWindow && !quickTestActive && !snoozeActivated) {
    if ((currentMinute % 4 == 2) && !alarmTriggered && alarmStage == 0) {
      Serial.println("Alarm triggered: starting pre-flash sequence");
      alarmTriggered = true;
      alarmStage = 1;
      alarmStageTime = millis();
    }
  }

  // Reset trigger when we leave the 4-minute block
  if (currentMinute % 4 == 3 && alarmTriggered && alarmStage == 0) {
    alarmTriggered = false;
  }

  // Handle alarm stage progression
  if (alarmStage > 0) {
    unsigned long now = millis();
    switch (alarmStage) {
      case 1:  // Initial flash: Turn relay ON for 0.25s
        activateRelay();
        alarmStage = 2;
        alarmStageTime = now;
        break;

      case 2:  // Wait 0.25s, then turn OFF
        if (now - alarmStageTime >= 250) {
          deactivateRelay();
          alarmStage = 3;
          alarmStageTime = now;
        }
        break;

      case 3:  // Stay OFF for 3 seconds
        if (now - alarmStageTime >= 3000) {
          startPulseMode();
          alarmStage = 4;  // Pulse mode active
        }
        break;

      case 4:
        // Pulse mode is handled by handlePulseMode()
        break;

      default:
        break;
    }
  }
}

void handleQuickTest() {
  if (!quickTestActive) return;

  unsigned long elapsed = millis() - quickTestStartTime;

  if (quickTestPhase == 0) {
    // Phase 0: Relay ON for specified interval
    if (elapsed < quickTestInterval) {
      activateRelay();
    } else {
      deactivateRelay();
      quickTestPhase = 1;
      quickTestStartTime = millis();
      quickTestInterval = 100;  // OFF interval
    }
  } else if (quickTestPhase == 1) {
    // Phase 1: Relay OFF for specified interval
    if (elapsed < quickTestInterval) {
      // Relay is already off
    } else {
      quickTestPhase = 0;  // Back to phase 0
      quickTestToggleCount++;
      quickTestStartTime = millis();

      // End test after 5 toggles
      if (quickTestToggleCount >= 5) {
        quickTestActive = false;
        quickTestToggleCount = 0;
        Serial.println("Quick test completed");
      }
    }
  }
}

void handleSingleFlash() {
  if (!singleFlashActive) return;

  unsigned long elapsed = millis() - singleFlashTime;

  if (elapsed < 500) {  // ON for 500ms
    activateRelay();
  } else {
    deactivateRelay();
    singleFlashActive = false;
    Serial.println("Single flash completed");
  }
}

void handleSnooze() {
  if (!snoozeActivated) return;

  unsigned long elapsed = millis() - snoozeStartTime;

  // Check if snooze time is up
  if (elapsed >= Config::SNOOZE_DURATION) {
    snoozeActivated = false;
    Serial.println("Snooze period ended");
  }
}