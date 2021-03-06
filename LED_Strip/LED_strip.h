#ifndef LED_HEADER
#define LED_HEADER

#include <FastLED.h>
#include <Keypad.h>

//Arduino set up
#define NUM_LEDS 300
#define LED_PIN 2
#define ANALOG_READ 0

//MIC settings
#define MIC_LOW 405
#define MIC_HIGH 435

//Arduino loop delay
#define DELAY 1

//How many previous sensor values effects the operating average?
#define AVGLEN 15
//How many previous sensor values decides if we are on a peak/HIGH (e.g. in a song)
#define LONG_SECTOR 5

//Mneumonics
#define HIGH_CONST  3
#define NORMAL 2

//How long do we keep the "current average" sound, before restarting the measuring
#define MSECS 2000
#define CYCLES MSECS / DELAY

/*Sometimes readings are wrong or strange*/
#define DEV_THRESH 0.8

//How many LEDs to we display
int curshow = NUM_LEDS;

//Showing different colors based on the mode.
int songmode = NORMAL;

//Average sound measurement the last CYCLES
unsigned long song_avg;

//The amount of iterations since the song_avg was reset
int iter = 0;

//The speed the LEDs fade to black if not relit
float fade_scale = 1.2;

//Led array
CRGB leds[NUM_LEDS];

/*Short sound avg used to "normalize" the input values.
  We use the short average instead of using the sensor input directly */
int avgs[AVGLEN] = { -1};

//Longer sound avg
int long_avg[LONG_SECTOR] = { -1};

//Keeping track how often, and how long times we hit a certain mode
struct time_keeping {
  unsigned long times_start;
  short times;
};

//How much to increment or decrement each color every cycle
struct color {
  int r;
  int g;
  int b;
};

struct time_keeping high;
struct color Color;

enum ColourState {
  red,
  green,
  blue
};
ColourState ledColour = blue;

//set default to 1 on boot
static char ledType = '1';

static int width = 1;
static int reverse = 0;

//Keypad setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {12, 11, 10, 9};
byte colPins[COLS] = {6, 5, 4};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

static int hue = 0;
int incomingByte = 0;

//Function definitions
void SetPalette(uint8_t, CRGBPalette16);
void UserColour();
void PingPong(byte, byte, byte, int, int, int);
void DoublePong(byte, byte, byte, int, int, int);

float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve);
void insert(int val, int *avgs, int len);
int compute_average(int *avgs, int len);
void visualize_music();
void FillLEDsFromPaletteColors(uint8_t);
void DisplaySolidColour();
void NewKITT(byte, byte, byte, int, int, int);
void RGBLoop();
float fscale(float, float, float, float, float, float);

#endif
