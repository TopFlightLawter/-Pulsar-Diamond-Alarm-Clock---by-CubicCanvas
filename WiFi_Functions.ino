// ======================================================================================
// WIFI FUNCTIONS
// Network connectivity management
// ======================================================================================

bool connectToNetwork(const char* ssid, const char* password, int maxAttempts) {
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(1000);
    Serial.print(".");
    attempts++;

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Connecting to WiFi"));
      display.setCursor(0, 20);
      display.print(ssid);
      display.setCursor(0, 40);
      for (int i = 0; i < attempts; i++) {
        display.print(".");
      }
      display.display();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected to: ");
    Serial.println(ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    activeNetwork = String(ssid);

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Connected to:"));
      display.setCursor(0, 16);
      display.print(ssid);
      display.setCursor(0, 32);
      display.print(F("IP: "));
      display.print(WiFi.localIP().toString());
      display.display();
      delay(1000);
    }
    return true;
  } else {
    Serial.println();
    Serial.print("Failed: ");
    Serial.println(ssid);

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Failed to connect to:"));
      display.setCursor(0, 16);
      display.print(ssid);
      display.display();
      delay(1000);
    }
    return false;
  }
}

bool connectToWiFi() {
  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_STA);
  delay(500);

  if (apMode) {
    WiFi.softAPdisconnect(true);
    apMode = false;
  }

  Serial.println("Starting WiFi connection process...");

  // Try primary network
  if (connectToNetwork(Config::PRIMARY_SSID, Config::PRIMARY_PASS, 3)) {
    return true;
  }

  // Try secondary network
  Serial.println("Trying secondary network...");
  if (connectToNetwork(Config::SECONDARY_SSID, Config::SECONDARY_PASS, 3)) {
    return true;
  }

  // Setup AP mode
  Serial.println("Starting Access Point mode.");
  setupAccessPoint();
  return false;
}

void setupAccessPoint() {
  Serial.println("Setting up Access Point...");

  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(Config::AP_SSID, Config::AP_PASS)) {
    Serial.println("AP established successfully");
    Serial.print("AP SSID: ");
    Serial.println(Config::AP_SSID);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    apMode = true;
    activeNetwork = "AP Mode: " + String(Config::AP_SSID);

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("WiFi AP Mode Active"));
      display.setCursor(0, 16);
      display.print(F("SSID: "));
      display.print(Config::AP_SSID);
      display.setCursor(0, 32);
      display.print(F("IP: "));
      display.print(WiFi.softAPIP().toString());
      display.display();
      delay(1000);
    }
  } else {
    Serial.println("Failed to start AP");

    if (oledAvailable && !showSplash) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Failed to start AP"));
      display.display();
      delay(1000);
    }
  }
}

void checkWiFiConnection() {
  if (millis() - lastWifiCheck >= Config::WIFI_CHECK_INTERVAL) {
    lastWifiCheck = millis();

    if (!apMode && WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost. Reconnecting...");
      if (connectToWiFi()) {
        synchronizeTime();
        lastSyncTime = millis();
      }
    }
  }
}