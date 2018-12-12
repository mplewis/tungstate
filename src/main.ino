#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Bounce2.h>

#define BEEP_PIN 3
#define LED_PIN 10

Bounce button1 = Bounce();
Bounce button2 = Bounce();
Bounce button3 = Bounce();

void setup() {
  Serial.begin(115200);

  pinMode(BEEP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BEEP_PIN, HIGH);

  button1.attach(A1, INPUT_PULLUP);
  button1.interval(20);
  button2.attach(A2, INPUT_PULLUP);
  button2.interval(20);
  button3.attach(A3, INPUT_PULLUP);
  button3.interval(20);

  xTaskCreate(CheckButtons, "CheckButtons", 100, NULL, 2, NULL);
  xTaskCreate(BeepOnDemand, "BeepOnDemand", 100, NULL, 1, NULL);
  xTaskCreate(IdleTask, "IdleTask", 100, NULL, 0, NULL);
}

void loop() {}

bool shouldBeep = false;
void beep() { shouldBeep = true; }

void BeepOnDemand(void* pvParameters) {
  while (true) {
    Serial.println(F("BeepOnDemand"));

    if (shouldBeep)
      Serial.println("beep!");
    else
      Serial.println("nope");

    if (shouldBeep) {
      digitalWrite(BEEP_PIN, LOW);
      vTaskDelay(50 / portTICK_PERIOD_MS);
      digitalWrite(BEEP_PIN, HIGH);
      shouldBeep = false;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void CheckButtons(void* pvParameters) {
  while (true) {
    Serial.println(F("CheckButtons"));

    button1.update();
    button2.update();
    button3.update();
    if (button1.fell()) {
      beep();
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void IdleTask(void* pvParameters) {
  while (true) delay(1);
}
