#include <Arduino.h>
#include <AccelStepper.h>

AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

// --- Вхідні сигнали ---
#define SIG1 6
#define SIG2 7

// --- Мотор 1 (Green) ---
#define D1_IN1 9
#define D1_IN2 10
#define D1_EN 8

// --- Мотор 2 (Red) ---
#define D2_IN1 12
#define D2_IN2 13
#define D2_EN 11

// --- Таймінги ---
const unsigned long pullInterval = 15000;  // період авто підтягування
const unsigned long pushDuration = 100;    // імпульс пушу 100 мс
const unsigned long debounceInterval = 300; // захист від повторного спрацювання

unsigned long lastPullTime = 0;
unsigned long lastSig1Time = 0;
unsigned long lastSig2Time = 0;
unsigned long kicker1Start = 0;
unsigned long kicker2Start = 0;

bool kicker1Active = false;
bool kicker2Active = false;

// --- Прототипи ---
void kicker1Push();
void kicker1Release();
void kicker2Push();
void kicker2Release();
void kicker1Backward();
void kicker2Backward();

void setup() {
  Serial.begin(9600);

  // --- Входи ---
  pinMode(SIG1, INPUT);
  pinMode(SIG2, INPUT);

  // --- Мотори ---
  pinMode(D1_IN1, OUTPUT);
  pinMode(D1_IN2, OUTPUT);
  pinMode(D1_EN, OUTPUT);

  pinMode(D2_IN1, OUTPUT);
  pinMode(D2_IN2, OUTPUT);
  pinMode(D2_EN, OUTPUT);

  kicker1Release();
  kicker2Release();

  // --- Налаштування крокового ---
  stepper.setMaxSpeed(1000);
  stepper.setSpeed(1000);
}

void loop() {
  unsigned long now = millis();
  stepper.runSpeed();

  // ---- Сигнали з debounce ----
  if (digitalRead(SIG1) == HIGH && (now - lastSig1Time >= debounceInterval) && !kicker1Active) {
    kicker1Push();
    kicker1Start = now;
    kicker1Active = true;
    lastSig1Time = now;
  }

  if (digitalRead(SIG2) == HIGH && (now - lastSig2Time >= debounceInterval) && !kicker2Active) {
    kicker2Push();
    kicker2Start = now;
    kicker2Active = true;
    lastSig2Time = now;
  }

  // ---- Авто підтягування кожні 15 сек ----
  if (!kicker1Active && !kicker2Active && now - lastPullTime >= pullInterval) {
    Serial.println("Auto pull");
    kicker1Backward();
    kicker2Backward();
    kicker1Start = now;
    kicker2Start = now;
    kicker1Active = true;
    kicker2Active = true;
    lastPullTime = now;
  }

  // ---- Вимкнення моторів після pushDuration мс ----
  if (kicker1Active && now - kicker1Start >= pushDuration) {
    kicker1Release();
    kicker1Active = false;
  }
  if (kicker2Active && now - kicker2Start >= pushDuration) {
    kicker2Release();
    kicker2Active = false;
  }
}

// ---- Керування моторами ----
void kicker1Push() {
  Serial.println("Motor 1 ON");
  digitalWrite(D1_IN1, HIGH);
  digitalWrite(D1_IN2, LOW);
  analogWrite(D1_EN, 200);
}

void kicker1Release() {
  Serial.println("Motor 1 OFF");
  digitalWrite(D1_IN1, LOW);
  digitalWrite(D1_IN2, LOW);
  analogWrite(D1_EN, 0);
}

void kicker2Push() {
  Serial.println("Motor 2 ON");
  digitalWrite(D2_IN1, HIGH);
  digitalWrite(D2_IN2, LOW);
  analogWrite(D2_EN, 200);
}

void kicker2Release() {
  Serial.println("Motor 2 OFF");
  digitalWrite(D2_IN1, LOW);
  digitalWrite(D2_IN2, LOW);
  analogWrite(D2_EN, 0);
}

// ---- Підтягування назад ----
void kicker1Backward() {
  Serial.println("Motor 1 BACKWARD");
  digitalWrite(D1_IN1, LOW);
  digitalWrite(D1_IN2, HIGH);
  analogWrite(D1_EN, 200);
}

void kicker2Backward() {
  Serial.println("Motor 2 BACKWARD");
  digitalWrite(D2_IN1, LOW);
  digitalWrite(D2_IN2, HIGH);
  analogWrite(D2_EN, 200);
}
