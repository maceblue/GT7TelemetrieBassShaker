#include "config.h"

// WiFi-Konfiguration
char* ssid = "xxxxxx";
char* password = "xxxxxx";

// Vibrationseinstellungen (Standardwerte)
int BASE_FREQUENCY = 20;
int FREQUENCY_PER_INTENSITY = 1;
int GEAR_SHIFT_FREQUENCY = 30;
int NORMAL_FREQUENCY = 20;
int GEAR_SHIFT_DURATION = 100;
int RPM_MAX = 8000;
int RPM_MIN = 0;
float AMPLITUDE_FACTOR = 0.01;
int FREQUENZ_DIVISOR = 75;
float TIRE_SLIP_FACTOR = 70.0;

// Variablen zur Steuerung der Vibrationsmethoden
bool useTireSlip = true;
bool useRPM = true;

// Intensität der Vibrationen in Prozent
int tireSlipIntensity = 50;
int rpmIntensity = 50;

// Variablen zur Überwachung von Änderungen
unsigned long lastChangeTime = 0;
float lastRPM = 0;
float lastTireSlip = 0;
unsigned long STOP_VIBRATION_DELAY = 5000;
