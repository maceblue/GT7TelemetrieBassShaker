#ifndef CONFIG_H
#define CONFIG_H

// WiFi-Konfiguration
extern char* ssid;
extern char* password;

// Vibrationseinstellungen (Standardwerte)
extern int BASE_FREQUENCY;
extern int FREQUENCY_PER_INTENSITY;
extern int GEAR_SHIFT_FREQUENCY;
extern int NORMAL_FREQUENCY;
extern int GEAR_SHIFT_DURATION;
extern int RPM_MAX;
extern int RPM_MIN;
extern float AMPLITUDE_FACTOR;
extern int FREQUENZ_DIVISOR;
extern float TIRE_SLIP_FACTOR;

// Variablen zur Steuerung der Vibrationsmethoden
extern bool useTireSlip;
extern bool useRPM;

// Intensität der Vibrationen in Prozent
extern int tireSlipIntensity;
extern int rpmIntensity;

// Variablen zur Überwachung von Änderungen
extern unsigned long lastChangeTime;
extern float lastRPM;
extern float lastTireSlip;
extern unsigned long STOP_VIBRATION_DELAY;

#endif // CONFIG_H
