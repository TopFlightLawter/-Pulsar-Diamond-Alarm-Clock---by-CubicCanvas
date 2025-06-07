// ======================================================================================
// OLED FUNCTIONS
// Display management and animations
// ======================================================================================

void initOLED() {
  Serial.println("Initializing OLED...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    oledAvailable = false;
    return;
  }

  oledAvailable = true;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  splashStartTime = millis();
  showSplash = true;
  animationFrame = 0;

  showSplashScreen();
  Serial.println("OLED initialized.");
}

void showSplashScreen() {
  if (!oledAvailable) return;

  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - 12 * 6) / 2, 4);
  display.println(F("PULSAR"));

  display.drawLine(0, 20, SCREEN_WIDTH, 20, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor((SCREEN_WIDTH - 10 * 6) / 2, 24);
  display.println(F("DIAMOND"));

  display.setCursor((SCREEN_WIDTH - 13 * 6) / 2, 38);
  display.println(F("QUANTUM MATRIX"));

  drawProgressBar(14, 52, 100, animationFrame * 25);

  if (splashDebug) {
    display.setCursor(0, 56);
    display.print("T:");
    unsigned long elapsed = millis() - splashStartTime;
    display.print(elapsed);
    display.print("ms");
  }

  display.display();
}

void drawProgressBar(int x, int y, int width, int progress) {
  int barWidth = map(progress, 0, 100, 0, width);
  display.drawRect(x, y, width, 6, SSD1306_WHITE);
  display.fillRect(x + 1, y + 1, barWidth - 2, 4, SSD1306_WHITE);
}

void updateOLEDDisplay() {
  if (!oledAvailable) return;

  if (showSplash) {
    if (millis() - animationTime > 500) {
      animationFrame = (animationFrame + 1) % Config::LOADING_FRAMES;
      animationTime = millis();
      showSplashScreen();
    }

    if (millis() - splashStartTime > Config::SPLASH_DURATION) {
      Serial.println("Splash timeout, transitioning to main display");
      showSplash = false;
      lastOledUpdate = 0;
    }

    if (showSplash) return;
  }

  if (millis() - lastOledUpdate < Config::OLED_UPDATE_INTERVAL && !relayState) {
    return;
  }

  lastOledUpdate = millis();
  display.clearDisplay();

  // Top status bar
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  // WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    drawWifiIcon(2, 2, 3);
  } else if (apMode) {
    display.drawTriangle(2, 8, 7, 2, 12, 8, SSD1306_WHITE);
    display.drawPixel(7, 0, SSD1306_WHITE);
  } else {
    display.drawLine(2, 2, 8, 8, SSD1306_WHITE);
    display.drawLine(2, 8, 8, 2, SSD1306_WHITE);
  }

  drawAlarmIcon(SCREEN_WIDTH - 10, 2, alarmsEnabled);
  drawBatteryIcon(SCREEN_WIDTH / 2 - 8, 2, 4);

  // Main time display
  String timeStr = getFormattedTime();
  display.setTextSize(2);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);

  display.setCursor((SCREEN_WIDTH - w) / 2, 16);
  display.print(timeStr);

  drawClockFace(SCREEN_WIDTH - 10, 25, 6);
  display.drawLine(0, 34, SCREEN_WIDTH, 34, SSD1306_WHITE);

  // Alarm info
  display.setTextSize(1);
  String nextAlarm = getNextAlarmTime();

  if (nextAlarm.startsWith("Today at ")) {
    nextAlarm = nextAlarm.substring(9);
  }

  display.setCursor(2, 37);
  display.print(F("Next: "));
  display.print(nextAlarm);

  // Network status
  display.setCursor(2, 47);
  String netStatus;
  if (WiFi.status() == WL_CONNECTED) {
    netStatus = activeNetwork;
    if (netStatus.length() > 16) {
      netStatus = netStatus.substring(0, 13) + "...";
    }
  } else if (apMode) {
    netStatus = "AP: " + String(Config::AP_SSID);
  } else {
    netStatus = "Offline";
  }
  display.print(netStatus);

  // Time remaining
  String remaining = getTimeRemaining();
  if (remaining != "Alarms disabled." && remaining != "No alarms for today." && remaining != "Unavailable") {
    display.setCursor(2, 56);
    display.print(F("In: "));
    display.print(remaining);
  }

  drawRelayStatus(relayState);

  // Snooze countdown
  if (snoozeActivated) {
    unsigned long remainingSnooze = Config::SNOOZE_DURATION - (millis() - snoozeStartTime);
    int snoozeSecs = remainingSnooze / 1000;

    display.fillRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, SSD1306_BLACK);
    display.drawRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

    display.setCursor(2, SCREEN_HEIGHT - 8);
    display.print(F("Snooze: "));
    display.print(snoozeSecs);
    display.print("s");
  }

  // Alarm active visual
  if (relayState || pulseMode) {
    if ((millis() / 500) % 2 == 0) {
      display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
      display.drawRect(2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, SSD1306_WHITE);
    }

    display.setTextSize(2);
    display.setCursor(32, 42);
    display.print(F("ALARM!"));
  }

  display.display();
}

