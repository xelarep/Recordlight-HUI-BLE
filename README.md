# RecordingLightHUI

A simple Recording Light for Apple Logic Pro, using the Mackie HUI protocol. Runs on a ESP32 DevKit with the Arduino BLE MIDI library. 
Lights used are 12 LEDs on a WS2812 based LED strip. Controlled by the FastLED library. Additional single LED as connector inidcator also supported.

## Light inidicators:

Idle/not connected via BT: 
- rainbow effect 

Bluetooth connection established:
- yellow

Stop Mode in Logic (once the first Ping from Mackie HUI/Logic is received)
- dark blue

Play:
- green

Record:
- red

__Currently only working, if PocketMIDI is used to connect the IAC driver output from Logic as input port and 'thru' to the RecordingLightHUI output port...__

