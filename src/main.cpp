#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "GT7UDPParser.h"
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"

// WiFi-Konfiguration
const char* ssid = "Villa Kunterbunt";
const char* password = "bc25fcb38b";
const IPAddress ip(192, 168, 178, 99);

// Webserver
WebServer server(80);

// Vibrationseinstellungen (Standardwerte)
int BASE_FREQUENCY = 20;         // Basis-Frequenz in Hz
int FREQUENCY_PER_INTENSITY = 1; // Hz pro Intensitätspunkt
int GEAR_SHIFT_FREQUENCY = 30;   // Frequenz für Gangwechsel
int NORMAL_FREQUENCY = 20;       // Normale Frequenz nach Gangwechsel
int GEAR_SHIFT_DURATION = 100;   // Dauer in ms
int RPM_MAX = 8000;              // Maximale Drehzahl
int RPM_MIN = 0;                 // Minimale Drehzahl
float AMPLITUDE_FACTOR = 0.01;   // Faktor zur Anpassung der Amplitude
int FREQUENZ_DIVISOR = 75;       // Divisor für die Frequenzberechnung
float TIRE_SLIP_FACTOR = 70.0;   // Faktor zur Anpassung der Frequenz basierend auf Reifenschlupf

// Variablen zur Steuerung der Vibrationsmethoden
bool useTireSlip = true;  // Vibration basierend auf Reifenschlupf aktivieren
bool useRPM = true;        // Vibration basierend auf RPM aktivieren

// Intensität der Vibrationen in Prozent
int tireSlipIntensity = 50; // Intensität der Reifenschlupf-Vibration
int rpmIntensity = 50;      // Intensität der RPM-Vibration

// Globale Variablen
GT7_UDP_Parser gt7Telem;
Packet packetContent;

unsigned long previousT = 0;
const long interval = 500;
uint8_t previousGear = 0;
const int LED_PIN = 2;

// Audio-Generierung
AudioInfo info(32000, 2, 16);
SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> sound(sineWave);
AudioBoardStream out(AudioKitEs8388V1);
StreamCopy copier(out, sound);

// Variablen zur Überwachung von Änderungen
unsigned long lastChangeTime = 0;
float lastRPM = 0;
float lastTireSlip = 0;
const unsigned long STOP_VIBRATION_DELAY = 5000; // 5 Sekunden

// Funktionsdeklarationen
void processTelemetryData(Packet packetContent);
int generateAudioSignalFromRPM(float rpm);
int generateTireSlipVibration(float tireSlip);
void generateGearChangeVibration();
void printTelemetry(float speed, float rpm, int intensity);
void handleRoot();
void handleUpdate();

void setup() {
  Serial.begin(115200);

  // WiFi verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(5000);
  Serial.println("WiFi verbunden");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Webserver starten
  server.on("/", handleRoot);      // Hauptseite
  server.on("/update", handleUpdate); // Parameter aktualisieren
  server.begin();
  Serial.println("Webserver gestartet");

  // Audio initialisieren
  auto config = out.defaultConfig(TX_MODE);
  config.copyFrom(info);
  out.begin(config);
  sineWave.begin(info, N_B4);

  // GT7 Telemetrie initialisieren
  gt7Telem.begin(ip);
  gt7Telem.sendHeartbeat();
}

void loop() {
  server.handleClient(); // Webserver-Anfragen verarbeiten

  unsigned long currentT = millis();
  packetContent = gt7Telem.readData();
  processTelemetryData(packetContent);

  if (currentT - previousT >= interval) {
    previousT = currentT;
    gt7Telem.sendHeartbeat();
  }
  copier.copy();
}

