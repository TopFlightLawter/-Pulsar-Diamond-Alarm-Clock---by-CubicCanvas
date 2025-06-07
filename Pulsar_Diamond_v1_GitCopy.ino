// ======================================================================================
// PULSAR DIAMOND v1
// Mesmerizing Web Server | Working OLED Terminal | Alarm Pulse Mode
// Push Buttons Prioritized | Reconstructed Onto A New Board
// Only 3 Buttons; Set (17), Test (15), Snooze (18)
// Modularized Architecture
// ======================================================================================

#include "Config.h"

// ======================================================================================
// GLOBAL VARIABLE DEFINITIONS
// ======================================================================================
// OLED Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledAvailable = false;
unsigned long lastOledUpdate = 0;

// Buttons
ezButton snoozeButton(Config::SNOOZE_BUTTON_PIN, INPUT_PULLUP);
ezButton killSwitch(Config::KILL_SWITCH_PIN, INPUT_PULLUP);
ezButton testButton(Config::TEST_BUTTON_PIN, INPUT_PULLUP);
ezButton setAlarmButton(Config::SET_ALARM_BUTTON_PIN, INPUT_PULLUP);

// Network state
bool apMode = false;
String activeNetwork = "Not Connected";
unsigned long lastWifiCheck = 0;

// Time tracking
unsigned long rtcMillis = 0;
unsigned long lastSyncTime = 0;

// Alarm state
bool alarmsEnabled = true;
bool alarmTriggered = false;
bool snoozeActivated = false;
unsigned long snoozeStartTime = 0;
int alarmStage = 0;
unsigned long alarmStageTime = 0;

// Relay state
bool relayState = false;
bool pulseMode = false;
unsigned long lastPulseTime = 0;
bool pulseState = false;

// Test mode tracking
bool quickTestActive = false;
int quickTestPhase = 0;
int quickTestToggleCount = 0;
unsigned long quickTestStartTime = 0;
unsigned long quickTestInterval = 0;
bool singleFlashActive = false;
unsigned long singleFlashTime = 0;

// OLED animation state
int animationFrame = 0;
unsigned long animationTime = 0;
bool showSplash = true;
unsigned long splashStartTime = 0;
bool splashDebug = true;

// Long press detection
unsigned long buttonPressStart[3] = { 0, 0, 0 };
bool buttonLongPressed[3] = { false, false, false };
bool buttonWasPressed[3] = { false, false, false };

// Web server
WebServer server(80);

// ======================================================================================
// PATTERN MANAGER IMPLEMENTATION
// ======================================================================================
const VibrationPattern PatternManager::PATTERNS[PatternManager::PATTERN_COUNT] = {
  { "Standard", { 75, 75, 75, 75, 75, 900, 250, 250, 1000, 900, 50, 50, 50, 100, 50, 50, 50, 400, 50, 500 }, 20 },
  { "Double Tap", { 250, 75, 250, 75 }, 4 },
  { "SOS", { 50, 70 }, 2 },
  { "Heartbeat", { 500, 150, 500, 1000 }, 4 },
  { "Escalating", { 100, 300, 200, 300, 300, 300, 400, 300 }, 8 },
  { "Random", { 0 }, 0 }
};

int PatternManager::currentPattern = 0;

void PatternManager::nextPattern() {
  currentPattern = (currentPattern + 1) % PATTERN_COUNT;
}

void PatternManager::prevPattern() {
  currentPattern = (currentPattern + PATTERN_COUNT - 1) % PATTERN_COUNT;
}

void PatternManager::setPattern(int index) {
  if (index >= 0 && index < PATTERN_COUNT) {
    currentPattern = index;
  }
}

const VibrationPattern* PatternManager::getCurrentPattern() {
  return &PATTERNS[currentPattern];
}

const char* PatternManager::getCurrentPatternName() {
  return PATTERNS[currentPattern].name;
}

// ======================================================================================
// SETUP FUNCTION
// ======================================================================================
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("\n\n=== Pulsar Diamond Alarm Clock v1  ===");
  Serial.println("Initializing modular system...");

  // Initialize Wire for I2C
  Wire.begin();

  // Initialize OLED
  initOLED();

  // Initialize GPIO
  pinMode(Config::RELAY_PIN, OUTPUT);
  pinMode(Config::LED_PIN, OUTPUT);
  digitalWrite(Config::RELAY_PIN, LOW);
  digitalWrite(Config::LED_PIN, alarmsEnabled ? HIGH : LOW);

  // Setup buttons
  snoozeButton.setDebounceTime(50);
  killSwitch.setDebounceTime(50);
  testButton.setDebounceTime(50);
  setAlarmButton.setDebounceTime(50);

  // Initialize button states
  snoozeButton.loop();
  killSwitch.loop();
  testButton.loop();
  setAlarmButton.loop();

  Serial.println("Hardware initialized.");

  // WiFi connection
  Serial.println("Starting WiFi connection...");
  if (connectToWiFi()) {
    Serial.println("WiFi connected successfully.");
    synchronizeTime();
    lastSyncTime = millis();
  } else {
    Serial.println("WiFi connection failed - running in AP mode.");
  }

  // Setup web server
  setupWebServer();

  // Initialize OTA
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
  });
  ArduinoOTA.begin();

  Serial.println("System initialization complete.");
  Serial.println("============================");
}

// ======================================================================================
// MAIN LOOP
// ======================================================================================
void loop() {
  // Handle physical controls first
  handleButtons();
  handleLongPress();

  // Handle OTA and web server
  ArduinoOTA.handle();
  server.handleClient();

  // Handle system functions
  handlePulseMode();
  handleSnooze();

  if (quickTestActive) handleQuickTest();
  if (singleFlashActive) handleSingleFlash();

  // Update display
  updateOLEDDisplay();

  // Periodic maintenance
  checkWiFiConnection();

  if (WiFi.status() == WL_CONNECTED && millis() - lastSyncTime > Config::SYNC_INTERVAL) {
    synchronizeTime();
    lastSyncTime = millis();
  }

  // Handle alarm logic
  handleAlarmLogic();

  delay(10);
}