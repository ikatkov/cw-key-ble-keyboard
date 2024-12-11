#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
}

void loop() {
  if(bleKeyboard.isConnected()) {
    bleKeyboard.print("s");
    delay(1000);
    bleKeyboard.print("o");
    delay(1000);
  }

  Serial.println("Waiting 5 seconds...");
  delay(5000);
}