void processTelemetryData(Packet packetContent) {
  float speed = packetContent.packetContent.speed * 3.6;
  float rpm = packetContent.packetContent.EngineRPM;

  float tireSlip1 = gt7Telem.getTyreSlipRatio(0);
  float tireSlip2 = gt7Telem.getTyreSlipRatio(1);
  float tireSlip3 = gt7Telem.getTyreSlipRatio(2);
  float tireSlip4 = gt7Telem.getTyreSlipRatio(3);

  // Gesamtschlupf basierend auf der Abweichung von 1 berechnen
  float totalTireSlip = abs(tireSlip1 - 1) + abs(tireSlip2 - 1) + abs(tireSlip3 - 1) + abs(tireSlip4 - 1);

  // Gangwechsel überprüfen
  uint8_t currentGear = packetContent.packetContent.gears & 0b00001111;
  if (currentGear != previousGear) {
    generateGearChangeVibration();
    previousGear = currentGear;
  }

  // Frequenz und Amplitude für den Bass Shaker setzen
  if (speed > 0) {
    int frequency = NORMAL_FREQUENCY;

    if (useRPM) {
      frequency = generateAudioSignalFromRPM(rpm);
      if (rpm != lastRPM) {
        lastRPM = rpm;
        lastChangeTime = millis(); // Änderung erkannt, Timer zurücksetzen
      }
    }

    if (useTireSlip) {
      int tireSlipFrequency = generateTireSlipVibration(totalTireSlip);
      if (totalTireSlip != lastTireSlip) {
        lastTireSlip = totalTireSlip;
        lastChangeTime = millis(); // Änderung erkannt, Timer zurücksetzen
      }
      if (useRPM) {
        // Gewichteter Durchschnitt der beiden Frequenzen berechnen
        frequency = (frequency * rpmIntensity + tireSlipFrequency * tireSlipIntensity) / (rpmIntensity + tireSlipIntensity);
      } else {
        frequency = tireSlipFrequency;
      }
    }

    // Vibration stoppen, wenn sich die Werte nicht ändern
    if (millis() - lastChangeTime > STOP_VIBRATION_DELAY) {
      sineWave.setFrequency(0); // Vibration stoppen
    } else {
      sineWave.setFrequency(frequency);
    }
  }
}

int generateAudioSignalFromRPM(float rpm) {
  // Frequenz auf einen sinnvollen Bereich begrenzen (10 Hz bis 100 Hz)
  int frequency = constrain(rpm / FREQUENZ_DIVISOR, 20, 90);
  Serial.print("RPM Frequency: ");
  Serial.println(frequency);
  return frequency;
}

int generateTireSlipVibration(float tireSlip) {
  // Frequenz basierend auf dem Reifenschlupf berechnen
  int frequency = constrain(20 + tireSlip * TIRE_SLIP_FACTOR, 20, 90);
  Serial.print("Tire Slip Frequency: ");
  Serial.println(frequency);
  return frequency;
}

void generateGearChangeVibration() {
  sineWave.setFrequency(GEAR_SHIFT_FREQUENCY);
  delay(GEAR_SHIFT_DURATION);
  sineWave.setFrequency(NORMAL_FREQUENCY);
}

void printTelemetry(float speed, float rpm, int intensity) {
  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.print(" km/h | RPM: ");
  Serial.print(rpm);
  Serial.print(" | Vibration: ");
  Serial.print(intensity);
  Serial.println("%");
}

// Webserver-Handler für die Hauptseite
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Vibrationseinstellungen</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background-color: black; color: white; }
    label { display: block; margin-top: 10px; }
    input { width: 100%; padding: 5px; margin-top: 5px; }
    button { margin-top: 20px; padding: 10px 20px; }
  </style>
