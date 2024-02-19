#include <arduino.h>
#include <BleKeyboard.h>

// Connect resistor from GPIO to 3.3v and switch to GND (Default is HIGH)
// Digital Inputs Input Pullup GPIO 16
// Connect resistor from GPIO to GND and switch to 3.3v (Default is LOW)
// Don't use pins 6 - 11 as may prevent booting
// GPIO 34 - 39 don't work with INPUT_PULLUP (Also Input only)
// GPIO 0 - 3 don't work with INPUT_PULLDOWN
// Best Digital Inputs (INPUT_PULLUP or INPUT_PULLDOWN) 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39
// Viable Digital Outputs GPIO 2, 4, 12, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
// Analogue Inputs (12 bit 0 - 4096) 32, 33, 34, 35, 36, 39
// Analogue Outputs PWM All Input Pins except 34 - 39
// Analogue Outputs DAC 25, 26

// Push Buttons Shift Register       
// Joystick J1Button = 14                                            Needs INPUT_PULLUP
// Joystick J1X = 35, J1Y = 36 (Analogue Input)
// 6_Way Switch = 32 (Analogue Input)
// Shift Register 1 SR1ClockPin = 21, SR1LatchPin = 19, SR1DataPin = 18
// Shift Register 2 SR2ClockPin = 17, SR2LatchPin = 16, SR2DataPin = 4


// 2 Analogue Ports
// 15 Digital Ports

// Debounce Constants
const long int debounceDelay = 50;
const int joystickSensitivity = 150;
const int sw6Sensitivity = 300;

// Master Shift Register Constants
const int delaySR = 20;

// Shift Register 1 Constants
const int SR1ClockPin = 17;
const int SR1LatchPin = 16;
const int SR1DataPin = 4;
const int numSR1 = 4;

// Shift Register 1 Variables
byte SRSwitchVar1[numSR1] = {72, 33, 55, 101}; //01001000 (non-zero to help debugging)
bool prevousSRSwitchState1[numSR1 * 8] 
      = {
        LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        , LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        , LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        , LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        };
long int lastSRSDebounceTime1[numSR1 * 8] 
      = {
        0, 0, 0, 0, 0, 0, 0, 0
        , 0, 0, 0, 0, 0, 0, 0, 0
        , 0, 0, 0, 0, 0, 0, 0, 0
        , 0, 0, 0, 0, 0, 0, 0, 0
        };

u_int8_t SRSwitchesKeyMap1[numSR1 * 8] 
      = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
        , 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'
        , 'q', 'r', 's', 't', 'u', 'v', 'w', 'x'
        , '0', '1', '2', '3', '4', '5', '6', '7'
        };

// Shift Register 2 Constants
const int SR2ClockPin = 21;
const int SR2LatchPin = 19;
const int SR2DataPin = 18;
const int numSR2 = 4;

// Shift Register2 Variables
byte SRSwitchVar2[numSR2] = {2, 33, 55, 101}; //01001000 (non-zero to help debugging)
bool prevousSRSwitchState2[numSR2 * 8] 
      = {
        LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        , LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        , LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        , LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
        };
long int lastSRSDebounceTime2[numSR2 * 8] 
      = {
        0, 0, 0, 0, 0, 0, 0, 0
        , 0, 0, 0, 0, 0, 0, 0, 0  
        , 0, 0, 0, 0, 0, 0, 0, 0
        , 0, 0, 0, 0, 0, 0, 0, 0
        };

u_int8_t SRSwitchesKeyMap2[numSR2 * 8] 
      = {
        '1', '2', '3', '4', '5', '6', '7', '*'
        , 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
        , 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'
        , 'q', 'r', 's', 't', 'u', 'v', 'w', 'x'
        };

// 6Way Switch
// 6Way Switch Constants
const int SW6Pin = 32;

// 6Way Switch Variables
int currentSW6State = 0;
int currentSW6Value = 0;
int previousSW6Value = 0;

// Mouse_Joystick 
// Mouse_Joystick Constants
const int joyButtonPin = 14;
//  J1ButtonPin = 32
int joyXYPin[2] = {35, 34};
//  J1XPin = 35, J1YPin = 34
const int joyLowerLimit = 900;
const int joyUpperLimit = 2700;

bool prevousJoyButtonState = LOW;
long int lastJoyButtonDebounceTime = 0;
int currentJoyXY[2] = {0, 0};
int previousJoyXY[2] = {0, 0};
int previousJoyState = 0;
/*
*/

BleKeyboard bleKeyboard;

void SetNewLayout();

void setup() {

  Serial.begin(115200);
  bleKeyboard.setName("Train Controller");

  bleKeyboard.begin();

  // Init Shift Register

  pinMode(SR1ClockPin, OUTPUT);
  pinMode(SR1LatchPin, OUTPUT);
  pinMode(SR1DataPin, INPUT);
/*

*/
  pinMode(SR2ClockPin, OUTPUT);
  pinMode(SR2LatchPin, OUTPUT);
  pinMode(SR2DataPin, INPUT);


  // Init 6Way Switch
  pinMode(SW6Pin, INPUT);

  // Init Joystick

  for (int i = 0; i < 2; ++i)
  {
    pinMode(joyXYPin[i], INPUT);
  }

  pinMode(joyButtonPin, INPUT_PULLUP);

  bleKeyboard.releaseAll();
}

