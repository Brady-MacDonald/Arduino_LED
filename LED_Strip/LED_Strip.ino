#include "LED_Strip.h"

int state = 0; 
int r=252;
int g=0;
int b=0;

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);

  int colorVal=0;
    for(int j = 0; j < NUM_LEDS; j++){
      int r = j;
      int b = 255 - j;
      int g = 0;
      leds[j] = CRGB(r, g, b);       
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

  /*
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(9), ReadInput,CHANGE);  //interrupt setup keypad row 1
  attachInterrupt(digitalPinToInterrupt(10),ReadInput,CHANGE);  //interrupt setup keypad row 2
  attachInterrupt(digitalPinToInterrupt(11),ReadInput,CHANGE);  //interrupt setup keypad row 3
  attachInterrupt(digitalPinToInterrupt(12),ReadInput,CHANGE);  //interrupt setup keypad row 4
  attachInterrupt(digitalPinToInterrupt(4), ReadInput,CHANGE);  //interrupt setup keypad row 1
  attachInterrupt(digitalPinToInterrupt(5),ReadInput,CHANGE);  //interrupt setup keypad row 2
  attachInterrupt(digitalPinToInterrupt(6),ReadInput,CHANGE);  //interrupt setup keypad row 3*/
}

void loop() {
  char padInput = keypad.getKey();

  if(Serial.available() > 0){  
    state = Serial.read();
  } 

  if(state){
    ledType = state;
  }
  else if(padInput){
    ledType = padInput;
  }
  
  LightController(ledType);
  delay(DELAY);
}

void LightController(int setting){
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1;
  
  switch (ledType) {
    case '1':
      visualize_music();
      break;
    case '2':
      MiddleOut();
      break;
    case '3': 
      Proton();
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
      break;
    case '8':
      PingPong(0, 0xff, 0, 5, 10);
      break;
    case '9':
      DoublePong(0, 0xff, 0, 10, 5, 10);
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
      SetAll(0, 255, 0);
      break;
  }
}

void showStrip(){
  FastLED.show();
}

void ReadInput(){
  Serial.println("IN");
  char padInput = keypad.getKey();

  //check for new input
  //needed as there null is read if no input 
  if(padInput){
    ledType = padInput;
    Serial.println(padInput);
  }
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

void SetAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CRGB(red, green, blue);
  }
   FastLED.show();
}

void UserColour(){
  char red[4]={'0','0','0','\0'}, green[4]={'0','0','0', '\0'}, blue[4]={'0','0','0','\0'};
  int redVal, greenVal, blueVal;
  int index = 0, count = 0;
  char padInput;
   
  while(index != 3){
    padInput = keypad.getKey();
    if(padInput){
      if(padInput == '*'){
        index++;
        count=0;
      }
      else{
        if(index == 0){
          green[count]=padInput;
          count++;
        }
        else if (index == 1){
          red[count]=padInput;
          count++;
        }
        else if(index == 2){
          blue[count]=padInput;
          count++;
        }  
      }      
    } 
  } 
  redVal = atoi(red);
  blueVal = atoi(blue);
  greenVal = atoi(green);

  SetAll(redVal, greenVal, blueVal);
}

int ReadSoundDifference() {
  int soundValue1 = analogRead(ANALOG_READ);
  delay(1);
  int soundValue2 = analogRead(ANALOG_READ);
  int diff = soundValue1 - soundValue2;
  return diff;
}

void PingPong(byte red, byte green, byte blue, int SpeedDelay, int ReturnDelay){
  if(width >= 25){
    reverse = 1;
  }
  if(width == 1 && reverse){
    reverse = 0;
  }
    
  RightToLeft(red, green, blue, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, SpeedDelay, ReturnDelay);
}

void DoublePong(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

void MiddleOut(){
  int s=ReadSoundDifference();
  Serial.println(s);
  if((s>=50))
  {
    leds[NUM_LEDS/2]=CRGB(20, 135, 217);
    leds[NUM_LEDS/2+1]=CRGB(20, 135, 217);
  }
  else if((s<=-50))
  {
    leds[NUM_LEDS/2]=CRGB (20, 217, 30);
    leds[NUM_LEDS/2+1]=CRGB (20, 217, 30);
  }
  else
  {
    leds[NUM_LEDS/2] = CRGB(0, 0, 0xff);
    leds[NUM_LEDS/2+1] = CRGB(0, 0, 0xff);
  }
  for (int i = 0; i <= NUM_LEDS/2; i++) 
  {
    leds[i] = leds[i+1];
    leds[NUM_LEDS-i] = leds[NUM_LEDS-1-i];
  }
 FastLED.show();
 delay(10);
}

void Proton(){
    for(int i = 0; i < 100; i++) {
      SetAll(0,0,0xff);
      leds[i] = CRGB(0, 0xff, 0);
      showStrip();
      delay(5);
    }
    for(int i = 100; i > 0; i--) {
      SetAll(0,0,0xff);
      leds[i] = CRGB(0, 0xff, 0);
      showStrip();
      delay(5);
    }
  delay(10);
}

void LeftToRight(byte red, byte green, byte blue, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i < NUM_LEDS-width-2; i++) {
    SetAll(0,0,0xff);
    for(int j = 0; j < width; j++) {
      leds[i+j] = CRGB(0, 0xff, 0);
    }
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
  if(reverse)
    width--;
  else
    width++;
}

void RightToLeft(byte blue, byte red, byte green, int SpeedDelay, int ReturnDelay) {
  for(int i = NUM_LEDS-width-2; i > 0; i--) {
    SetAll(0,0, 0xff);
    for(int j = 0; j < width; j++) {
      leds[i+j] = CRGB(0, 0xff, 0);
    }
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
  if(reverse)
    width--;
  else
    width++;
}

void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i =((NUM_LEDS-EyeSize)/2); i>=0; i--) {
    SetAll(0, 0, 0xff);
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
  for(int i = 0; i<=((NUM_LEDS-EyeSize)/2); i++) {
    SetAll(0,0,0xff);
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

//Main function for visualizing the sounds
void visualize_music() {
  int sensor_value, mapped, avg, longavg;

  //Actual sensor value
  sensor_value = analogRead(ANALOG_READ);
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