</head>
<body>
  <h1>Vibrationseinstellungen</h1>
  <form action="/update" method="GET">
    <label for="base_freq">Basis-Frequenz (Hz):</label>
    <input type="number" id="base_freq" name="base_freq" value=")=====";
  html += BASE_FREQUENCY;
  html += R"=====(">

    <label for="freq_per_int">Frequenz pro Intensität (Hz):</label>
    <input type="number" id="freq_per_int" name="freq_per_int" value=")=====";
  html += FREQUENCY_PER_INTENSITY;
  html += R"=====(">

    <label for="gear_shift_freq">Gangwechsel-Frequenz (Hz):</label>
    <input type="number" id="gear_shift_freq" name="gear_shift_freq" value=")=====";
  html += GEAR_SHIFT_FREQUENCY;
  html += R"=====(">

    <label for="normal_freq">Normale Frequenz (Hz):</label>
    <input type="number" id="normal_freq" name="normal_freq" value=")=====";
  html += NORMAL_FREQUENCY;
  html += R"=====(">

    <label for="gear_shift_dur">Gangwechsel-Dauer (ms):</label>
    <input type="number" id="gear_shift_dur" name="gear_shift_dur" value=")=====";
  html += GEAR_SHIFT_DURATION;
  html += R"=====(">

    <label for="tire_slip_factor">Reifenschlupf-Faktor:</label>
    <input type="number" step="0.01" id="tire_slip_factor" name="tire_slip_factor" value=")=====";
  html += TIRE_SLIP_FACTOR;
  html += R"=====(">

    <label for="use_tire_slip">Reifenschlupf verwenden:</label>
    <select id="use_tire_slip" name="use_tire_slip">
      <option value="1" )=====";
  html += useTireSlip ? "selected" : "";
  html += R"=====(>Ja</option>
      <option value="0" )=====";
  html += !useTireSlip ? "selected" : "";
  html += R"=====(>Nein</option>
    </select>

    <label for="use_rpm">RPM verwenden:</label>
    <select id="use_rpm" name="use_rpm">
      <option value="1" )=====";
  html += useRPM ? "selected" : "";
  html += R"=====(>Ja</option>
      <option value="0" )=====";
  html += !useRPM ? "selected" : "";
  html += R"=====(>Nein</option>
    </select>

    <label for="tire_slip_intensity">Reifenschlupf-Intensität (%):</label>
    <input type="range" id="tire_slip_intensity" name="tire_slip_intensity" min="0" max="100" value=")=====";
  html += tireSlipIntensity;
  html += R"=====(">
    <span id="tire_slip_intensity_value">)=====";
  html += tireSlipIntensity;
  html += R"=====(</span>%

    <label for="rpm_intensity">RPM-Intensität (%):</label>
    <input type="range" id="rpm_intensity" name="rpm_intensity" min="0" max="100" value=")=====";
  html += rpmIntensity;
  html += R"=====(">
    <span id="rpm_intensity_value">)=====";
  html += rpmIntensity;
  html += R"=====(</span>%

    <button type="submit">Aktualisieren</button>
  </form>

  <script>
    document.getElementById('tire_slip_intensity').addEventListener('input', function() {
      document.getElementById('tire_slip_intensity_value').textContent = this.value;
    });
    document.getElementById('rpm_intensity').addEventListener('input', function() {
      document.getElementById('rpm_intensity_value').textContent = this.value;
    });
  </script>
</body>
</html>
)=====";
  server.send(200, "text/html", html);
}

// Webserver-Handler für die Parameteraktualisierung
void handleUpdate() {
  if (server.hasArg("base_freq")) BASE_FREQUENCY = server.arg("base_freq").toInt();
  if (server.hasArg("freq_per_int")) FREQUENCY_PER_INTENSITY = server.arg("freq_per_int").toInt();
  if (server.hasArg("gear_shift_freq")) GEAR_SHIFT_FREQUENCY = server.arg("gear_shift_freq").toInt();
  if (server.hasArg("normal_freq")) NORMAL_FREQUENCY = server.arg("normal_freq").toInt();
  if (server.hasArg("gear_shift_dur")) GEAR_SHIFT_DURATION = server.arg("gear_shift_dur").toInt();
  if (server.hasArg("tire_slip_factor")) TIRE_SLIP_FACTOR = server.arg("tire_slip_factor").toFloat();
  if (server.hasArg("use_tire_slip")) useTireSlip = server.arg("use_tire_slip").toInt() == 1;
  if (server.hasArg("use_rpm")) useRPM = server.arg("use_rpm").toInt() == 1;
  if (server.hasArg("tire_slip_intensity")) tireSlipIntensity = server.arg("tire_slip_intensity").toInt();
  if (server.hasArg("rpm_intensity")) rpmIntensity = server.arg("rpm_intensity").toInt();

  server.sendHeader("Location", "/");
  server.send(303);
}
