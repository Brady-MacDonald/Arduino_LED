#include <FastLED.h>
#include <Keypad.h>

//Arduino set up
#define NUM_LEDS 149
#define LED_PIN 2
#define ANALOG_READ 0

//MIC settings
#define MIC_LOW 400
#define MIC_HIGH 420

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

float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve);
void insert(int val, int *avgs, int len);
int compute_average(int *avgs, int len);
void visualize_music();

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

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);
  
  for (int i = 0; i < NUM_LEDS; i+=2){
    int r = i;
    int b = 256 - i;
    int g = 0;
    leds[i] = CRGB(r, g, b);       
    FastLED.show();
  }
  for (int i = NUM_LEDS; i > 0; i-=2){
    int r = 256 - i;
    int b = i;
    int g = 0;
    leds[i] = CRGB(r, g, b);       
    FastLED.show();
  }
    
  for (int i = 0; i < AVGLEN; i++) {
    insert(250, avgs, AVGLEN);
  }

  //Initial values
  high.times = 0;
  high.times_start = millis();
  Color.r = 0;
  Color.g = 1;
  Color.b = 0;
}

void FillLEDsFromPaletteColors(uint8_t);
void DisplaySolidColour();
void NewKITT(byte, byte, byte, int, int, int);
void RGBLoop();
float fscale(float, float, float, float, float, float);

void loop() {

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1;

  char padInput = keypad.getKey();

  //check for new input
  //needed as there null is read if no input 
  if(padInput){
    ledType = padInput;
    Serial.println(padInput);
  }
  
  switch (ledType) {
    case '1':
      visualize_music();
      break;
    case '2':
      //DisplaySolidColour();
      break;
    case '3': 
      break;
    case '4':
      SetPalette(startIndex, LavaColors_p);
      break;
    case '5':
      SetPalette(startIndex, OceanColors_p);
      break;
    case '6':
      SetPalette(startIndex, ForestColors_p);
      break;
    case '7':
        RGBLoop();
    case '8':
        PingPong(0, 0xff, 0, 10, 10, 50);
      break;
    case '9':
        DoublePong(0, 0xff, 0, 10, 10, 50);
      break;
    case '0':
      UserColour();
      break;
    case '*':
      SetAll(0, 200, 50);
      break;
    case '#':
      SetAll(125, 0, 25);
      break;
    default:
      break;
  }
  delay(DELAY);
}

void showStrip(){
  FastLED.show();
}

void SetPalette(uint8_t colorIndex, CRGBPalette16 palette)
{  
  uint8_t brightness = 255;
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
  FastLED.show();
}

void UserColour(){
  byte red=0, green=0, blue=0;
  int index = 0;
  int count = 0;
  char padInput;
  char userInput[3];
  
  while(index != 3){
    padInput = keypad.getKey();

    if(padInput){
      if(padInput == '*'){
        index++;
      }
      else{
        Serial.print("index: ");
        Serial.println(index);

        Serial.print("padInput: ");
        Serial.println(padInput);

        if(index == 0){
          red = padInput;
          Serial.print("red: ");
          Serial.println(red);
        }
        else if (index == 1){
          blue = padInput;
          Serial.print("blue: ");
          Serial.println(blue);
        }
        else if(index == 2){
          green = padInput;
          Serial.print("green: ");
          Serial.println(green);
        }  
      }      
    } 
  } 
  Serial.println("FINAL: ");
  Serial.println(red);
  Serial.println(green);
  Serial.println(blue);
  SetAll(green, red, blue);
  delay(5000);
}

