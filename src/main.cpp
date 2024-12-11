#include <Arduino.h>
#include <BleKeyboard.h>
#include "OneButton.h"

BleKeyboard bleKeyboard;
#define PADDLE_DIT_PIN 4
#define PADDLE_DAH_PIN 5
OneButton button_dit(PADDLE_DIT_PIN, true);
OneButton button_dah(PADDLE_DAH_PIN, true);

byte button_dit_oldSwitchState = HIGH;
byte button_dah_oldSwitchState = HIGH;
const unsigned long debounceTime = 10; // milliseconds
unsigned long button_ditPressTime;
unsigned long button_dahPressTime;

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

void setup()
{
  pinMode(PADDLE_DIT_PIN, INPUT_PULLUP);
  pinMode(PADDLE_DAH_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  Serial.println("Starting Morse Code Keyer BLE");
  bleKeyboard.setName("Morse Code Keyer");
  bleKeyboard.begin();
}

void loop()
{
  byte button_dit_switchState = digitalRead(PADDLE_DIT_PIN);
  // has it changed since last time?
  if (button_dit_switchState != button_dit_oldSwitchState)
  {
    // debounce
    if (millis() - button_ditPressTime >= debounceTime)
    {
      button_ditPressTime = millis();   // when we closed the switch
      button_dit_oldSwitchState = button_dit_switchState; // remember for next time
      if (button_dit_switchState == LOW)
      {
        Serial.println("DIT down");
        if(bleKeyboard.isConnected()){
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
    if (millis() - button_dahPressTime >= debounceTime)
    {
      button_dahPressTime = millis();   // when we closed the switch
      button_dah_oldSwitchState = button_dah_switchState; // remember for next time
      if (button_dah_switchState == LOW)
      {
        Serial.println("DAH down");
        if(bleKeyboard.isConnected()){
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

}
