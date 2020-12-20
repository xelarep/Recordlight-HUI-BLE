# RecordingLightHUI

A simple Recording Light for Apple Logic Pro, using the Mackie HUI protocol. Runs on a ESP32 DevKit with the Arduino BLE MIDI library. 

Lights are 12 LEDs on a WS2812 based LED strip.

Idle/not connected via BT: 
- Rainbow

Connected:
- Yellow

Stop Mode (once the first Ping from Mackie HUI/Logic is received)
- dark blue

Play:
- green

Record:
- red