void drawBatteryIcon(int x, int y, int level) {
  display.drawRect(x, y, 10, 6, SSD1306_WHITE);
  display.drawRect(x + 10, y + 1, 2, 4, SSD1306_WHITE);

  for (int i = 0; i < level; i++) {
    display.fillRect(x + 2 + i * 2, y + 2, 1, 2, SSD1306_WHITE);
  }
}

void drawWifiIcon(int x, int y, int strength) {
  for (int i = 0; i < strength; i++) {
    int radius = 2 + (i * 2);
    display.drawCircle(x, y + 7, radius, SSD1306_WHITE);
  }
}

void drawAlarmIcon(int x, int y, bool enabled) {
  display.drawTriangle(x, y + 2, x + 5, y, x + 10, y + 2, SSD1306_WHITE);
  display.drawRoundRect(x, y + 2, 10, 6, 2, SSD1306_WHITE);
  display.drawLine(x + 5, y + 8, x + 5, y + 9, SSD1306_WHITE);

  if (!enabled) {
    display.drawLine(x, y, x + 10, y + 9, SSD1306_WHITE);
  }
}

void drawClockFace(int x, int y, int radius) {
  struct tm timeInfo = { 0 };
  if (!getLocalTime(&timeInfo)) {
    display.drawCircle(x, y, radius, SSD1306_WHITE);
    display.drawLine(x, y, x, y - radius + 2, SSD1306_WHITE);
    display.drawLine(x, y, x + 2, y, SSD1306_WHITE);
    return;
  }

  display.drawCircle(x, y, radius, SSD1306_WHITE);

  float hourAngle = ((timeInfo.tm_hour % 12) * 30 + timeInfo.tm_min * 0.5) * PI / 180.0;
  float minAngle = timeInfo.tm_min * 6 * PI / 180.0;

  int hourHandLength = radius - 3;
  int hourX = x + sin(hourAngle) * hourHandLength;
  int hourY = y - cos(hourAngle) * hourHandLength;
  display.drawLine(x, y, hourX, hourY, SSD1306_WHITE);

  int minHandLength = radius - 1;
  int minX = x + sin(minAngle) * minHandLength;
  int minY = y - cos(minAngle) * minHandLength;
  display.drawLine(x, y, minX, minY, SSD1306_WHITE);
}

void showPatternOnOLED(int patternIndex) {
  if (!oledAvailable) return;

  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - strlen(PatternManager::PATTERNS[patternIndex].name) * 12) / 2, 5);
  display.print(PatternManager::PATTERNS[patternIndex].name);

  display.setTextSize(1);
  display.setCursor(15, 30);
  display.print("Pattern Preview:");

  int xStart = 10;
  int yLine = 45;
  int xSpacing = (SCREEN_WIDTH - 20) / 10;

  for (int i = 0; i < PatternManager::PATTERNS[patternIndex].steps && i < 10; i++) {
    int duration = PatternManager::PATTERNS[patternIndex].sequence[i];
    int lineLength = map(duration, 100, 600, 5, xSpacing - 2);

    if (i % 2 == 0) {
      display.fillRect(xStart + i * xSpacing, yLine, lineLength, 3, SSD1306_WHITE);
    } else {
      for (int j = 0; j < lineLength; j += 2) {
        display.drawPixel(xStart + i * xSpacing + j, yLine + 1, SSD1306_WHITE);
      }
    }
  }

  display.setCursor(5, 56);
  display.print("Hold < to prev, > to next");
  display.display();

  // Demonstrate pattern
  for (int repeat = 0; repeat < 2; repeat++) {
    for (int i = 0; i < PatternManager::PATTERNS[patternIndex].steps; i++) {
      if (i % 2 == 0) {
        display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
      } else {
        display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
      }
      display.display();
      delay(PatternManager::PATTERNS[patternIndex].sequence[i]);
    }
  }

  updateOLEDDisplay();
}

void killSwitchAnimation() {
  if (!oledAvailable) return;

  for (int i = 0; i < 10; i++) {
    display.invertDisplay(true);
    delay(50 - i * 4);
    display.invertDisplay(false);
    delay(50 - i * 4);
  }

  display.clearDisplay();
  display.drawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, SSD1306_WHITE);
  display.display();
  delay(1000);

  display.invertDisplay(true);
  updateOLEDDisplay();
}

void drawRelayStatus(bool active) {
  if (active || pulseMode) {
    int frame = (millis() / 250) % Config::PULSE_FRAMES;

    display.setCursor(SCREEN_WIDTH - 35, 56);
    display.print("BUZZ");

    for (int i = 0; i <= frame; i++) {
      display.print("!");
    }
  }
}