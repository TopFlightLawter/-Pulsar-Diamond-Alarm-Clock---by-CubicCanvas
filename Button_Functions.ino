// ======================================================================================
// BUTTON FUNCTIONS
// Physical button handling and long press detection
// ======================================================================================

void handleButtons() {
  snoozeButton.loop();
  killSwitch.loop();
  testButton.loop();
  setAlarmButton.loop();

  // Test Button = Enable alarms + immediate trigger
  if (testButton.isPressed()) {
    Serial.println("Test pressed: Alarms enabled + alarm triggered");
    setAlarmsEnabled(true);
    stopPulseMode();
    alarmTriggered = true;
    alarmStage = 1;
    alarmStageTime = millis();
  }

  // Snooze Button = Stop alarm + 60s snooze
  if (snoozeButton.isPressed()) {
    Serial.println("Snooze pressed: 60s snooze activated");
    stopPulseMode();
    deactivateRelay();
    alarmStage = 0;
    snoozeActivated = true;
    snoozeStartTime = millis();
  }

  // Kill Switch = Disable all alarms
  if (killSwitch.isPressed()) {
    Serial.println("Kill switch pressed: Alarms disabled");
    setAlarmsEnabled(false);
    stopPulseMode();
    deactivateRelay();
    alarmTriggered = false;
    snoozeActivated = false;
    alarmStage = 0;
  }

  // Set Alarm Button = Enable alarms only
  if (setAlarmButton.isPressed()) {
    Serial.println("Set pressed: Alarms enabled");
    setAlarmsEnabled(true);
  }
}

void handleLongPress() {
  // Test button (GPIO 15) - cycle backward through patterns
  if (testButton.getState() == LOW) {
    if (!buttonWasPressed[0]) {
      buttonPressStart[0] = millis();
      buttonWasPressed[0] = true;
    } else if (!buttonLongPressed[0] && millis() - buttonPressStart[0] > 2000) {
      PatternManager::prevPattern();
      Serial.print("Pattern: ");
      Serial.println(PatternManager::getCurrentPatternName());
      showPatternOnOLED(PatternManager::currentPattern);
      buttonLongPressed[0] = true;
    }
  } else {
    buttonWasPressed[0] = false;
    buttonLongPressed[0] = false;
  }

  // Set button (GPIO 17) - cycle forward through patterns
  if (setAlarmButton.getState() == LOW) {
    if (!buttonWasPressed[1]) {
      buttonPressStart[1] = millis();
      buttonWasPressed[1] = true;
    } else if (!buttonLongPressed[1] && millis() - buttonPressStart[1] > 2000) {
      PatternManager::nextPattern();
      Serial.print("Pattern: ");
      Serial.println(PatternManager::getCurrentPatternName());
      showPatternOnOLED(PatternManager::currentPattern);
      buttonLongPressed[1] = true;
    }
  } else {
    buttonWasPressed[1] = false;
    buttonLongPressed[1] = false;
  }

  // Snooze button (GPIO 18) - kill switch (5s hold)
  if (snoozeButton.getState() == LOW) {
    if (!buttonWasPressed[2]) {
      buttonPressStart[2] = millis();
      buttonWasPressed[2] = true;
    } else if (!buttonLongPressed[2] && millis() - buttonPressStart[2] > 5000) {
      Serial.println("Kill switch long press activated");
      setAlarmsEnabled(false);
      stopPulseMode();
      deactivateRelay();
      alarmTriggered = false;
      snoozeActivated = false;
      alarmStage = 0;
      killSwitchAnimation();
      buttonLongPressed[2] = true;
    }
  } else {
    buttonWasPressed[2] = false;
    buttonLongPressed[2] = false;
  }
}