void loop() {
  
  long int now = millis();
  if(bleKeyboard.isConnected()) 
  {

    // Read Shift Registers

    digitalWrite(SR1LatchPin, 1);
    //digitalWrite(SR2LatchPin, 1);
    delayMicroseconds(delaySR);
    digitalWrite(SR1LatchPin, 0);
    //digitalWrite(SR2LatchPin, 0);

    for (int n = 0; n < numSR1; n++)
    { 
      SRSwitchVar1[n] = shiftIn(SR1DataPin, SR1ClockPin, MSBFIRST);   
    }
/*

*/
    digitalWrite(SR2LatchPin, 1);
    delayMicroseconds(delaySR);
    digitalWrite(SR2LatchPin, 0);

    for (int n = 0; n < numSR2; n++)
    { 
        SRSwitchVar2[n] = shiftIn(SR2DataPin, SR2ClockPin, MSBFIRST);   
    }


    // 6 Way Switch
    currentSW6Value = analogRead(SW6Pin);
    if ((abs(currentSW6Value - previousSW6Value) > sw6Sensitivity) && (currentSW6Value > 340))
    {
      // calc new state currentSW6Value
      if (currentSW6Value < 1023) 
        currentSW6State = 0;
      else if (currentSW6Value < 1706)
        currentSW6State = 1;
      else if (currentSW6Value < 2389)
        currentSW6State = 2;
      else if (currentSW6Value < 2572)
        currentSW6State = 3;
      else if (currentSW6Value < 3755)
        currentSW6State = 4;
      else
        currentSW6State = 5;
      SetNewLayout();
      // set previous value
      previousSW6Value = currentSW6Value;
    }

    // SR Switches


    for (int nSR = 0; nSR < numSR1; nSR++)
    {
      for (int n = 0; n < 8; n++)
      {
        if ((now - lastSRSDebounceTime1[n + nSR * 8]) > debounceDelay)
        {
          bool buttonState = SRSwitchVar1[nSR] & (1 << n);

          if (buttonState != prevousSRSwitchState1[n + nSR * 8] && (buttonState == HIGH))
          {
            bleKeyboard.press(SRSwitchesKeyMap1[n + nSR * 8]);
            lastSRSDebounceTime1[n + nSR * 8] = now;
          }
          else if (buttonState != prevousSRSwitchState1[n + nSR * 8] && (buttonState == LOW))
          {
            bleKeyboard.release(SRSwitchesKeyMap1[n + nSR * 8]);
            lastSRSDebounceTime1[n + nSR * 8] = now;
          }
          prevousSRSwitchState1[n + nSR * 8] = buttonState;
        }
      }  
    }
/*

*/
    for (int nSR = 0; nSR < numSR2; nSR++)
    {
      for (int n = 0; n < 8; n++)
      {
        if ((now - lastSRSDebounceTime2[n + nSR * 8]) > debounceDelay)
        {
          bool buttonState = SRSwitchVar2[nSR] & (1 << n);

          if (buttonState != prevousSRSwitchState2[n + nSR * 8] && (buttonState == HIGH))
          {
            bleKeyboard.press(SRSwitchesKeyMap2[n + nSR * 8]);
            lastSRSDebounceTime2[n + nSR * 8] = now;
          }
          else if (buttonState != prevousSRSwitchState2[n + nSR * 8] && (buttonState == LOW))
          {
            bleKeyboard.release(SRSwitchesKeyMap2[n + nSR * 8]);
            lastSRSDebounceTime2[n + nSR * 8] = now;
          }
          prevousSRSwitchState2[n + nSR * 8] = buttonState;
        }
      }  
    }

    // Joystick
    // Button
  
    if ((now - lastJoyButtonDebounceTime) > debounceDelay)
    {
      // read the pushbutton:
      int buttonState = digitalRead(joyButtonPin);
      if ((buttonState != previousJoyState)
        && (buttonState == HIGH))
      {
        Serial.println("Joystick Button pressed");
        lastJoyButtonDebounceTime = now;
      } else if ((buttonState != previousJoyState)
        && (buttonState == LOW))
        {
          Serial.println("Joystick Button released");
          lastJoyButtonDebounceTime = now;
        }
      // save the current button state for comparison next time:
      previousJoyState = buttonState;

    }
    
    // Joy XY

    for (int i = 0; i < 2; i++)
    {
      currentJoyXY[i] = analogRead(joyXYPin[i]);
      if (abs(currentJoyXY[i] - previousJoyXY[i]) > joystickSensitivity)
        {
          //Serial.println("Joy pos " + String(i) + " = " + String(currentJoyXY[i]));
          if (i == 0)
          {
            if (currentJoyXY[i] < joyLowerLimit)
            {
              bleKeyboard.press(KEY_RIGHT_ARROW);
            } else if (currentJoyXY[i] > joyUpperLimit)
            {
              bleKeyboard.press(KEY_LEFT_ARROW);
            } else 
            {
              bleKeyboard.release(KEY_LEFT_ARROW);
              bleKeyboard.release(KEY_RIGHT_ARROW);
            }
          } else
          {
            if (currentJoyXY[i] < joyLowerLimit)
            {
              bleKeyboard.press(KEY_UP_ARROW);
            } else if (currentJoyXY[i] > joyUpperLimit)
            {
              bleKeyboard.press(KEY_DOWN_ARROW);
            } else
            {
              bleKeyboard.release(KEY_UP_ARROW);
              bleKeyboard.release(KEY_DOWN_ARROW);
            }
          }
          previousJoyXY[i] = currentJoyXY[i];
        }
    }   
  }
}

void SetNewLayout(){
  Serial.println("Current Mode = " + String(currentSW6State));
  delay(10);
  Serial.println("Current SW6 Value = " + String(currentSW6Value));
  bleKeyboard.releaseAll();
}