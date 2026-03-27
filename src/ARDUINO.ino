#include <Arduino.h>
#include <AccelStepper.h>

// -------------------- Піни --------------------
// Сигнали для DC моторів
#define SIG1 6
#define SIG2 7

// DC Мотор 1
#define D1_IN1 8
#define D1_IN2 9
#define D1_EN  10

// DC Мотор 2
#define D2_IN1 13
#define D2_IN2 12
#define D2_EN  11

// Stepper (FULL4WIRE на 2,3,4,5)
AccelStepper stepper(AccelStepper::FULL4WIRE, 3,2, 5,4);

// -------------------- Таймінги --------------------
const uint16_t PULL_PERIOD  = 20000; // підтягування раз у 20 c
const uint8_t M1_FORWARD_TIME   = 200;   // мс рух вперед (скидання)
const uint8_t M2_FORWARD_TIME   = 200;   // мс рух вперед (скидання)
const uint8_t M1_BACKWARD_TIME  = 200;   // мс рух назад (піднімання)
const uint8_t M2_BACKWARD_TIME  = 200;   // мс рух назад (піднімання)

// PWM для різних режимів
const uint8_t M1_KICK_PWM = 90;   // Скидання M1
const uint8_t M2_KICK_PWM = 90;   // Скидання M2
const uint8_t M1_PULL_PWM = 70;   // Підтягування M1
const uint8_t M2_PULL_PWM = 50;   // Підтягування M2

// -------------------- Стани DC моторів --------------------
enum State { IDLE, FORWARD, BACKWARD };

struct Motor {
  byte in1, in2, en;
  State state;
  unsigned long tStart;
  bool active;

  Motor(byte _in1, byte _in2, byte _en)
    : in1(_in1), in2(_in2), en(_en), state(IDLE), tStart(0), active(false) {}
};

Motor m1(D1_IN1, D1_IN2, D1_EN);
Motor m2(D2_IN1, D2_IN2, D2_EN);

// Фіксація фронтів сигналів
bool sig1Prev = false;
bool sig2Prev = false;

// Таймер авто-підтягування
unsigned long lastPull = 0;

// -------------------- Утиліти керування DC --------------------
// ⚡️ Тут я перевернув полярність (міняємо in1/in2 місцями)
inline void motorForward(Motor &m, int pwm, const char *name) {
  digitalWrite(m.in1, LOW);
  digitalWrite(m.in2, HIGH);
  analogWrite(m.en, pwm);
  m.state = FORWARD;
  m.tStart = millis();
  m.active = true;
  Serial.print(name); Serial.println(" FORWARD");
}

inline void motorBackward(Motor &m, int pwm, const char *name) {
  digitalWrite(m.in1, HIGH);
  digitalWrite(m.in2, LOW);
  analogWrite(m.en, pwm);
  m.state = BACKWARD;
  m.tStart = millis();
  m.active = true;
  Serial.print(name); Serial.println(" BACKWARD");
}

inline void motorStop(Motor &m, const char *name) {
  digitalWrite(m.in1, LOW);
  digitalWrite(m.in2, LOW);
  analogWrite(m.en, 0);
  m.state = IDLE;
  m.active = false;
  Serial.print(name); Serial.println(" STOP");
}

inline void handleMotor(Motor &m, unsigned long fwdMs, unsigned long backMs, int pwm, const char *name) {

  unsigned long now = millis();
  switch (m.state) {
    case FORWARD:
      if (now - m.tStart >= fwdMs) {
        motorBackward(m, pwm, name);
      }
      break;
    case BACKWARD:
      if (now - m.tStart >= backMs) {
        motorStop(m, name);
      }
      break;
    default: break;
  }
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(9600);

  pinMode(SIG1, INPUT);
  pinMode(SIG2, INPUT);

  pinMode(m1.in1, OUTPUT);
  pinMode(m1.in2, OUTPUT);
  pinMode(m1.en,  OUTPUT);

  pinMode(m2.in1, OUTPUT);
  pinMode(m2.in2, OUTPUT);
  pinMode(m2.en,  OUTPUT);

  motorBackward(m1, M1_PULL_PWM, "M1");
  motorBackward(m2, M2_PULL_PWM, "M2");

  delay(100);

  motorStop(m1, "M1");
  motorStop(m2, "M2");

  // Безперервна робота кроковика
  stepper.setMaxSpeed(1500); 
  stepper.setSpeed(1000);    

  Serial.println("=== SETUP DONE ===");
}

// -------------------- LOOP --------------------
void loop() {
  uint64_t now = millis();

  // Кроковий крутиться постійно
  stepper.runSpeed();

  // ----- Фронти сигналів -----
  bool s1 = digitalRead(SIG1);
  bool s2 = digitalRead(SIG2);

  if (s1 && !sig1Prev && !m1.active) {
    Serial.println("SIG1 TRIGGER");
    motorForward(m1, M1_KICK_PWM, "M1");   // Скидання
  }
  if (s2 && !sig2Prev && !m2.active) {
    Serial.println("SIG2 TRIGGER");
    motorForward(m2, M2_KICK_PWM, "M2");   // Скидання
  }

  sig1Prev = s1;
  sig2Prev = s2;

  // ----- Авто-підтягування раз у 20 с -----
  if ((now - lastPull) >= PULL_PERIOD && !m1.active && !m2.active) {
    Serial.println("=== AUTO PULL ===");
    motorBackward(m1, M1_PULL_PWM, "M1");
    motorBackward(m2, M2_PULL_PWM, "M2");
    lastPull = now;
  }

  // ----- Обробка станів DC моторів -----
  handleMotor(m1, M1_FORWARD_TIME, M1_BACKWARD_TIME, M1_KICK_PWM, "M1");
  handleMotor(m2, M2_FORWARD_TIME, M2_BACKWARD_TIME, M2_KICK_PWM, "M2");
}
