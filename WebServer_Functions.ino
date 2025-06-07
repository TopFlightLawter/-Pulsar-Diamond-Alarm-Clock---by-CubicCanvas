// ======================================================================================
// WEBSERVER FUNCTIONS
// Web interface and API endpoints
// ======================================================================================

void setupWebServer() {
  // Main page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", generateHtmlPage());
  });

  // Toggle alarms
  server.on("/toggleAlarm", HTTP_GET, []() {
    if (server.hasArg("state")) {
      String state = server.arg("state");
      if (state == "on") {
        setAlarmsEnabled(true);
        server.send(200, "text/plain", "Alarms Enabled");
      } else if (state == "off") {
        setAlarmsEnabled(false);
        server.send(200, "text/plain", "Alarms Disabled");
      } else {
        server.send(400, "text/plain", "Invalid State");
      }
    } else {
      server.send(400, "text/plain", "No State Provided");
    }
  });

  // Toggle relay
  server.on("/toggleRelay", HTTP_GET, []() {
    if (server.hasArg("state")) {
      String state = server.arg("state");
      if (state == "on") {
        stopPulseMode();
        startPulseMode();
        server.send(200, "text/plain", "Relay Pulse Mode Started");
      } else if (state == "off") {
        stopPulseMode();
        deactivateRelay();
        server.send(200, "text/plain", "Relay Turned Off");
      } else {
        server.send(400, "text/plain", "Invalid State");
      }
    } else {
      server.send(400, "text/plain", "No State Provided");
    }
  });

  // Test alarm
  server.on("/testAlarm", HTTP_GET, []() {
    stopPulseMode();
    alarmTriggered = true;
    alarmStage = 1;
    alarmStageTime = millis();
    server.send(200, "text/plain", "Test Alarm Activated");
  });

  // Turn off motor
  server.on("/turnOffMotor", HTTP_GET, []() {
    stopPulseMode();
    deactivateRelay();
    alarmTriggered = false;
    alarmStage = 0;
    server.send(200, "text/plain", "Motor Turned Off");
  });

  // Quick test
  server.on("/quickTest", HTTP_GET, []() {
    if (!quickTestActive) {
      quickTestActive = true;
      quickTestPhase = 0;
      quickTestToggleCount = 0;
      quickTestInterval = 100;
      quickTestStartTime = millis();
      server.send(200, "text/plain", "Quick Test Started");
    } else {
      server.send(200, "text/plain", "Quick Test Already Active");
    }
  });

  // Single flash test
  server.on("/singleFlashTest", HTTP_GET, []() {
    if (!singleFlashActive) {
      singleFlashActive = true;
      singleFlashTime = millis();
      server.send(200, "text/plain", "Single Flash Started");
    } else {
      server.send(200, "text/plain", "Single Flash Already Active");
    }
  });

  // Current time
  server.on("/currentTime", HTTP_GET, []() {
    server.send(200, "text/plain", getFormattedTime());
  });

  // Next alarm info
  server.on("/nextAlarm", HTTP_GET, []() {
    String json = "{ \"time\": \"" + getNextAlarmTime() + "\", \"remaining\": \"" + getTimeRemaining() + "\" }";
    server.send(200, "application/json", json);
  });

  // Network status
  server.on("/networkStatus", HTTP_GET, []() {
    String status;
    if (apMode) {
      status = "Access Point Mode: " + String(Config::AP_SSID);
    } else if (WiFi.status() == WL_CONNECTED) {
      status = "Connected to: " + activeNetwork;
    } else {
      status = "Disconnected from network";
    }
    server.send(200, "text/plain", status);
  });

  // WiFi reconnection
  server.on("/reconnectWiFi", HTTP_GET, []() {
    bool success = connectToWiFi();
    String responseMessage;

    if (success) {
      synchronizeTime();
      responseMessage = "Successfully connected to WiFi";
    } else {
      responseMessage = apMode ? "Failed to connect, running in AP mode" : "Failed to connect, offline mode";
    }

    server.send(200, "text/plain", responseMessage);
  });

  // Alarm status
  server.on("/alarmsStatus", HTTP_GET, []() {
    String json = "{ \"alarmsEnabled\": " + String(alarmsEnabled ? "true" : "false") + ", \"relayState\": " + String(relayState || pulseMode ? "true" : "false") + " }";
    server.send(200, "application/json", json);
  });

  // Set vibration pattern
  server.on("/setPattern", HTTP_GET, []() {
    if (server.hasArg("index")) {
      int index = server.arg("index").toInt();
      PatternManager::setPattern(index);
      Serial.print("Web pattern set: ");
      Serial.println(PatternManager::getCurrentPatternName());
      server.send(200, "text/plain", "Pattern set to " + String(PatternManager::getCurrentPatternName()));
    } else {
      server.send(400, "text/plain", "No pattern index provided");
    }
  });

  server.begin();
  Serial.println("Web server started.");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Interface: http://");
    Serial.println(WiFi.localIP());
  } else if (apMode) {
    Serial.print("Interface: http://");
    Serial.println(WiFi.softAPIP());
  }
}

