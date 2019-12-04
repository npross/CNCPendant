# CNCPendant


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
  * 2, 3 wired to encoder A, B
  * 8, 9, 10, 11 wired to AXIS X, Y, Z, 4
  * 5, 6, 7 wired to X1, X10, X100
  * 12 wired to enable button
  * 13 wired to LED
  * Encoder also has 5V and GND
  * LED is wired to 5V
