# TM1638 Prop Bomb

Electronic bomb simulator designed for Airsoft matches (Mounrive Studio).

Make sure to copy the files tm1638.c and tm1638.h to the same folder as main.c

======

## 1. Overview
The Prop Bomb is an electronic bomb simulator designed for Airsoft matches based on TM1638 module. It is a Mounriver Studio project. 
It consists of a programmable count down timer with optional password protection. It provides a dry contact ouptput that is activated when the time runs out. 

## 2. Setup & Configuration (Programming Mode)

To set or change the game timer and defusal password, press and hold the Configuration Button while connecting/turning on the battery, otherwise the prop will use the last time and password set.

### Setting the Time (TIME):

1. Use the buttons S5 to S8 to set the set the minutes and seconds (MM.SS). The minutes can be up to 99, which means that the maximum play time is 1 hour and 40 minutes.
1. Press Button S1 to confirm and proceed to the password setup.

### Setting the Password (PASS):

1. Use Buttons S5 to S8 to select a 4-digit defusal code.
2. Press Button S1 to confirm. 

The system automatically saves these settings to internal memory and enters standby mode.

Note: if no password is entered, the default code is 0000.

### Starting the game 

When the circuit is powerer on with the configuration switch unactivated, or right after setting the time and password, the display will show the ready message and will blink the Status LED. 

To start the game, press Button S4. All LEDs will flash to signal that the game has begun, then the display will continuously shows the remaining time (MM:SS).

### In-Game Controls (Countdown & Defusal)



Attempting a Defusal:

1. Use Buttons S5 to S8 to enter the 4-digit code. 
2. Press Button S1 to submit the code.

Note: Button S2 can be used to reset the current entry.


Defusal Results:

- Incorrect Password (Penalty): Entering a wrong code immediately cuts the remaining time in half, penalizing failed attempts.
- Correct Password: The display will show "good" then the individual LEDs will glow with a victory animation. 

Note: if no passwort is set, pressing button S1 will disarm the bomb, which equivake as if there is no password set.

## Game Over (Detonation)

If the timer reaches 00:00 before a successful defusal, the individual LEDs will glow with a detonation animation and the dry contact will be activated.

Note: To deactivate the output, power off the circuit.