String generateHtmlPage() {
  String currentTime = getFormattedTime();
  String nextAlarm = getNextAlarmTime();
  String timeRemaining = getTimeRemaining();

  String networkStatus;
  if (apMode) {
    networkStatus = "Access Point Mode: " + String(Config::AP_SSID);
  } else if (WiFi.status() == WL_CONNECTED) {
    networkStatus = "Connected to: " + activeNetwork;
  } else {
    networkStatus = "Disconnected";
  }

  return String(R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pulsar Diamond | Alarm System</title>
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css" rel="stylesheet">
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;500;600;700;800;900&family=Rajdhani:wght@300;400;500;600;700&display=swap');
      
        :root {
            --primary: #00e5ff;
            --primary-glow: rgba(0, 229, 255, 0.5);
            --secondary: #ff00e5;
            --secondary-glow: rgba(255, 0, 229, 0.5);
            --bg-dark: #0a0e17;
            --panel-bg: rgba(16, 23, 34, 0.8);
            --success: #00ff9d;
            --warning: #ffbb00;
            --danger: #ff2e63;
            --text: #e0e6ff;
            --disabled: #3a4052;
        }
      
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
      
        body {
            font-family: 'Rajdhani', sans-serif;
            background-color: var(--bg-dark);
            color: var(--text);
            background-image:
                radial-gradient(circle at 10% 20%, rgba(0, 229, 255, 0.05) 0%, transparent 20%),
                radial-gradient(circle at 90% 80%, rgba(255, 0, 229, 0.05) 0%, transparent 20%),
                linear-gradient(to bottom, rgba(10, 14, 23, 0.99), rgba(10, 14, 23, 0.97));
            background-attachment: fixed;
            min-height: 100vh;
        }
      
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
      
        .header {
            text-align: center;
            margin-bottom: 30px;
            position: relative;
        }
      
        h1, h2, h3, h4 {
            font-family: 'Orbitron', sans-serif;
            letter-spacing: 2px;
            text-transform: uppercase;
        }
      
        .header h1 {
            font-size: 2.5rem;
            color: var(--primary);
            text-shadow: 0 0 10px var(--primary-glow);
            margin: 0;
            position: relative;
            display: inline-block;
        }
      
        .header h1::before,
        .header h1::after {
            content: '';
            position: absolute;
            height: 2px;
            top: 50%;
            width: 50px;
            background: linear-gradient(90deg, transparent, var(--primary), transparent);
        }
      
        .header h1::before {
            right: 105%;
        }
      
        .header h1::after {
            left: 105%;
        }
      
        .nexus-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 25px;
            margin-bottom: 30px;
        }
      
        .nexus-panel {
            background: var(--panel-bg);
            border-radius: 15px;
            padding: 25px;
            position: relative;
            overflow: hidden;
            backdrop-filter: blur(10px);
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.2),
                        inset 0 0 0 1px rgba(255, 255, 255, 0.1);
            transition: all 0.3s ease;
        }
      
        .nexus-panel::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 2px;
            background: linear-gradient(90deg, transparent, var(--primary), var(--secondary), transparent);
            animation: border-flow 4s linear infinite;
        }
      
        @keyframes border-flow {
            0% { background-position: 0% 0%; }
            100% { background-position: 200% 0%; }
        }
      
        .nexus-panel:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 30px rgba(0, 0, 0, 0.3),
                        inset 0 0 0 1px rgba(255, 255, 255, 0.2);
        }
 
        .pattern-selector {
            position: relative;
            padding: 15px;
            margin-top: 15px;
            background: rgba(0, 20, 40, 0.5);
            border-radius: 10px;
            overflow: hidden;
        }
       
        .pattern-title {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
            font-size: 1.1rem;
            font-weight: 500;
        }
       
        .pattern-title i {
            margin-right: 8px;
            font-size: 1.2rem;
            color: var(--primary);
        }
       
        .pattern-carousel {
            position: relative;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 80px;
        }
       
        .pattern-item {
            background: rgba(0, 10, 20, 0.6);
            border-radius: 8px;
            padding: 15px;
            width: 100%;
            text-align: center;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
            transition: all 0.3s ease;
        }
       
        .pattern-name {
            font-family: 'Orbitron', sans-serif;
            font-size: 1.2rem;
            margin-bottom: 10px;
            color: var(--primary);
        }
       
        .pattern-visual {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 30px;
            margin-bottom: 10px;
        }
       
        .pattern-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background-color: var(--primary);
            margin: 0 3px;
            opacity: 0.6;
        }
       
        .pattern-dot.active {
            opacity: 1;
            box-shadow: 0 0 10px var(--primary-glow);
        }
       
        .pattern-nav {
            display: flex;
            justify-content: space-between;
            margin-top: 10px;
        }
       
        .pattern-nav-btn {
            background: rgba(0, 229, 255, 0.1);
            color: var(--primary);
            border: none;
            border-radius: 50%;
            width: 36px;
            height: 36px;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: all 0.3s ease;
        }
       
        .pattern-nav-btn:hover {
            background: rgba(0, 229, 255, 0.2);
            box-shadow: 0 0 15px rgba(0, 229, 255, 0.3);
        }
      
        .panel-title {
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            display: flex;
            align-items: center;
        }
      
        .panel-title h2 {
            font-size: 1.5rem;
            color: var(--primary);
            margin: 0;
        }
      
        .panel-title i {
            margin-right: 10px;
            font-size: 1.2rem;
            color: var(--primary);
        }
      
        .time-display {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 250px;
            position: relative;
        }
      
        .time-circle {
            position: relative;
            width: 180px;
            height: 180px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            background: rgba(0, 10, 20, 0.5);
            box-shadow: 0 0 30px rgba(0, 229, 255, 0.2);
            margin-bottom: 20px;
        }
      
        .time-circle::before {
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            border-radius: 50%;
            background: linear-gradient(45deg, var(--primary), var(--secondary));
            z-index: -1;
            opacity: 0.7;
            animation: rotate 10s linear infinite;
        }
      
        @keyframes rotate {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
      
        .current-time {
            font-family: 'Orbitron', sans-serif;
            font-size: 2.5rem;
            text-align: center;
            color: white;
            line-height: 1;
            position: relative;
        }
      
        .ampm {
            position: absolute;
            top: -10px;
            right: -25px;
            font-size: 0.8rem;
            color: var(--primary);
        }
      
        .date-info {
            font-size: 1rem;
            opacity: 0.7;
            letter-spacing: 1px;
            text-align: center;
        }
      
        .alarm-info {
            margin-top: auto;
            width: 100%;
            padding: 15px;
            background: rgba(0, 20, 40, 0.5);
            border-radius: 10px;
            text-align: center;
        }
      
        .alarm-status {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
        }
      
        .next-alarm-time {
            font-size: 1.2rem;
            font-weight: 600;
            color: var(--warning);
            margin-bottom: 5px;
        }
      
        .time-remaining {
            font-size: 1rem;
            color: var(--text);
            display: flex;
            align-items: center;
            justify-content: center;
        }
      
        .time-remaining i {
            margin-right: 5px;
            color: var(--warning);
        }
      
        .controls-container {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
      
        .control-group {
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
      
        .toggle-control {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 15px;
            background: rgba(0, 20, 40, 0.5);
            border-radius: 10px;
            margin-bottom: 10px;
            transition: all 0.3s ease;
        }
      
        .toggle-control:hover {
            background: rgba(0, 30, 60, 0.5);
        }
      
        .toggle-label {
            font-size: 1.1rem;
            font-weight: 500;
            display: flex;
            align-items: center;
        }
      
        .toggle-label i {
            margin-right: 8px;
            font-size: 1rem;
        }
      
        .toggle-switch {
            position: relative;
            width: 60px;
            height: 30px;
        }
      
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
      
        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: var(--disabled);
            transition: .4s;
            border-radius: 30px;
            box-shadow: inset 0 0 5px rgba(0, 0, 0, 0.5);
        }
      
        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 22px;
            width: 22px;
            left: 4px;
            bottom: 4px;
            background-color: var(--text);
            transition: .4s;
            border-radius: 50%;
        }
      
        input:checked + .toggle-slider {
            background-color: var(--primary);
            box-shadow: 0 0 10px var(--primary-glow);
        }
      
        input:checked + .toggle-slider:before {
            transform: translateX(30px);
        }
      
        .button-group {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
        }
      
        .nexus-button {
            position: relative;
            padding: 12px 15px;
            background: transparent;
            color: var(--text);
            border: none;
            border-radius: 8px;
            font-family: 'Rajdhani', sans-serif;
            font-size: 1rem;
            font-weight: 600;
            letter-spacing: 1px;
            text-transform: uppercase;
            cursor: pointer;
            transition: all 0.3s ease;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: inset 0 0 0 1px rgba(255, 255, 255, 0.2);
        }
      
        .nexus-button::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
            transition: all 0.6s ease;
        }
      
        .nexus-button:hover::before {
            left: 100%;
        }
      
        .nexus-button:hover {
            box-shadow: 0 0 15px var(--primary-glow),
                        inset 0 0 0 1px var(--primary);
        }
      
        .nexus-button i {
            margin-right: 8px;
        }
      
        .nexus-button.test {
            background: linear-gradient(45deg, rgba(255, 187, 0, 0.1), rgba(255, 187, 0, 0.2));
            color: var(--warning);
        }
      
        .nexus-button.test:hover {
            box-shadow: 0 0 15px rgba(255, 187, 0, 0.5),
                        inset 0 0 0 1px var(--warning);
        }
      
        .nexus-button.off {
            background: linear-gradient(45deg, rgba(255, 46, 99, 0.1), rgba(255, 46, 99, 0.2));
            color: var(--danger);
        }
      
        .nexus-button.off:hover {
            box-shadow: 0 0 15px rgba(255, 46, 99, 0.5),
                        inset 0 0 0 1px var(--danger);
        }
      
        .nexus-button.flash {
            background: linear-gradient(45deg, rgba(0, 229, 255, 0.1), rgba(0, 229, 255, 0.2));
            color: var(--primary);
        }
      
        .nexus-button.flash:hover {
            box-shadow: 0 0 15px rgba(0, 229, 255, 0.5),
                        inset 0 0 0 1px var(--primary);
        }
      
        .nexus-button.quick {
            background: linear-gradient(45deg, rgba(0, 255, 157, 0.1), rgba(0, 255, 157, 0.2));
            color: var(--success);
        }
      
        .nexus-button.quick:hover {
            box-shadow: 0 0 15px rgba(0, 255, 157, 0.5),
                        inset 0 0 0 1px var(--success);
        }
      
        .nexus-button.network {
            background: linear-gradient(45deg, rgba(255, 153, 0, 0.1), rgba(255, 153, 0, 0.2));
            color: #FF9900;
        }
      
        .nexus-button.network:hover {
            box-shadow: 0 0 15px rgba(255, 153, 0, 0.5),
                        inset 0 0 0 1px #FF9900;
        }
      
        .status-panel {
            padding: 15px;
        }
      
        .status-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
        }
      
        .status-item {
            background: rgba(0, 20, 40, 0.5);
            border-radius: 8px;
            padding: 15px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            text-align: center;
        }
      
        .status-icon {
            font-size: 1.8rem;
            margin-bottom: 10px;
        }
      
        .status-value {
            font-size: 1.2rem;
            font-weight: 600;
        }
      
        .status-label {
            font-size: 0.9rem;
            opacity: 0.7;
        }
      
        .status-item.wifi .status-icon {
            color: var(--primary);
        }
      
        .status-item.relay .status-icon {
            color: var(--success);
        }
      
        .status-item.alarm .status-icon {
            color: var(--warning);
        }
      
        .status-item.system .status-icon {
            color: var(--secondary);
        }
      
        @keyframes pulse {
            0% {
                transform: scale(1);
                opacity: 1;
            }
            50% {
                transform: scale(1.1);
                opacity: 0.7;
            }
            100% {
                transform: scale(1);
                opacity: 1;
            }
        }
      
        .status-icon.active {
            animation: pulse 2s infinite;
        }
      
        .animated-bg {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: -1;
            overflow: hidden;
        }
      
        .particle {
            position: absolute;
            width: 2px;
            height: 2px;
            background-color: var(--primary);
            border-radius: 50%;
            opacity: 0;
            animation: float 15s infinite ease-in-out;
        }
      
        @keyframes float {
            0% {
                transform: translateY(100vh) translateX(0);
                opacity: 0;
            }
            10% {
                opacity: 0.5;
            }
            90% {
                opacity: 0.5;
            }
            100% {
                transform: translateY(0) translateX(20px);
                opacity: 0;
            }
        }
      
        .footer {
            margin-top: 40px;
            text-align: center;
            font-size: 0.9rem;
            opacity: 0.5;
            padding: 20px;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
        }
      
        .footer p {
            margin: 5px 0;
        }
      
        .alarm-pulse {
            display: none;
            position: absolute;
            top: 10px;
            right: 10px;
            width: 15px;
            height: 15px;
            border-radius: 50%;
            background-color: var(--danger);
            box-shadow: 0 0 10px var(--danger);
        }
      
        .alarm-pulse.active {
            display: block;
            animation: alarm-pulse 1s infinite;
        }
      
        @keyframes alarm-pulse {
            0% {
                transform: scale(1);
                opacity: 1;
            }
            50% {
                transform: scale(1.5);
                opacity: 0.5;
            }
            100% {
                transform: scale(1);
                opacity: 1;
            }
        }
      
        .network-status {
            position: relative;
            padding: 8px 15px;
            background: rgba(0, 20, 40, 0.6);
            border-radius: 20px;
            font-size: 0.9rem;
            display: inline-flex;
            align-items: center;
            margin-top: 10px;
            border: 1px solid rgba(0, 229, 255, 0.2);
        }
      
        .network-status i {
            margin-right: 8px;
            font-size: 1rem;
        }
      
        .network-status.connected i {
            color: var(--success);
        }
      
        .network-status.ap-mode i {
            color: var(--warning);
        }
      
        .network-status.disconnected i {
            color: var(--danger);
        }
      
        @media (max-width: 768px) {
            .nexus-grid {
                grid-template-columns: 1fr;
            }
          
            .header h1 {
                font-size: 2rem;
            }
          
            .header h1::before,
            .header h1::after {
                width: 30px;
            }
          
            .button-group {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="animated-bg" id="animatedBg"></div>
  
    <div class="container">
        <div class="header">
            <h1>Pulsar Diamond v1</h1>
        </div>
      
        <div class="nexus-grid">
            <div class="nexus-panel">
                <div class="alarm-pulse" id="alarmPulse"></div>
                <div class="panel-title">
                    <i class="fas fa-clock"></i>
                    <h2>Chronos</h2>
                </div>
                <div class="time-display">
                    <div class="time-circle">
                        <div class="current-time">
                            <span id="current-time-value">--:--</span>
                            <span id="current-time-ampm" class="ampm">--</span>
                        </div>
                    </div>
                    <div class="date-info" id="date-info">
                        Synchronizing...
                    </div>
                    <div class="alarm-info">
                        <div class="alarm-status">
                            <span>Next Wake Protocol:</span>
                            <span id="next-alarm-time">--:--</span>
                        </div>
                        <div class="time-remaining">
                            <i class="fas fa-hourglass-half"></i>
                            <span id="time-remaining">--h --m</span>
                        </div>
                    </div>
                    <div id="network-status-container" class="network-status connected">
                        <i id="network-icon" class="fas fa-wifi"></i>
                        <span id="network-status-text">)rawliteral"
                + networkStatus + R"rawliteral(</span>
                    </div>
                </div>
            </div>
          
            <div class="nexus-panel">
                <div class="panel-title">
                    <i class="fas fa-sliders-h"></i>
                    <h2>Command Center</h2>
                </div>
                <div class="controls-container">
                    <div class="control-group">
                        <div class="toggle-control">
                            <div class="toggle-label">
                                <i class="fas fa-bell"></i>
                                Protocol Status
                            </div>
                            <label class="toggle-switch">
                                <input type="checkbox" id="alarmToggle" onchange="toggleAlarmSet(this.checked ? 'on' : 'off')">
                                <span class="toggle-slider"></span>
                            </label>
                        </div>
                        <div class="toggle-control">
                            <div class="toggle-label">
                                <i class="fas fa-bolt"></i>
                                Power Matrix
                            </div>
                            <label class="toggle-switch">
                                <input type="checkbox" id="relayToggle" onchange="toggleRelay(this.checked ? 'on' : 'off')">
                                <span class="toggle-slider"></span>
                            </label>
                        </div>
                    </div>
                  
                    <div class="button-group">
                        <button class="nexus-button test" id="testAlarmBtn" onclick="testAlarm()">
                            <i class="fas fa-play-circle"></i>
                            Test Protocol
                        </button>
                        <button class="nexus-button off" id="turnOffMotorBtn" onclick="turnOffMotor()">
                            <i class="fas fa-power-off"></i>
                            Deactivate
                        </button>
                        <button class="nexus-button quick" id="quickTestBtn" onclick="quickTest()">
                            <i class="fas fa-bolt"></i>
                            Diagnostics
                        </button>
                        <button class="nexus-button flash" id="singleFlashBtn" onclick="singleFlashTest()">
                            <i class="fas fa-lightbulb"></i>
                            Pulse Signal
                        </button>
                        <button class="nexus-button network" id="reconnectBtn" onclick="attemptWiFiReconnect()">
                            <i class="fas fa-network-wired"></i>
                            Reconnect Matrix
                        </button>
                    </div>

                    <div class="pattern-selector">
                        <div class="pattern-title">
                            <i class="fas fa-sliders-h"></i>
                            Vibration Pattern
                        </div>
                        <div class="pattern-carousel">
                            <div class="pattern-item">
                                <div class="pattern-name" id="pattern-name">Standard</div>
                                <div class="pattern-visual" id="pattern-visual">
                                </div>
                            </div>
                        </div>
                        <div class="pattern-nav">
                            <button class="pattern-nav-btn" onclick="changePattern('prev')">
                                <i class="fas fa-chevron-left"></i>
                            </button>
                            <button class="pattern-nav-btn" onclick="changePattern('next')">
                                <i class="fas fa-chevron-right"></i>
                            </button>
                        </div>
                    </div>
                </div>
            </div>
           
            <div class="nexus-panel">
                <div class="panel-title">
                    <i class="fas fa-chart-line"></i>
                    <h2>System Status</h2>
                </div>
                <div class="status-grid">
                    <div class="status-item wifi">
                        <i class="fas fa-wifi status-icon" id="wifi-icon"></i>
                        <div class="status-value" id="wifi-status">Checking...</div>
                        <div class="status-label">Network</div>
                    </div>
                    <div class="status-item relay">
                        <i class="fas fa-bolt status-icon" id="relay-icon"></i>
                        <div class="status-value" id="relay-status">Inactive</div>
                        <div class="status-label">Power Matrix</div>
                    </div>
                    <div class="status-item alarm">
                        <i class="fas fa-bell status-icon" id="alarm-icon"></i>
                        <div class="status-value" id="alarm-status">Standby</div>
                        <div class="status-label">Protocol</div>
                    </div>
                    <div class="status-item system">
                        <i class="fas fa-microchip status-icon active"></i>
                        <div class="status-value">Operational</div>
                        <div class="status-label">System Core</div>
                    </div>
                </div>
            </div>
        </div>
      
        <div class="footer">
            <p>Pulsar Diamond Interface v1 | ESP32 Core</p>
            <p>Quantum Chronos Matrix</p>
        </div>
    </div>
  
    <script>
        function createParticles() {
            const bg = document.getElementById('animatedBg');
            const particleCount = 20;
          
            for (let i = 0; i < particleCount; i++) {
                const particle = document.createElement('div');
                particle.classList.add('particle');
              
                const startPosX = Math.random() * 100;
                const size = Math.random() * 3 + 1;
                const delay = Math.random() * 15;
                const duration = Math.random() * 10 + 10;
              
                particle.style.left = `${startPosX}%`;
                particle.style.width = `${size}px`;
                particle.style.height = `${size}px`;
                particle.style.animationDelay = `${delay}s`;
                particle.style.animationDuration = `${duration}s`;
              
                if (Math.random() > 0.5) {
                    particle.style.backgroundColor = 'var(--primary)';
                } else {
                    particle.style.backgroundColor = 'var(--secondary)';
                }
              
                bg.appendChild(particle);
            }
        }
      
        function updateDateInfo() {
            const now = new Date();
            const days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
            const months = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];
          
            const day = days[now.getDay()];
            const date = now.getDate();
            const month = months[now.getMonth()];
            const year = now.getFullYear();
          
            document.getElementById('date-info').innerText = `${day}, ${month} ${date}, ${year}`;
        }
      
        function animateButton(buttonId) {
            const button = document.getElementById(buttonId);
            button.classList.add('active');
            setTimeout(() => button.classList.remove('active'), 500);
        }
      
        function updateTime() {
            fetch('/currentTime')
            .then(response => response.text())
            .then(data => {
                const timeParts = data.split(' ');
                const timeValue = timeParts[0];
                const ampm = timeParts[1];
              
                document.getElementById('current-time-value').innerText = timeValue;
                document.getElementById('current-time-ampm').innerText = ampm;
            });
        }
      
        function updateAlarmInfo() {
            fetch('/nextAlarm')
            .then(response => response.json())
            .then(data => {
                document.getElementById('next-alarm-time').innerText = data.time;
                document.getElementById('time-remaining').innerText = data.remaining;
            });
        }
      
        function updateNetworkStatus() {
            fetch('/networkStatus')
            .then(response => response.text())
            .then(data => {
                const statusContainer = document.getElementById('network-status-container');
                const statusText = document.getElementById('network-status-text');
                const statusIcon = document.getElementById('network-icon');
                const wifiIcon = document.getElementById('wifi-icon');
                const wifiStatus = document.getElementById('wifi-status');
              
                statusText.innerText = data;
                statusContainer.classList.remove('connected', 'ap-mode', 'disconnected');
              
                if (data.includes('Connected to')) {
                    statusContainer.classList.add('connected');
                    statusIcon.className = 'fas fa-wifi';
                    wifiIcon.className = 'fas fa-wifi status-icon active';
                    wifiStatus.innerText = 'Connected';
                } else if (data.includes('Access Point Mode')) {
                    statusContainer.classList.add('ap-mode');
                    statusIcon.className = 'fas fa-broadcast-tower';
                    wifiIcon.className = 'fas fa-broadcast-tower status-icon active';
                    wifiStatus.innerText = 'AP Mode';
                } else {
                    statusContainer.classList.add('disconnected');
                    statusIcon.className = 'fas fa-exclamation-triangle';
                    wifiIcon.className = 'fas fa-wifi status-icon';
                    wifiStatus.innerText = 'Disconnected';
                }
            });
        }
      
        function toggleAlarmSet(state) {
            fetch('/toggleAlarm?state=' + state)
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateAlarmInfo();
                updateStatusIndicators();
            });
        }
      
        function toggleRelay(state) {
            fetch('/toggleRelay?state=' + state)
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatusIndicators();
            });
        }
      
        function testAlarm() {
            animateButton('testAlarmBtn');
            fetch('/testAlarm')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatusIndicators();
                document.getElementById('alarmPulse').classList.add('active');
                setTimeout(() => {
                    document.getElementById('alarmPulse').classList.remove('active');
                }, 5000);
            });
        }
      
        function turnOffMotor() {
            animateButton('turnOffMotorBtn');
            fetch('/turnOffMotor')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatusIndicators();
            });
        }
      
        function quickTest() {
            animateButton('quickTestBtn');
            fetch('/quickTest')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                const relayIcon = document.getElementById('relay-icon');
                relayIcon.classList.add('active');
                setTimeout(() => relayIcon.classList.remove('active'), 3000);
            });
        }
      
        function singleFlashTest() {
            animateButton('singleFlashBtn');
            fetch('/singleFlashTest')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                const relayIcon = document.getElementById('relay-icon');
                relayIcon.classList.add('active');
                setTimeout(() => relayIcon.classList.remove('active'), 1000);
            });
        }
      
        function attemptWiFiReconnect() {
            animateButton('reconnectBtn');
          
            const notification = document.createElement('div');
            notification.style.position = 'fixed';
            notification.style.top = '20px';
            notification.style.left = '50%';
            notification.style.transform = 'translateX(-50%)';
            notification.style.backgroundColor = 'rgba(0, 229, 255, 0.2)';
            notification.style.borderRadius = '8px';
            notification.style.padding = '12px 20px';
            notification.style.boxShadow = '0 0 15px rgba(0, 229, 255, 0.3)';
            notification.style.border = '1px solid var(--primary)';
            notification.style.color = 'white';
            notification.style.fontFamily = 'Rajdhani, sans-serif';
            notification.style.zIndex = '1000';
            notification.innerHTML = '<i class="fas fa-sync fa-spin" style="margin-right: 10px;"></i> Reconnecting to Matrix...';
          
            document.body.appendChild(notification);
          
            fetch('/reconnectWiFi')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                notification.innerHTML = `<i class="fas fa-check-circle" style="margin-right: 10px; color: var(--success);"></i> ${data}`;
              
                setTimeout(() => {
                    notification.style.opacity = '0';
                    notification.style.transition = 'opacity 0.5s ease-out';
                    setTimeout(() => notification.remove(), 500);
                }, 5000);
              
                setTimeout(() => {
                    updateNetworkStatus();
                    updateStatusIndicators();
                }, 2000);
            })
            .catch(error => {
                notification.innerHTML = `<i class="fas fa-exclamation-triangle" style="margin-right: 10px; color: var(--danger);"></i> Connection failed!`;
                setTimeout(() => {
                    notification.style.opacity = '0';
                    notification.style.transition = 'opacity 0.5s ease-out';
                    setTimeout(() => notification.remove(), 500);
                }, 5000);
            });
        }
      
        function updateStatusIndicators() {
            fetch('/alarmsStatus')
            .then(response => response.json())
            .then(data => {
                document.getElementById('alarmToggle').checked = data.alarmsEnabled;
                document.getElementById('relayToggle').checked = data.relayState;
              
                const alarmIcon = document.getElementById('alarm-icon');
                const alarmStatus = document.getElementById('alarm-status');
              
                if (data.alarmsEnabled) {
                    alarmIcon.classList.add('active');
                    alarmStatus.innerText = 'Active';
                } else {
                    alarmIcon.classList.remove('active');
                    alarmStatus.innerText = 'Standby';
                }
              
                const relayIcon = document.getElementById('relay-icon');
                const relayStatus = document.getElementById('relay-status');
              
                if (data.relayState) {
                    relayIcon.classList.add('active');
                    relayStatus.innerText = 'Active';
                } else {
                    relayIcon.classList.remove('active');
                    relayStatus.innerText = 'Inactive';
                }
            });
        }
 
        const patterns = [
            { name: "Standard", sequence: [75, 75, 75, 75, 75, 900, 250, 250, 1000, 900, 50, 50, 50, 100, 50, 50, 50, 400, 50, 500] },
            { name: "Double Tap", sequence: [250, 75, 250, 75] },
            { name: "SOS", sequence: [50, 70] },
            { name: "Heartbeat", sequence: [500, 150, 500, 1000] },
            { name: "Escalating", sequence: [100, 300, 200, 300, 300, 300, 400, 300] },
            { name: "Random", sequence: [] }
        ];
       
        let currentPatternIndex = 0;
       
        function updatePatternVisual() {
            const visualContainer = document.getElementById('pattern-visual');
            const patternName = document.getElementById('pattern-name');
           
            visualContainer.innerHTML = '';
            patternName.innerText = patterns[currentPatternIndex].name;
           
            const pattern = patterns[currentPatternIndex];
           
            if (currentPatternIndex === patterns.length - 1) {
                visualContainer.innerHTML = '<i class="fas fa-random" style="font-size: 1.5rem; color: var(--primary);"></i>';
                return;
            }
           
            for (let i = 0; i < pattern.sequence.length; i++) {
                const dot = document.createElement('div');
                dot.classList.add('pattern-dot');
               
                const size = 4 + ((pattern.sequence[i] / 600) * 8);
                dot.style.width = `${size}px`;
                dot.style.height = `${size}px`;
               
                if (i % 2 === 0) {
                    dot.classList.add('active');
                }
               
                visualContainer.appendChild(dot);
               
                if (i < pattern.sequence.length - 1) {
                    const space = document.createElement('div');
                    space.style.width = '3px';
                    visualContainer.appendChild(space);
                }
            }
           
            animatePattern();
        }
       
        function animatePattern() {
            const dots = document.querySelectorAll('.pattern-dot');
            if (dots.length === 0) return;
           
            if (currentPatternIndex === patterns.length - 1) return;
           
            let currentStep = 0;
            const pattern = patterns[currentPatternIndex];
           
            const intervalId = setInterval(() => {
                dots.forEach((dot, i) => {
                    if (i % 2 === 0) {
                        dot.style.opacity = '0.6';
                        dot.style.boxShadow = 'none';
                    }
                });
               
                if (currentStep % 2 === 0 && currentStep < dots.length) {
                    dots[currentStep].style.opacity = '1';
                    dots[currentStep].style.boxShadow = '0 0 10px var(--primary-glow)';
                }
               
                currentStep = (currentStep + 1) % pattern.sequence.length;
               
                if (currentStep === 0) {
                    clearInterval(intervalId);
                }
            }, 300);
        }
       
        function changePattern(direction) {
            if (direction === 'next') {
                currentPatternIndex = (currentPatternIndex + 1) % patterns.length;
            } else {
                currentPatternIndex = (currentPatternIndex + patterns.length - 1) % patterns.length;
            }
           
            updatePatternVisual();
           
            fetch('/setPattern?index=' + currentPatternIndex)
            .then(response => response.text())
            .then(data => {
                console.log(data);
            });
        }
      
        window.onload = function() {
            createParticles();
            updateTime();
            updateDateInfo();
            updateAlarmInfo();
            updateNetworkStatus();
            updateStatusIndicators();
            updatePatternVisual();
          
            setInterval(updateTime, 1000);
            setInterval(updateDateInfo, 60000);
            setInterval(updateAlarmInfo, 60000);
            setInterval(updateStatusIndicators, 10000);
            setInterval(updateNetworkStatus, 30000);
        }
    </script>
</body>
</html>)rawliteral");
}