void PingPong(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

void DoublePong(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  int r = 0;
  int g = 0;
  int b = 125;
  for(int i =((NUM_LEDS-EyeSize)/2); i>=0; i--) {
    SetAll(r, g, b);
   
    leds[i] = CRGB(red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      leds[i+j] = CRGB(red, green, blue);
    }
    leds[i+EyeSize+1] = CRGB(red/10, green/10, blue/10);
   
    leds[NUM_LEDS-i] = CRGB(red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      leds[NUM_LEDS-i-j] = CRGB(red, green, blue);
    }
    leds[NUM_LEDS-i-EyeSize-1] = CRGB(red/10, green/10, blue/10);
   
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  int randy = random(50,200);
  int r = 0;
  int g = 25;
  int b = 100;
  
  for(int i = 0; i<=((NUM_LEDS-EyeSize)/2); i++) {
    SetAll(r,g,b);
   
    leds[i] = CRGB(red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      leds[i+j] = CRGB(red, green, blue);
    }
    leds[i+EyeSize+1] = CRGB(red/10, green/10, blue/10);
   
    leds[NUM_LEDS-i] = CRGB(red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      leds[NUM_LEDS-i-j] = CRGB(red, green, blue);
    }
    leds[NUM_LEDS-i-EyeSize-1] = CRGB(red/10, green/10, blue/10);
   
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void LeftToRight(byte green, byte blue, byte red, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    if(hue>255){
      hue = 0;
    }
    hue++;
    SetAll(0,0,0);
    //leds[i] = CRGB(hue/10, hue/10, hue/10);
    leds[i] = ColorFromPalette( currentPalette, i, 150, currentBlending);
    for(int j = 1; j <= EyeSize; j++) {
      //leds[i+j] = CRGB(hue, hue, hue);
      leds[i+j] = ColorFromPalette( currentPalette, i+j, 255, currentBlending);
    }
    leds[i+EyeSize+1] = ColorFromPalette( currentPalette, i+EyeSize+1, 150, currentBlending);
    //leds[i+EyeSize+1] = CRGB(hue/10, hue/10, hue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void RightToLeft(byte blue, byte red, byte green, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    if(hue>255){
      hue = 0;
    }
    hue++;
    SetAll(0,0,0);
    //leds[i] = CRGB(hue/10, hue/10, hue/10);
    leds[i] = ColorFromPalette( currentPalette, i, 150, currentBlending);

    for(int j = 1; j <= EyeSize; j++) {
      leds[i+j] = ColorFromPalette( currentPalette, i+j, 255, currentBlending);
    }
    leds[i+EyeSize+1] = ColorFromPalette( currentPalette, i+EyeSize+1, 150, currentBlending);
    //leds[i+EyeSize+1] = CRGB(hue/10, hue/10, hue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void RGBLoop(){
  for(int j = 0; j < 3; j++ ) {
    // Fade IN
    for(int k = 0; k < 256; k++) {
      switch(j) {
        case 0: SetAll(k,0,0); break;
        case 1: SetAll(0,k,0); break;
        case 2: SetAll(0,0,k); break;
      }
      FastLED.show();
      delay(3);
    }
    // Fade OUT
    for(int k = 255; k >= 0; k--) {
      switch(j) {
        case 0: SetAll(k,0,0); break;
        case 1: SetAll(0,k,0); break;
        case 2: SetAll(0,0,k); break;
      }
      delay(3);
    }
  }
}

void SetAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB(red, green, blue);
  }
   FastLED.show();
}

int ReadSoundDifference() {
  int soundValue1 = analogRead(ANALOG_READ);
  delay(1);
  int soundValue2 = analogRead(ANALOG_READ);
  int diff = soundValue1 - soundValue2;
  //Serial.println(soundValue1);
  //Serial.println(soundValue2);
  return diff;
}

void ChangeColourState() {
  if(currentPalette==PartyColors_p){
    currentPalette=RainbowColors_p;
  }
  else if(currentPalette==CloudColors_p){
    currentPalette=PartyColors_p;
  }
  else if(currentPalette==RainbowColors_p){
    currentPalette=CloudColors_p;
  }
  /*if (ledColour == red) {
    ledColour = green;
  }
  else if (ledColour == green) {
    ledColour = blue;
  }
  else if (ledColour == blue) {
    ledColour = red;
  }*/
}

void DisplaySolidColour() {
  int sound = ReadSoundDifference();
  //Serial.println(sound);
  
  for(int i=0; i<NUM_LEDS; i++){
  if(sound > 100 || sound < -100){
        leds[i] = CHSV(abs(sound)/2, 0, i/10);
  }
  else if(sound > 50 || sound < -50){
        leds[i] = CHSV(0, i/5, abs(sound));
  }
  else if(sound > 30 || sound < -30){
        leds[i] = CHSV(0, abs(sound), i);
  }
  else if(sound > 25 || sound < -25){
    leds[i] = CHSV(i, 15, abs(sound));
  }
  else if(sound > 20 || sound < -20){
    leds[i] = CHSV(30, i, abs(sound));
  }
  else if(sound > 15 || sound < -15){
    leds[i] = CHSV(abs(sound), 10, i);
  }
  else if(sound > 10 || sound < -10){
    leds[i] = CHSV(20, i, abs(sound));
  }
  else if(sound > 5 || sound < -5){
        currentPalette = CloudColors_p;
    leds[i] = ColorFromPalette( currentPalette, i, 150, currentBlending);
  }
  else if(sound == 0){
    FastLED.clear();
    FastLED.show();
  }
  else{
    currentPalette = RainbowColors_p;
    leds[i] = ColorFromPalette( currentPalette, i, 150, currentBlending);
  }
  }
  /*
  if (sound > 2 || sound < -2) {
    for (int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
      leds[ledIndex] = ColorFromPalette( currentPalette, ledIndex, 255, currentBlending);
      
      if (ledColour == red) {
        leds[ledIndex] = CRGB(255, 0, 0);
      }
      else if (ledColour == green) {
        leds[ledIndex] = CRGB(0, 255, 0);
      }
      else if (ledColour == blue) {
        leds[ledIndex] = CRGB(0, 0, 255);
      }*/
    
    FastLED.show();
    delay(50);
    ChangeColourState();
}  

/**Funtion to check if the lamp should either enter a HIGH mode,
  or revert to NORMAL if already in HIGH. If the sensors report values
  that are higher than 1.1 times the average values, and this has happened
  more than 30 times the last few milliseconds, it will enter HIGH mode.
  TODO: Not very well written, remove hardcoded values, and make it more
  reusable and configurable.  */
void check_high(int avg) {
  if (avg > (song_avg / iter * 1.1))  {
    if (high.times != 0) {
      if (millis() - high.times_start > 200.0) {
        high.times = 0;
        songmode = NORMAL;
      } else {
        high.times_start = millis();
        high.times++;
      }
    } else {
      high.times++;
      high.times_start = millis();

    }
  }
  if (high.times > 30 && millis() - high.times_start < 50.0)
    songmode = HIGH_CONST;
  else if (millis() - high.times_start > 200) {
    high.times = 0;
    songmode = NORMAL;
  }
}

//Main function for visualizing the sounds in the lamp
void visualize_music() {
  int sensor_value, mapped, avg, longavg;

  //Actual sensor value
  sensor_value = analogRead(ANALOG_READ);
  //Serial.println(sensor_value);
  //If 0, discard immediately. Probably not right and save CPU.
  if (sensor_value == 0)
    return;

  //Discard readings that deviates too much from the past avg.
  mapped = (float)fscale(MIC_LOW, MIC_HIGH, MIC_LOW, (float)MIC_HIGH, (float)sensor_value, 2.0);
  avg = compute_average(avgs, AVGLEN);
  if (((avg - mapped) > avg * DEV_THRESH))
    return;

  //Insert new avg. values
  insert(mapped, avgs, AVGLEN);
  insert(avg, long_avg, LONG_SECTOR);

  //Compute the "average" sensor value
  song_avg += avg;
  iter++;
  if (iter > CYCLES) {
    song_avg = song_avg / iter;
    iter = 1;
  }

  longavg = compute_average(long_avg, LONG_SECTOR);

  //Check if we enter HIGH mode
  check_high(longavg);

  if (songmode == HIGH_CONST) {
    fade_scale = 3;
    Color.r =  8;
    Color.g = 1;
    Color.b = -2;
  }
  else if (songmode == NORMAL) {
    fade_scale = 3;
    Color.r = -1;
    Color.b = 6;
    Color.g = -2;
  }

  //Decides how many of the LEDs will be lit
  curshow = fscale(MIC_LOW, MIC_HIGH, 0.0, (float)NUM_LEDS, float(avg), -1);

  /*if(avg == 524){
    curshow = NUM_LEDS/2;
    }
    else if(avg == 525 || avg == 523){
    curshow = NUM_LEDS/2 + 20;
    }
    else if(avg == 526 || avg == 522){
    curshow = NUM_LEDS/2 + 30;
    }
    else if(avg == 527 || avg == 521){
    curshow = NUM_LEDS/2 + 40;
    }
    else if(avg == 528 || avg == 520){
    curshow = NUM_LEDS/2 + 50;
    }
    else if(avg == 529 || avg == 519){
    curshow = NUM_LEDS;
    }*/

  /*Set the different leds. Control for too high and too low values.
    Fun thing to try: Dont account for overflow in one direction,
    some interesting light effects appear! */
  hue++;
  if (hue > 255) {
    hue = 0;
  }

  for (int i = 0; i < NUM_LEDS; i++)
    //The leds we want to show
    if (i < NUM_LEDS / 2 + curshow / 2 + 20 && i > NUM_LEDS / 2 - curshow / 2 - 20) {
      leds[i] = CHSV(hue, 255, 255);
      //All the other LEDs begin their fading journey to eventual total darkness
    } else {
      leds[i] = CRGB(leds[i].r / fade_scale, leds[i].g / fade_scale, leds[i].b / fade_scale);
    }
  FastLED.show();
}
//Compute average of a int array, given the starting pointer and the length
int compute_average(int *avgs, int len) {
  int sum = 0;
  for (int i = 0; i < len; i++)
    sum += avgs[i];

  return (int)(sum / len);
}

//Insert a value into an array, and shift it down removing
//the first value if array already full
void insert(int val, int *avgs, int len) {
  for (int i = 0; i < len; i++) {
    if (avgs[i] == -1) {
      avgs[i] = val;
      return;
    }
  }

  for (int i = 1; i < len; i++) {
    avgs[i - 1] = avgs[i];
  }
  avgs[len - 1] = val;
}

//Function imported from the arduino website.
//Basically map, but with a curve on the scale (can be non-uniform).
float fscale( float originalMin, float originalMax, float newBegin, float
              newEnd, float inputValue, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin) {
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;
  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
