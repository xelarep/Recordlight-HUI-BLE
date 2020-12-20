/*
 * RecordLightHUI
 * ==============
 * 
 * Mackie HUI compatibel Record Light for Logic
 * 
 * Expect Data on Channel 1 from Logic Mackie HUI Control
 * 
 * A. Peter, 15.12.2020
 */
#include <BLEMIDI_Transport.h>

//#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#include <hardware/BLEMIDI_ESP32.h>
//#include <hardware/BLEMIDI_nRF52.h>
//#include <hardware/BLEMIDI_ArduinoBLE.h>

BLEMIDI_CREATE_INSTANCE("RecordingLightHUI", MIDI)

bool isConnected = false;
#define LED_BLELINK 23            // BLE MIDI Active LED

#include <FastLED.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    22            // WS2812 Data Pin
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    12
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

// State Machine
enum eStatus { Idle, Connected, Active, Armed, Play, Record };
eStatus curState = Idle;
eStatus prevState = Idle;

// pending Update
bool updateState = false;
bool sendPing = false;     

// Filter for zone
byte HUIzone = 0;

byte transport = 0x00;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  MIDI.begin();

  pinMode(LED_BLELINK, OUTPUT);
  digitalWrite(LED_BLELINK, LOW);

  BLEMIDI.setHandleConnected(OnConnected);
  BLEMIDI.setHandleDisconnected(OnDisconnected);

  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
 
  delay(2000);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);  
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  MIDI.read();

  if (curState == Idle)
    rainbow();
  else
  {
   if(updateState == true)
   {
    switch(transport)
    {
      case 0x04 : curState = Active;
                  break;
      case 0x02 : curState = Play;
                  break;
      case 0x01 :
      case 0x03 : curState = Record;
                  break;
      default :   curState = Connected;
                  break;
    }
    switch(curState) {
      case Connected:
                    //SetCRGBColor(CRGB::Black);        // Off
                    SetRGBColor(0x04, 0x02, 0x00);
                    break;
      case Active:
                    SetRGBColor(0x00, 0x00, 0x05);    // Stop / Connected to Logic
                    break;
      case Armed:
                    SetCRGBColor(CRGB::Yellow);       // not supportted yet
                    break;
      case Play:                                      // Playback
                    SetCRGBColor(CRGB::Green);
                    break;
      case Record:                                    // Recording
                    SetCRGBColor(CRGB::Red);
                    break;

      case Idle:    
      default:    rainbow();
    }
    
    prevState = curState;
    updateState = false;
   }

   if(sendPing == true)
   {
     //MIDI.sendNoteOn (00, 127, 1); // note 60, velocity 127 on channel 1  
     sendPing = false;
   }
  }
    
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  if (curState == Idle)
    FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// Device connected
// -----------------------------------------------------------------------------
void OnConnected() {
  isConnected = true;
  curState = Connected;
  prevState = Connected;
  updateState = true;
  HUIzone = 0;
  sendPing = false;
  transport = 0x00;

  digitalWrite(LED_BLELINK, HIGH);
}

// -----------------------------------------------------------------------------
// Device disconnected
// -----------------------------------------------------------------------------
void OnDisconnected() {
  isConnected = false;
  curState = Idle;
  prevState = Idle;
  updateState = false;
  digitalWrite(LED_BLELINK, LOW);
}

// -----------------------------------------------------------------------------
// Received note on
// -----------------------------------------------------------------------------
void OnNoteOn(byte channel, byte note, byte velocity) {
  
}

// -----------------------------------------------------------------------------
// Received note off
// -----------------------------------------------------------------------------
void OnNoteOff(byte channel, byte note, byte velocity) {
  if(channel == 1)
  {
     if(note == 0)    // Ping
     {
        if (curState == Connected)
        {
          curState = Active;
          updateState = true;
        }
     }
  }  
}

// -----------------------------------------------------------------------------
// Continous Control
// -----------------------------------------------------------------------------
void OnControlChange(byte channel, byte number, byte value) {
   if(channel == 1)
   {
     if (number == 0x0c)    // get zone
       HUIzone = value;

     if(HUIzone == 0x0e && number == 0x2c)     // get zone ctrl and value
     {
        updateState = true;

        switch(value)                         // rebuild buttons in single value
        {
          case 0x45:        // record on
                            transport |= 0x01;
                            break;
          case 0x44:        // play on
                            transport |= 0x02;
                            break;
          case 0x43:        // stop on
                            transport |= 0x04;
                            break;
          case 0x05:        // record off
                            transport &= ~0x01;
                            break;
          case 0x04:        // play off
                            transport &= ~0x02;
                            break;
          case 0x03:        // stop off
                            transport &= ~0x04;
                            break;
          default:          
                            updateState = false;
                            break;                         
        }

     }
   }  
}

// -----------------------------------------------------------------------------
// Color management
void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void SetCRGBColor(CRGB col) {
  for( int i = 0; i < NUM_LEDS; i++) 
     leds[i] = col;      
}
void SetRGBColor(byte r, byte g, byte b) {
  for( int i = 0; i < NUM_LEDS; i++) 
     leds[i].setRGB( r, g, b);
}
// -----------------------------------------------------------------------------
