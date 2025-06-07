# 🌟 Pulsar Diamond v1 - Quantum Chronos Matrix 
Handcrafted by - CubicCanvas & Claude Sonnet/Opus
> *An ESP32-powered smart alarm clock with mesmerizing web interface, OLED terminal, and customizable vibration patterns*

![Version](https://img.shields.io/badge/version-1.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)
![Status](https://img.shields.io/badge/status-operational-success.svg)

## ✨ Overview

Pulsar Diamond v1 is not just an alarm clock - it's a sophisticated wake-up experience system that combines precision timing, customizable vibration patterns, and a stunning web interface. Built on the ESP32 platform, it features redundant network connectivity, real-time NTP synchronization, and an intuitive OLED display that brings your morning routine into the future.

### 🎯 Key Features

- **Intelligent Alarm Logic**: Triggers at precise 4-minute intervals during your configured wake window
- **6 Vibration Patterns**: From gentle heartbeat to urgent SOS patterns
- **Dual Control Interface**: Physical buttons + stunning web dashboard
- **Network Resilience**: Auto-failover between WiFi networks with AP mode fallback
- **OLED Animations**: Real-time clock display with mesmerizing boot sequence
- **OTA Updates**: Wireless firmware updates without physical access
- **60-Second Snooze**: Because sometimes you need just one more minute

## 🛠️ Hardware Requirements

### Core Components
- **ESP32 Development Board** (tested on ESP32-WROOM-32)
- **SSD1306 OLED Display** (128x64, I2C interface)
- **5V Relay Module** (for motor/vibration control)
- **Vibration Motor** or buzzer (connected through relay)
- **4x Push Buttons** (normally open, with pull-up resistors)
- **Status LED** (with appropriate current-limiting resistor)

### Pin Connections

| Component | GPIO Pin | Description |
|-----------|----------|-------------|
| Relay Control | 25 | Controls vibration motor/buzzer |
| Snooze Button | 18 | 60-second snooze activation |
| Kill Switch | 19 | Emergency alarm disable |
| Test Button | 15 | Test alarm + pattern navigation |
| Set Alarm Button | 17 | Enable alarms + pattern navigation |
| Status LED | 23 | Alarm enabled indicator |
| OLED SDA | 21 | I2C Data (default ESP32) |
| OLED SCL | 22 | I2C Clock (default ESP32) |

### Wiring Diagram
```
ESP32          Component
-----          ---------
GPIO25 ------> Relay IN
GPIO18 ------> Snooze Button -> GND
GPIO19 ------> Kill Switch -> GND  
GPIO15 ------> Test Button -> GND
GPIO17 ------> Set Button -> GND
GPIO23 ------> LED -> Resistor -> GND
GPIO21 ------> OLED SDA
GPIO22 ------> OLED SCL
3.3V --------> OLED VCC
GND ---------> OLED GND
5V ----------> Relay VCC
GND ---------> Relay GND
```

## 📦 Software Dependencies

### Required Libraries
```cpp
#include <WiFi.h>              // ESP32 Core
#include <time.h>              // ESP32 Core
#include <ezButton.h>          // v1.0.4+
#include <ArduinoOTA.h>        // ESP32 Core
#include <WebServer.h>         // ESP32 Core
#include <Wire.h>              // ESP32 Core
#include <Adafruit_GFX.h>      // v1.11.5+
#include <Adafruit_SSD1306.h>  // v2.5.7+
```

### Installing Libraries
1. Open Arduino IDE
2. Go to **Tools > Manage Libraries**
3. Search and install:
   - `ezButton` by ArduinoGetStarted
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`

## 🚀 Installation & Setup

### 1. Configure WiFi Credentials
Edit `Config.h` and update your network settings:
```cpp
const char* PRIMARY_SSID = "YourMainWiFi";
const char* PRIMARY_PASS = "YourMainPassword";
const char* SECONDARY_SSID = "YourBackupWiFi";
const char* SECONDARY_PASS = "YourBackupPassword";
const char* AP_SSID = "PulsarDiamond";
const char* AP_PASS = "quantum123";
```

### 2. Set Your Timezone
Adjust the GMT offset in `Config.h`:
```cpp
const long GMT_OFFSET_SEC = -5 * 3600;  // EST (UTC-5)
const int DAYLIGHT_OFFSET_SEC = 3600;   // DST adjustment
```

### 3. Configure Alarm Window
Default window is 9:30 PM - 10:15 PM. Modify in `Config.h`:
```cpp
const int ALARM_START_HOUR = 21;    // 9 PM (24-hour format)
const int ALARM_START_MINUTE = 30;  // 30 minutes
const int ALARM_END_HOUR = 22;      // 10 PM
const int ALARM_END_MINUTE = 15;    // 15 minutes
```

### 4. Upload the Code
1. Connect ESP32 via USB
2. Select **Tools > Board > ESP32 Dev Module**
3. Select correct COM port
4. Click **Upload**

## 🎮 Usage Guide

### Physical Controls

| Button | Short Press | Long Press (2s) | Ultra Long (5s) |
|--------|------------|-----------------|-----------------|
| **Test** | Enable alarms + trigger immediately | Previous vibration pattern | - |
| **Set** | Enable alarms | Next vibration pattern | - |
| **Snooze** | Activate 60s snooze | - | Kill switch (disable all) |
| **Kill** | Disable all alarms | - | - |

### Web Interface Access

1. **Connected Mode**: Navigate to `http://[ESP32-IP-ADDRESS]`
2. **AP Mode**: Connect to "PulsarDiamond" WiFi, then visit `http://192.168.4.1`

The web interface provides:
- Real-time clock display
- Next alarm countdown
- Alarm enable/disable toggle
- Motor control toggle
- Pattern selection carousel
- Network status monitoring
- Test functions (quick test, single flash)
- WiFi reconnection utility

### Alarm Behavior

The alarm system follows a precise pattern:
1. **Trigger Condition**: When `currentMinute % 4 == 2` within alarm window
2. **Sequence**:
   - Initial flash (0.25s ON)
   - Pause (0.25s OFF)
   - Wait period (3s OFF)
   - Enter pulse mode with selected pattern

### Vibration Patterns

1. **Standard**: Complex 20-step sequence with varied timings
2. **Double Tap**: Quick double-pulse pattern
3. **SOS**: Morse code distress signal
4. **Heartbeat**: Mimics natural heart rhythm
5. **Escalating**: Progressively intense pulses
6. **Random**: Randomly selects from other patterns

## 🔧 Advanced Configuration

### Custom Vibration Patterns

Add new patterns in `Config.h`:
```cpp
const VibrationPattern PatternManager::PATTERNS[] = {
    {"MyPattern", {100, 200, 100, 500}, 4},  // ON, OFF, ON, OFF
    // ... existing patterns
};
```

### Network Behavior

The system implements intelligent network management:
1. Attempts primary network (3 retries)
2. Falls back to secondary network (3 retries)
3. Creates Access Point if both fail
4. Pre-connects at 9:29 PM for alarm reliability
5. Auto-reconnects every 5 minutes if disconnected

### OLED Customization

The display shows:
- Current time with AM/PM indicator
- Animated clock face
- Network status (WiFi/AP/Offline)
- Alarm status and countdown
- Active vibration alerts
- Boot animation sequence

## 🐛 Troubleshooting

### OLED Not Working
- Verify I2C address: Default is `0x3C`
- Check wiring: SDA to GPIO21, SCL to GPIO22
- Run I2C scanner sketch to detect address

### WiFi Connection Issues
- Ensure credentials are correct (case-sensitive)
- Check router's 2.4GHz band (ESP32 doesn't support 5GHz)
- Try AP mode: SSID "PulsarDiamond", Password "quantum123"

### Alarm Not Triggering
- Verify system time is correct (check serial monitor)
- Confirm alarm is enabled (LED should be ON)
- Check you're within alarm window (9:30 PM - 10:15 PM default)

### Motor Not Activating
- Test relay with multimeter
- Verify relay is receiving 5V power
- Check motor connections to relay NO/COM terminals

## 📊 System Architecture

```
┌─────────────────────────────────────────────────┐
│                 Main Loop                       │
├─────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐ │
│  │ Buttons  │  │   WiFi   │  │ Web Server   │ │
│  │ Handler  │  │ Manager  │  │   Handler    │ │
│  └──────────┘  └──────────┘  └──────────────┘ │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐ │
│  │  Alarm   │  │   Time   │  │    OLED      │ │
│  │  Logic   │  │   Sync   │  │   Display    │ │
│  └──────────┘  └──────────┘  └──────────────┘ │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐ │
│  │  Relay   │  │ Pattern  │  │     OTA      │ │
│  │ Control  │  │ Manager  │  │   Updates    │ │
│  └──────────┘  └──────────┘  └──────────────┘ │
└─────────────────────────────────────────────────┘
```

### File Structure
```
Pulsar_Diamond_v1/
├── Pulsar_Diamond_v1_GitCopy.ino  # Main sketch
├── Config.h                       # Configuration & globals
├── Alarm_Functions.ino           # Alarm logic
├── Button_Functions.ino          # Physical controls
├── OLED_Functions.ino           # Display management
├── Relay_Functions.ino          # Motor control
├── Time_Functions.ino           # NTP & time formatting
├── WebServer_Functions.ino      # Web interface
└── WiFi_Functions.ino           # Network management
```

## 🌐 Web API Endpoints

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/` | GET | - | Main web interface |
| `/toggleAlarm` | GET | state={on\|off} | Enable/disable alarms |
| `/toggleRelay` | GET | state={on\|off} | Control motor directly |
| `/testAlarm` | GET | - | Trigger test alarm |
| `/turnOffMotor` | GET | - | Stop motor immediately |
| `/quickTest` | GET | - | Run 5-cycle test |
| `/singleFlashTest` | GET | - | Single 500ms pulse |
| `/currentTime` | GET | - | Get formatted time |
| `/nextAlarm` | GET | - | JSON: next alarm info |
| `/alarmsStatus` | GET | - | JSON: system status |
| `/networkStatus` | GET | - | Network connection info |
| `/reconnectWiFi` | GET | - | Attempt reconnection |
| `/setPattern` | GET | index={0-5} | Change vibration pattern |

## 🔐 Security Considerations

- Change default AP password in production
- Use WPA2 encryption for all WiFi connections
- Consider implementing web authentication for remote access
- OTA updates use default Arduino security (consider custom implementation)

## 🚧 Future Enhancements

- [ ] Multiple alarm time windows
- [ ] Day-of-week scheduling
- [ ] Smartphone app integration
- [ ] Light sensor for adaptive display brightness
- [ ] Battery backup with power loss detection
- [ ] Temperature/humidity monitoring
- [ ] Custom alarm sounds via I2S
- [ ] MQTT integration for home automation

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- Adafruit for excellent display libraries
- ArduinoGetStarted for the ezButton library
- The ESP32 community for comprehensive documentation
- Font Awesome for web interface icons

---

**Built with ❤️ and ☕ for the quantum chronos matrix**

*Remember: Time is an illusion, but waking up on time doesn't have to be!*
