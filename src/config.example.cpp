#include <Arduino.h>
#include "config.h"

// WiFi-Konfiguration
const char* ssid = "xxxxxxxx";
const char* password = "xxxxxxxx";

// IP-Adresse als separate Bytes
const byte ip_part1 = 192;
const byte ip_part2 = 168;
const byte ip_part3 = 178;
const byte ip_part4 = 99;

// IP-Adresse aus den Bytes erstellen
const IPAddress playstationIP(ip_part1, ip_part2, ip_part3, ip_part4);

// Vibrationseinstellungen (Standardwerte)
int BASE_FREQUENCY = 20;
int FREQUENCY_PER_INTENSITY = 1;
int GEAR_SHIFT_FREQUENCY = 30;
int NORMAL_FREQUENCY = 20;
int GEAR_SHIFT_DURATION = 100;
int RPM_MAX = 8000;
int RPM_MIN = 0;
float AMPLITUDE_FACTOR = 0.01;
int FREQUENCY_DIVISOR = 75;
float TIRE_SLIP_FACTOR = 70.0;

// Variablen zur Steuerung der Vibrationsmethoden
bool useTireSlip = true;
bool useRPM = true;
bool useSuspHeight = true;

// Intensität der Vibrationen in Prozent
int tireSlipIntensity = 50;
int rpmIntensity = 50;
int suspHeightIntensity = 50;

// Variablen zur Überwachung von Änderungen
unsigned long lastChangeTime = 0;
float lastRPM = 0;
float lastTireSlip = 0;
float lastSuspHeight = 0;
const unsigned long STOP_VIBRATION_DELAY = 5000; // 5 Sekunden
