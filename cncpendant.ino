/*
  CNCPendant

  This sketch converts buttons, switches and encoder outputs from a handwheel
  to keystrokes.

  For Leonardo boards only.

  The side "deadman switch" enables and disables the sending of keystrokes.  
  When the side button is pressed (enablebutton input 12 pulled low):
     * the led lights up,  
     * the keyboard is enabled
     * the current jog speed (1, 2, 3) is sent 
  When the side button is released
     * keyboard is disabled
     * led turned off
     * current encoder value is sent to serial port
     
  The X1, X10, X100, axis switch immediately sends (1, 2, 3) to set jog speed

  Circuit:
  - 2, 3 wired to encoder A, B
  - 8, 9, 10, 11 wired to AXIS X, Y, Z, 4
  - 5, 6, 7 wired to X1, X10, X100
  - 12 wired to enable button
  - 13 wired to LED
  - Encoder also has 5V and GND
  - LED is wired to 5V

  created Nov 5 2019
  by Norm Ross
*/

#include <limits.h>
#include "Keyboard.h"
#include <util/atomic.h>

volatile long counter = 0;
long prevCounter = 0;

constexpr int outputA = 2;
constexpr int outputB = 3;
constexpr int speedX1 = 5;
constexpr int speedX10 = 6;
constexpr int speedX100 = 7;
constexpr int axisX = 8;
constexpr int axisY = 9;
constexpr int axisZ = 10;
constexpr int axis4 = 11;
constexpr int enableButton = 12;
constexpr int led = 13;

bool pinOutput[14] = {};
unsigned char  pinIntegrator[14] = {};
constexpr unsigned int debounceTime = 1;
constexpr unsigned int sampleInterval = 20;
constexpr unsigned int pinIntegratorMax = sampleInterval/debounceTime;
unsigned long lastDebounceTime = 0;

// Interrupt routine for quadrature encoder
// https://howtomechatronics.com/tutorials/arduino/rotary-encoder-works-use-arduino/
// 
void handwheel()
{
  if (pinOutput[enableButton] == LOW) {
      if (digitalRead(outputA) != digitalRead(outputB)) {
        counter++;
      } else {
        counter--;
      }
  }
}

void setup() {
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(speedX1, INPUT_PULLUP);
  pinMode(speedX10, INPUT_PULLUP);
  pinMode(speedX100, INPUT_PULLUP);
  pinMode(axisX, INPUT_PULLUP);
  pinMode(axisY, INPUT_PULLUP);
  pinMode(axisZ, INPUT_PULLUP);
  pinMode(axis4, INPUT_PULLUP);
  pinMode(enableButton, INPUT_PULLUP);
  pinMode(led, OUTPUT);

  digitalWrite(led, LOW);

  attachInterrupt(digitalPinToInterrupt(outputA), handwheel, CHANGE);
}

void sendAxis() {
    if (!pinOutput[enableButton]) {
      if (!pinOutput[speedX1]) {
        Keyboard.write('1');      
      } else if (!pinOutput[speedX10]) {
        Keyboard.write('2');            
      } else if (!pinOutput[speedX100]) {
        Keyboard.write('3');                  
      }
    }
}

void loop() {

  if ((millis() - lastDebounceTime) > debounceTime) {
    lastDebounceTime = millis();  
  
    for (int i = 5; i <= 12; i++) {
      if ((digitalRead(i) == LOW) && (pinIntegrator[i] > 0)) {
        pinIntegrator[i]--;
        if (pinIntegrator[i] == 0 && pinOutput[i]) {
            pinOutput[i] = false;
    //      Serial.print(i);
   //       Serial.println("-");

            switch(i) {
              case axisX:
              case axisY:
              case axisZ:
              case axis4:
                counter = 0;
                prevCounter = 0;
                break;              
              case enableButton:
                digitalWrite(led, HIGH);
                Keyboard.begin();
                counter = 0;
                prevCounter = 0;
                // fall through
              case speedX1:
              case speedX10:
              case speedX100:
                sendAxis();  
                break;
            }
        }
      } else if (pinIntegrator[i] < pinIntegratorMax) { 
        pinIntegrator[i]++;
        if ((pinIntegrator[i] == pinIntegratorMax) && !pinOutput[i]) {
          pinOutput[i] = true;
          switch(i) {
            case enableButton:
              digitalWrite(led, LOW);
              Keyboard.end();
              Serial.println(counter);
              break;
          }
    //      Serial.print(i);
    //      Serial.println("+");
        }
      }
    }
  }
  

  if (pinOutput[enableButton] == LOW) {
 
    int diffCounter;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      diffCounter = (counter - prevCounter)/2;
      prevCounter += diffCounter*2;
    }
  
    for (;diffCounter > 0; diffCounter--) {  
        if (pinOutput[axisX] == LOW) {
          Keyboard.press(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_RIGHT_ARROW);
        } else if (pinOutput[axisY] == LOW) {
          Keyboard.press(KEY_UP_ARROW);
          Keyboard.release(KEY_UP_ARROW);          
        } else if (pinOutput[axisZ] == LOW) {
          Keyboard.write('.');
        }    
    }
    for (;diffCounter < 0; diffCounter++) {  
        if (pinOutput[axisX] == LOW) {
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_ARROW);
        } else if (pinOutput[axisY] == LOW) {
          Keyboard.press(KEY_DOWN_ARROW);
          Keyboard.release(KEY_DOWN_ARROW);          
        } else if (pinOutput[axisZ] == LOW) {
          Keyboard.write(',');
        }
    }
  }
}
