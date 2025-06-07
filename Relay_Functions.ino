// ======================================================================================
// RELAY FUNCTIONS
// Relay control and pulse pattern management
// ======================================================================================

void activateRelay() {
  digitalWrite(Config::RELAY_PIN, HIGH);
  Serial.println("Relay ON");
  relayState = true;
  if (oledAvailable) updateOLEDDisplay();
}

void deactivateRelay() {
  digitalWrite(Config::RELAY_PIN, LOW);
  Serial.println("Relay OFF");
  relayState = false;
  if (oledAvailable) updateOLEDDisplay();
}

void startPulseMode() {
  Serial.println("Starting pulse mode");
  pulseMode = true;
  lastPulseTime = millis();
  pulseState = true;
  activateRelay();
}

void stopPulseMode() {
  Serial.println("Stopping pulse mode");
  pulseMode = false;
  deactivateRelay();
}

void handlePulseMode() {
  if (!pulseMode) return;

  unsigned long currentTime = millis();
  static int patternStep = 0;
  static unsigned long lastStepTime = 0;
  static int activePattern = 0;
  static unsigned long patternResetTime = 0;

  // Random pattern handling
  if (PatternManager::currentPattern == PatternManager::PATTERN_COUNT - 1) {
    if (currentTime - patternResetTime >= 2500) {
      activePattern = random(0, PatternManager::PATTERN_COUNT - 1);
      patternResetTime = currentTime;
      patternStep = 0;
    }
  } else {
    activePattern = PatternManager::currentPattern;
  }

  const VibrationPattern* pattern = &PatternManager::PATTERNS[activePattern];

  // Fallback to standard if no steps
  if (pattern->steps == 0) {
    pattern = &PatternManager::PATTERNS[0];
  }

  // Check timing for next step
  if (currentTime - lastStepTime >= pattern->sequence[patternStep]) {
    patternStep = (patternStep + 1) % pattern->steps;
    lastStepTime = currentTime;

    // Toggle relay based on step (even = ON, odd = OFF)
    if (patternStep % 2 == 0) {
      activateRelay();
    } else {
      deactivateRelay();
    }
  }
}