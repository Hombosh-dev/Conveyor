#include "Wire.h"
#include "Adafruit_TCS34725.h"

// --- Піни моторів Dropper ---
#define D1_EN 19 // Green
#define D2_EN 18 // Red

// Затримки до скидачів (налаштовуєш сам)
unsigned long delayToDropper1 = 3480; // Green
unsigned long delayToDropper2 = 4960; // Yellow

// Час пульса
unsigned long dropperPulse = 120;

// Час блокування повторного детекту (мс)
unsigned long detectionCooldown = 400;
unsigned long lastDetectionTime = 0;

Adafruit_TCS34725 tcs(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

struct Task {
  unsigned long triggerTime;
  int dropperId;
  bool active;
  unsigned long startTime;
  bool motorRunning;
};

#define MAX_TASKS 50
Task tasks[MAX_TASKS];
int taskCount = 0;

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(9600);

  pinMode(D1_EN, OUTPUT);
  pinMode(D2_EN, OUTPUT);

  stopMotor(1);
  stopMotor(2);

  if (!tcs.begin()) {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  Serial.println("Color sorter ready");
}

// -------------------- LOOP --------------------
void loop() {
  uint16_t r, g, b, c, lux;
  tcs.getRawData(&r, &g, &b, &c);
  lux = tcs.calculateLux(r, g, b);

  float sum = r + g + b;
  if (sum == 0) sum = 1;

  float nr = r / sum;
  float ng = g / sum;
  float nb = b / sum;

  String color = detectColorSimple(nr, ng, nb, lux);

  // Перевірка на cooldown
  if (millis() - lastDetectionTime >= detectionCooldown) {
    if (color == "Green") {
      addTask(1, millis() + delayToDropper1);
      lastDetectionTime = millis();
      Serial.println("Detected GREEN - scheduled for Dropper 1");
    }
    else if (color == "Yellow") {
      addTask(2, millis() + delayToDropper2);
      lastDetectionTime = millis();
      Serial.println("Detected YELLOW - scheduled for Dropper 2");
    }
  }

  checkTasks();
}

// -------------------- Детекція кольору --------------------
String detectColorSimple(float nr, float ng, float nb, uint16_t lux) {
  if (lux < 10) return "Too Dark";
  if (nr < 0.18 && ng < 0.18 && nb < 0.18) return "Black";
  if (abs(nr - ng) < 0.07 && abs(nr - nb) < 0.07 && nr > 0.35) return "White";
  if (nr > ng + 0.12 && nr > nb + 0.12) return "Red";
  if (ng > nr + 0.12 && ng > nb + 0.12) return "Green";
  if (nb > nr + 0.05 && nb > ng + 0.05 && nb > 0.25) return "Blue";
  if ((nr > 0.28 && ng > 0.28) && nb < 0.20) return "Yellow";
  return "Unknown";
}

// -------------------- Робота з задачами --------------------
void addTask(int dropperId, unsigned long triggerTime) {
  if (taskCount < MAX_TASKS) {
    tasks[taskCount].dropperId    = dropperId;
    tasks[taskCount].triggerTime  = triggerTime;
    tasks[taskCount].active       = false;
    tasks[taskCount].motorRunning = false;
    taskCount++;
  }
}

void checkTasks() {
  unsigned long now = millis();

  for (int i = 0; i < taskCount; i++) {
    if (!tasks[i].active && now >= tasks[i].triggerTime) {
      Serial.print("Activating Dropper ");
      Serial.println(tasks[i].dropperId);

      tasks[i].active       = true;
      tasks[i].motorRunning = true;
      tasks[i].startTime    = now;

      forwardMotor(tasks[i].dropperId);
    }

    if (tasks[i].motorRunning && now - tasks[i].startTime >= dropperPulse) {
      Serial.print("Stopping Dropper ");
      Serial.println(tasks[i].dropperId);

      stopMotor(tasks[i].dropperId);
      tasks[i].motorRunning = false;

      removeTask(i);
      i--;
    }
  }
}

void removeTask(int index) {
  for (int i = index; i < taskCount - 1; i++) {
    tasks[i] = tasks[i + 1];
  }
  taskCount--;
}

// -------------------- Функції мотора --------------------
void forwardMotor(int dropperId) {
  if (dropperId == 1) {
    digitalWrite(D1_EN, HIGH);
  } else {
    digitalWrite(D2_EN, HIGH);
  }
}

void stopMotor(int dropperId) {
  if (dropperId == 1) {
    digitalWrite(D1_EN, LOW);
  } else {
    digitalWrite(D2_EN, LOW);
  }
}
