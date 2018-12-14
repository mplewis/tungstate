#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Bounce2.h>
#include <DigitalIO.h>

#define BEEP_PIN 3
#define LED_PIN 10
#define LATCH_PIN 4
#define CLK_PIN 7
#define DATA_PIN 8

const byte ALL_DIGITS_OFF = 0x00;
const byte ALL_SEGMENTS_OFF = 0xFF;
const byte DIGIT_AT_INDEX[] = {0x01, 0x02, 0x04, 0x08};
const byte NUM_TO_SEGMENTS[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
                                0x92, 0x82, 0xF8, 0X80, 0X90};

Bounce button1 = Bounce();
Bounce button2 = Bounce();
Bounce button3 = Bounce();

SoftSPI<DATA_PIN, DATA_PIN, CLK_PIN, 0> display;

void setup() {
  Serial.begin(115200);

  pinMode(BEEP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(LATCH_PIN, OUTPUT);
  display.begin();

  digitalWrite(LATCH_PIN, LOW);
  display.send(ALL_SEGMENTS_OFF);
  display.send(ALL_DIGITS_OFF);
  digitalWrite(LATCH_PIN, HIGH);

  digitalWrite(BEEP_PIN, HIGH);

  button1.attach(A1, INPUT_PULLUP);
  button1.interval(20);
  button2.attach(A2, INPUT_PULLUP);
  button2.interval(20);
  button3.attach(A3, INPUT_PULLUP);
  button3.interval(20);

  xTaskCreate(SpinSegments, "SpinSegments", 100, NULL, 3, NULL);
  xTaskCreate(CheckButtons, "CheckButtons", 100, NULL, 2, NULL);
  xTaskCreate(BeepOnDemand, "BeepOnDemand", 100, NULL, 1, NULL);
  xTaskCreate(IdleTask, "IdleTask", 100, NULL, 0, NULL);
}

void loop() {}

bool shouldBeep = false;
void beep() { shouldBeep = true; }

void BeepOnDemand(void* _) {
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

// 0-7: A-G + DP, on = LOW
// 8, 9, 10, 11: segments from left to right, on = HIGH

uint8_t digit = 0;
uint8_t count = 0;
void SpinSegments(void* _) {
  while (true) {
    digitalWrite(LATCH_PIN, LOW);
    display.send(NUM_TO_SEGMENTS[count]);
    display.send(DIGIT_AT_INDEX[digit]);
    digitalWrite(LATCH_PIN, HIGH);

    digit = (digit + 1) % 4;
    count = (count + 1) % 10;

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void CheckButtons(void* _) {
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

void IdleTask(void* _) {
  while (true) delay(1);
}
