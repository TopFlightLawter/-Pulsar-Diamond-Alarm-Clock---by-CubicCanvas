// ======================================================================================
// TIME FUNCTIONS
// NTP synchronization and time formatting
// ======================================================================================

void synchronizeTime() {
  Serial.println("Syncing time with NTP...");
  configTime(Config::GMT_OFFSET_SEC, Config::DAYLIGHT_OFFSET_SEC, "pool.ntp.org");
  struct tm timeInfo = { 0 };
  int attempts = 0;
  const int maxAttempts = 5;

  if (oledAvailable && !showSplash) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Syncing time with NTP"));
    display.display();
  }

  while (!getLocalTime(&timeInfo) && attempts < maxAttempts) {
    Serial.println("NTP retry...");
    delay(1000);
    attempts++;

    if (oledAvailable && !showSplash) {
      display.setCursor(0, 20);
      display.print(F("Attempt "));
      display.print(attempts);
      display.print(F("/"));
      display.print(maxAttempts);
      display.display();
    }
  }

  if (attempts < maxAttempts) {
    Serial.println("Time synchronized.");
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeInfo);
    Serial.print("Current time: ");
    Serial.println(timeStr);
    rtcMillis = millis();
    lastSyncTime = millis();

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Time synced:"));
      display.setCursor(0, 16);
      display.print(timeStr);
      display.display();
      delay(1000);
    }
  } else {
    Serial.println("Time sync failed.");

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Time sync failed"));
      display.display();
      delay(1000);
    }
  }
}

String getFormattedTime() {
  struct tm timeInfo = { 0 };
  if (!getLocalTime(&timeInfo)) {
    unsigned long currentMillis = millis();
    unsigned long elapsedMillis = currentMillis - rtcMillis;
    timeInfo.tm_sec += elapsedMillis / 1000;
    rtcMillis = currentMillis;
    mktime(&timeInfo);
  }

  int hour = timeInfo.tm_hour % 12;
  if (hour == 0) hour = 12;
  String ampm = (timeInfo.tm_hour >= 12) ? "pm" : "am";

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%d:%02d %s", hour, timeInfo.tm_min, ampm.c_str());
  return String(buffer);
}

String getNextAlarmTime() {
  struct tm timeInfo = { 0 };
  if (!getLocalTime(&timeInfo)) {
    return "RTC time unavailable.";
  }

  int currentHour = timeInfo.tm_hour;
  int currentMinute = timeInfo.tm_min;

  if (!alarmsEnabled) {
    return "Alarms are disabled.";
  }

  if (currentHour < Config::ALARM_START_HOUR || (currentHour == Config::ALARM_START_HOUR && currentMinute < Config::ALARM_START_MINUTE)) {
    return "9:30 PM";
  } else if (currentHour == Config::ALARM_START_HOUR && currentMinute >= Config::ALARM_START_MINUTE && currentMinute <= 57) {
    int nextMinute = ((currentMinute / 4) + 1) * 4 + 2;
    if (nextMinute > 59) nextMinute = 30;
    return "Today at 9:" + String(nextMinute) + " PM";
  } else if (currentHour == Config::ALARM_END_HOUR && currentMinute <= Config::ALARM_END_MINUTE) {
    int nextMinute = ((currentMinute / 4) + 1) * 4 + 2;
    if (nextMinute > 59) nextMinute = 2;
    return "Today at 10:" + String(nextMinute) + " PM";
  } else {
    return "No alarms for the rest of the day.";
  }
}

String getTimeRemaining() {
  struct tm timeInfo = { 0 };
  if (!getLocalTime(&timeInfo)) {
    return "Unavailable";
  }

  struct tm nextAlarm = timeInfo;

  if (!alarmsEnabled) {
    return "Alarms disabled.";
  }

  if (timeInfo.tm_hour < Config::ALARM_START_HOUR || (timeInfo.tm_hour == Config::ALARM_START_HOUR && timeInfo.tm_min < Config::ALARM_START_MINUTE)) {
    nextAlarm.tm_hour = Config::ALARM_START_HOUR;
    nextAlarm.tm_min = Config::ALARM_START_MINUTE;
  } else if (timeInfo.tm_hour == Config::ALARM_START_HOUR && timeInfo.tm_min >= Config::ALARM_START_MINUTE && timeInfo.tm_min <= 57) {
    nextAlarm.tm_min = ((timeInfo.tm_min / 4) + 1) * 4 + 2;
    if (nextAlarm.tm_min > 59) {
      nextAlarm.tm_min = 2;
      nextAlarm.tm_hour = Config::ALARM_END_HOUR;
    }
  } else if (timeInfo.tm_hour == Config::ALARM_END_HOUR && timeInfo.tm_min <= Config::ALARM_END_MINUTE) {
    nextAlarm.tm_min = ((timeInfo.tm_min / 4) + 1) * 4 + 2;
    if (nextAlarm.tm_min > 59) {
      nextAlarm.tm_min = 2;
      nextAlarm.tm_hour++;
    }
  } else {
    return "No alarms for today.";
  }

  time_t currentTime = mktime(&timeInfo);
  time_t alarmTime = mktime(&nextAlarm);

  if (alarmTime < currentTime) {
    return "No alarms for today.";
  }

  long diffSeconds = difftime(alarmTime, currentTime);
  int hours = diffSeconds / 3600;
  int minutes = (diffSeconds % 3600) / 60;

  String remaining = "";
  if (hours > 0) {
    remaining += String(hours) + "h ";
  }
  remaining += String(minutes) + "m";

  return remaining;
}