#ifndef CONFIG_H
#define CONFIG_H

// ======================================================================================
// INCLUDES
// ======================================================================================
#include <WiFi.h>
#include <time.h>
#include <ezButton.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ======================================================================================
// OLED CONFIGURATION
// ======================================================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// ======================================================================================
// SYSTEM CONFIGURATION
// ======================================================================================
namespace Config {
// WiFi credentials
const char* PRIMARY_SSID = "MainSSID";
const char* PRIMARY_PASS = "MainPASS";
const char* SECONDARY_SSID = "BackUpSSID";
const char* SECONDARY_PASS = "BackUpPASS";
const char* AP_SSID = "ApSSID";
const char* AP_PASS = "ApPASS";

// GPIO pin assignments
const int RELAY_PIN = 25;
const int SNOOZE_BUTTON_PIN = 18;
const int KILL_SWITCH_PIN = 19;
const int TEST_BUTTON_PIN = 15;
const int SET_ALARM_BUTTON_PIN = 17;
const int LED_PIN = 23;

// Time configuration
const long GMT_OFFSET_SEC = -5 * 3600;
const int DAYLIGHT_OFFSET_SEC = 3600;
const unsigned long SYNC_INTERVAL = 6UL * 3600UL * 1000UL;
const unsigned long WIFI_CHECK_INTERVAL = 300000;

// Alarm time windows
// Default 9:30 pm -> 10:15 pm
const int ALARM_START_HOUR = 21;
const int ALARM_START_MINUTE = 30;
const int ALARM_END_HOUR = 22;
const int ALARM_END_MINUTE = 15;

// Timing constants
const unsigned long SNOOZE_DURATION = 60000;  // 1 Minute Snooze
const int LOADING_FRAMES = 5;
const int PULSE_FRAMES = 3;
const unsigned long SPLASH_DURATION = 3000;
const unsigned long PULSE_ON_DURATION = 300;
const unsigned long PULSE_OFF_DURATION = 300;
const unsigned long OLED_UPDATE_INTERVAL = 1000;
}

// ======================================================================================
// PATTERN MANAGER
// ======================================================================================
struct VibrationPattern {
  const char* name;
  const uint16_t sequence[20];
  uint8_t steps;
};

class PatternManager {
public:
  static const int PATTERN_COUNT = 6;

  static const VibrationPattern PATTERNS[PATTERN_COUNT];
  static int currentPattern;

  static void nextPattern();
  static void prevPattern();
  static void setPattern(int index);
  static const VibrationPattern* getCurrentPattern();
  static const char* getCurrentPatternName();
};

// ======================================================================================
// GLOBAL VARIABLES
// ======================================================================================
// OLED Display
extern Adafruit_SSD1306 display;
extern bool oledAvailable;
extern unsigned long lastOledUpdate;

// Buttons
extern ezButton snoozeButton;
extern ezButton killSwitch;
extern ezButton testButton;
extern ezButton setAlarmButton;

// Network state
extern bool apMode;
extern String activeNetwork;
extern unsigned long lastWifiCheck;

// Time tracking
extern unsigned long rtcMillis;
extern unsigned long lastSyncTime;

// Alarm state
extern bool alarmsEnabled;
extern bool alarmTriggered;
extern bool snoozeActivated;
extern unsigned long snoozeStartTime;
extern int alarmStage;
extern unsigned long alarmStageTime;

// Relay state
extern bool relayState;
extern bool pulseMode;
extern unsigned long lastPulseTime;
extern bool pulseState;

// Test mode tracking
extern bool quickTestActive;
extern int quickTestPhase;
extern int quickTestToggleCount;
extern unsigned long quickTestStartTime;
extern unsigned long quickTestInterval;
extern bool singleFlashActive;
extern unsigned long singleFlashTime;

// OLED animation state
extern int animationFrame;
extern unsigned long animationTime;
extern bool showSplash;
extern unsigned long splashStartTime;
extern bool splashDebug;

// Long press detection
extern unsigned long buttonPressStart[3];
extern bool buttonLongPressed[3];
extern bool buttonWasPressed[3];

// Web server
extern WebServer server;

// ======================================================================================
// FUNCTION PROTOTYPES
// ======================================================================================
// WiFi Functions
bool connectToWiFi();
bool connectToNetwork(const char* ssid, const char* password, int maxAttempts = 5);
void setupAccessPoint();
void checkWiFiConnection();

// Time Functions
void synchronizeTime();
String getFormattedTime();
String getNextAlarmTime();
String getTimeRemaining();

// OLED Functions
void initOLED();
void updateOLEDDisplay();
void showSplashScreen();
void drawProgressBar(int x, int y, int width, int progress);
void drawBatteryIcon(int x, int y, int level);
void drawWifiIcon(int x, int y, int strength);
void drawAlarmIcon(int x, int y, bool enabled);
void drawClockFace(int x, int y, int radius);
void drawRelayStatus(bool active);
void showPatternOnOLED(int patternIndex);
void killSwitchAnimation();

// Relay Functions
void activateRelay();
void deactivateRelay();
void startPulseMode();
void stopPulseMode();
void handlePulseMode();

// Button Functions
void handleButtons();
void handleLongPress();

// WebServer Functions
void setupWebServer();
String generateHtmlPage();

// Alarm Functions
void setAlarmsEnabled(bool state);
void handleAlarmLogic();
void handleQuickTest();
void handleSingleFlash();
void handleSnooze();

#endif  // CONFIG_H