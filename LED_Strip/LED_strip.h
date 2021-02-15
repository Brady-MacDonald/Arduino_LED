//Header File

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
