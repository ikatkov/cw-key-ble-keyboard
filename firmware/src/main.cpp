#include <Arduino.h>
#include <BleKeyboard.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BleKeyboard bleKeyboard;
// #define PADDLE_DIT_PIN 4
// #define PADDLE_DAH_PIN 5
// #define POWER_LATCH_PIN 18
// #define BUTTON_SENSE_PIN 23
// #define VOLTAGE_DETECTOR_PIN 36
// #define BUZZER_PIN 22
// #define LED_PIN 19

#define PADDLE_DIT_PIN 33
#define PADDLE_DAH_PIN 32

#define VOLTAGE_DETECTOR_PIN 36
#define BUZZER_PIN 17
#define BUTTON_SENSE_PIN 19
#define POWER_LATCH_PIN 18

byte button_dit_oldSwitchState = HIGH;
byte button_dah_oldSwitchState = HIGH;
byte button_pwr_oldSwitchState = HIGH;
const unsigned long debounceTime = 10; // milliseconds
unsigned long button_dit_press_time;
unsigned long button_dah_press_time;
unsigned long button_pwr_press_time;

unsigned long lastLowVoltageCheck;
bool turnoff_countdown;

// Callback function to be called when button1 is pressed
void onDitPressed()
{
  Serial.println("onDitPressed");
  if (bleKeyboard.isConnected())
  {
    bleKeyboard.print("]");
  }
}

// Callback function to be called when button2 is pressed
void onDahPressed()
{
  Serial.println("onDahPressed");
  if (bleKeyboard.isConnected())
  {
    bleKeyboard.print("[");
  }
}

void startup_tone()
{
  tone(BUZZER_PIN, 1000, 200);
  delay(200);
  tone(BUZZER_PIN, 1500, 600);
  delay(600);
}

void shutdown_tone()
{
  tone(BUZZER_PIN, 1500, 200);
  delay(200);
  tone(BUZZER_PIN, 1000, 600);
  delay(600);
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(PADDLE_DIT_PIN, INPUT_PULLUP);
  pinMode(PADDLE_DAH_PIN, INPUT_PULLUP);
  pinMode(VOLTAGE_DETECTOR_PIN, INPUT);
  pinMode(BUTTON_SENSE_PIN, INPUT_PULLDOWN);
  pinMode(POWER_LATCH_PIN, OUTPUT);
  // pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(POWER_LATCH_PIN, HIGH);

  // digitalWrite(LED_PIN, HIGH);
  // delay(1000);
  // digitalWrite(LED_PIN, LOW);

  analogSetAttenuation(ADC_2_5db);
  analogSetWidth(12);

  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  Serial.println("Starting Morse Code Keyer BLE");
  bleKeyboard.setName("Morse Code Keyer");
  bleKeyboard.begin();

  // digitalWrite(LED_PIN, HIGH);
  startup_tone();
  // digitalWrite(LED_PIN, LOW);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1);
}

uint8_t mapBatteryValue(uint16_t input) {
    // Ensure the input value is clamped within the source range
    if (input < 800) {
        input = 800;
    } else if (input > 1100) {
        input = 1100;
    }

    // Perform the mapping using linear interpolation
    return (uint8_t)((input - 800) * (100 - 10) / (1100 - 800) + 10);
}
void loop()
{
  if (millis() - 5000 > lastLowVoltageCheck)
  {
    // check VOLTAGE_DETECTOR_PIN
    u_int16_t voltage = analogReadMilliVolts(VOLTAGE_DETECTOR_PIN);    
    lastLowVoltageCheck = millis();
    bleKeyboard.setBatteryLevel(mapBatteryValue(voltage));
    if (voltage < 800)
    {
      Serial.print("Low voltage detected");
      Serial.println(voltage);
      tone(BUZZER_PIN, 1000, 100);
      delay(100);
      tone(BUZZER_PIN, 1000, 100);
      delay(100);
    }
  }

  byte button_dit_switchState = digitalRead(PADDLE_DIT_PIN);
  // has it changed since last time?
  if (button_dit_switchState != button_dit_oldSwitchState)
  {
    // debounce
    if (millis() - button_dit_press_time >= debounceTime)
    {
      turnoff_countdown = false;
      button_dit_press_time = millis();                   // when we closed the switch
      button_dit_oldSwitchState = button_dit_switchState; // remember for next time
      if (button_dit_switchState == LOW)
      {
        Serial.println("DIT down");
        if (bleKeyboard.isConnected())
        {
          bleKeyboard.press(0x5D); //]
        }
      }
      else
      {
        Serial.println("DIT up");
        bleKeyboard.release(0x5D);
      }
    }
  }

  byte button_dah_switchState = digitalRead(PADDLE_DAH_PIN);
  // has it changed since last time?
  if (button_dah_switchState != button_dah_oldSwitchState)
  {
    // debounce
    if (millis() - button_dah_press_time >= debounceTime)
    {
      turnoff_countdown = false;
      button_dah_press_time = millis();                   // when we closed the switch
      button_dah_oldSwitchState = button_dah_switchState; // remember for next time
      if (button_dah_switchState == LOW)
      {
        Serial.println("DAH down");
        if (bleKeyboard.isConnected())
        {
          bleKeyboard.press(0x5B); //[
        }
      }
      else
      {
        Serial.println("DAH up");
        bleKeyboard.release(0x5B);
      }
    }
  }

  if (digitalRead(BUTTON_SENSE_PIN) == HIGH)
  {
    Serial.println("BUTTON_SENSE_PIN HIGH");
    delay(3000);                               // 3 second delay after button pressed
    if (digitalRead(BUTTON_SENSE_PIN) == HIGH) // check if button still pressed and wait till the button is released
    {
      Serial.println("Shutdown manual");
      shutdown_tone();
      // shut down
      digitalWrite(POWER_LATCH_PIN, LOW);
      delay(1000);
    }
  }

  if (!turnoff_countdown && (millis() - max(button_dit_press_time, button_dit_press_time)) > 300000)
  {
    Serial.println("Shutdown count down");
    tone(BUZZER_PIN, 2000, 300);
    delay(500);
    tone(BUZZER_PIN, 2000, 300);
    delay(500);
    tone(BUZZER_PIN, 2000, 300);
    delay(500);
    turnoff_countdown = true;
  }

  if (turnoff_countdown && (millis() - max(button_dit_press_time, button_dit_press_time)) > 320000)
  {
    Serial.println("Shutting down after inactivity");
    shutdown_tone();
    // shut down
    digitalWrite(POWER_LATCH_PIN, LOW);
    delay(1000);
  }
  delay(10);
}
