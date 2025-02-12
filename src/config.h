#ifndef CONFIG_H
#define CONFIG_H

// WiFi-Konfiguration
extern const char* ssid;
extern const char* password;

// IP-Adresse als separate Bytes
extern const byte ip_part1;
extern const byte ip_part2;
extern const byte ip_part3;
extern const byte ip_part4;

extern const IPAddress playstationIP;

// Vibrationseinstellungen (Standardwerte)
extern int BASE_FREQUENCY;
extern int FREQUENCY_PER_INTENSITY;
extern int GEAR_SHIFT_FREQUENCY;
extern int NORMAL_FREQUENCY;
extern int GEAR_SHIFT_DURATION;
extern int RPM_MAX;
extern int RPM_MIN;
extern float AMPLITUDE_FACTOR;
extern int FREQUENCY_DIVISOR;
extern float TIRE_SLIP_FACTOR;

// Variablen zur Steuerung der Vibrationsmethoden
extern bool useTireSlip;
extern bool useRPM;
extern bool useSuspHeight;

// Intensität der Vibrationen in Prozent
extern int tireSlipIntensity;
extern int rpmIntensity;
extern int suspHeightIntensity;

// Variablen zur Überwachung von Änderungen
extern unsigned long lastChangeTime;
extern float lastRPM;
extern float lastTireSlip;
extern float lastSuspHeight;
extern const unsigned long STOP_VIBRATION_DELAY;

#endif // CONFIG_H
