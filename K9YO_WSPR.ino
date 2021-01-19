#define DEBUG // Debug output is generated if DEBUG is defined
//#define GPS_CHARGE  // When defined will keep the gps on all the time
const char call[] = "K9YO"; // Amateur callsign
const char telemID[] = "T1"; // Telemetry call prefix
//If you go to Wikipedia and look up ITU prefix you will find that there are many more prefixes available. 
//For example "any letter other than A,K,W,R,M,B,F,G,I,N, + 1", "X + any number", E8, E9,J9, " letter O + any number" , T9, "U + any number"
#define SEND_INTERVAL 1 // The number of minutes between transmissions
#define WSPR_FREQ       14097100  // Center of WSPR 20m band
// Variables needed for SI5351 processing
volatile bool CalibrationDone = false; 
volatile unsigned long mult=0;
volatile unsigned int tcount=0;
volatile unsigned long XtalFreq=25000000UL;
volatile int32_t FreqCorrection_ppb = 0;
volatile float correction =0;
unsigned long freq = (unsigned long) (WSPR_FREQ);

#include <Time.h>

//#pragma GCC diagnostic error "-Wconversion"
/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/
#include <avr/interrupt.h> 
#include <avr/io.h> 
//#include <avr/wdt.h>
#include <TimeLib.h>
#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <TinyGPS++.h>

//TinyGPSPlus gps;

// Enumerations
enum mode
{
  MODE_JT9,
  MODE_WSPR
};

Si5351 si5351;
JTEncode jtencode;

// Global variables


#define DAYTIME_RADIATION 180 // Operation is prevented below these values (e.g. nighttime)
#define MORN_TEMP -30
#define MIN_VOLTAGE 2.9
#define BMP280_I2C_ALT true // True if the pressure sensor (BMP280) uses the alternate i2c add
//uint8_t dbm; // dbm field of WSPR

float tempOutside, pressure; // set once in tempPress.h
int volts = 0;
double gpsAltitude = 10000; // Testing value
double gpsSpeed = 0.;       // Testing values
char call_telemetry[7];     // WSPR telemetry callsign
char loc_telemetry[5];      // WSPR telemetry locator
uint8_t dbm_telemetry;      // WSPR telemetry dbm

char message1[14] = ""; // Message1 (13 char limit) for JT9
char message2[14] = ""; // Message2 (13 char limit) for JT9

char loc4[5]; // 4 digit gridsquare locator
char loc6[7]; // 6 digit gridsquare locator
char loc8[3]; // Last 2 digits of the 8-digit locator

byte Hour, Minute, Second; // used for timing
long lat, lon;             //oldlat, oldlon;     // used for location
//uint8_t tx_buffer[255];            // WSPR Tx buffer
uint8_t tx_buffer[165];
uint8_t symbol_count = WSPR_SYMBOL_COUNT; // JTencode
uint16_t tone_delay, tone_spacing;        // JTencode

int alt_meters = 0;
bool telemetry_set = false;
int Sats = 0;
int gps_speed = 0;
double latitude = 10.;
double longitude = 10.;

// Function prototypes below
double getAltitude();
int readRadiation();
void setGPStime();
void rf_off();
void rf_on(int32_t freqCorrection);
void call_telem();
void loc8calc();
void call_dbm_telem();
void loc_dbm_telem();
void sleep();
float getTempCPU();
double getTemperature();

#ifdef DEBUG
#define POUTPUT(x) Serial.print x
#else
#define POUTPUT(x)
#endif

#ifdef DEBUG
#define POUTPUTLN(x) Serial.println x
#else
#define POUTPUTLN(x)
#endif

#define ppsPin  2 
#define RADIATION_PIN A7 // Analog radiation sensor
#define RFPIN 9 // Not used
#define SLEEP_PIN 7
#define DBGPIN 13
#define GPS_POWER 6 // Pull down to turn on GPS module
#define  XtalCalibratCtPIN 5   // SI5351 calibration clock
static const int RxPin = 4;
static const int TxPin = 3;
static const uint32_t GPSBaud = 9600;

#include "GPS.h"            // code to set U-Blox GPS into airborne mode
//#include "ModeDef.h"        // JT mode definitions
#include "SI5351Interface.h" // Sends messages using SI5351
#include "SendMessages.h"        // schedules the sending of messages

TinyGPSPlus gps;
SoftwareSerial ss(RxPin, TxPin);
// gps must lock within 15 minutes or system will sleep or use the default location
const unsigned long gpsTimeout = 9000000; 
unsigned long gpsStartTime = 0;

void PPSinterrupt()
{
  // Called on every pulse per second after gps sat lock
  if (CalibrationDone == true) return;
  tcount++;
  if (tcount == 4)  // Start counting the 2.5 MHz signal from Si5351A CLK0
  {
    TCCR1B = 7;    //Count on rising edge of pin 5
    TCNT1  = 0;    //Reset counter to zero
  }
  else if (tcount == 44)  //The 40 second counting time has elapsed - stop counting
  {     
    TCCR1B = 0;                                  //Turn off counter
    // XtalFreq = overflow count + current count
    XtalFreq = 40 + (mult * 0x10000 + TCNT1)/4;  
    correction = 25000000./(float)XtalFreq;
    //freq = (unsigned long) (WSPR_FREQ*(correction));
    freq = (unsigned long) (WSPR_FREQ);
    FreqCorrection_ppb = (int32_t)((1.-correction)*1e9);
    POUTPUT(F(" Final Xtal Corrections "));
    POUTPUTLN((XtalFreq));
    POUTPUTLN((freq));
    POUTPUTLN((FreqCorrection_ppb));          //Calculated correction factor
    mult = 0;
    tcount = 0;                              //Reset the seconds counter
    CalibrationDone = true;                  
  }
}

// Timer 1 overflow intrrupt vector.
ISR(TIMER1_OVF_vect) 
{ // This executes when the count of cycles on pin 5 overflows
  // pin 5 is connected to clock 2 of si5351
  mult++;                                          //Increment multiplier
  TIFR1 = (1<<TOV1);                               //Clear overlow flag 
}
void setup()
{
    //Set up Timer1 as a frequency counter - input at pin 5
  TCCR1B = 0;                                    //Disable Timer5 during setup
  TCCR1A = 0;                                    //Reset
  TCNT1  = 0;                                    //Reset counter to zero
  TIFR1  = 1;                                    //Reset overflow
  TIMSK1 = 1;                                    //Turn on overflow flag

  pinMode(RADIATION_PIN, INPUT);
  pinMode(RFPIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  pinMode(GPS_POWER, OUTPUT);
  digitalWrite(RFPIN, LOW);
  digitalWrite(SLEEP_PIN, LOW);

#ifdef DEBUG
  Serial.begin(9600);
#endif

  gpsOn();

  si5351_calibrate_init();



    // Inititalize GPS 1pps input
  pinMode(ppsPin, INPUT_PULLUP);

  // Set 1PPS pin 2 for external interrupt input
  attachInterrupt(digitalPinToInterrupt(ppsPin), PPSinterrupt, RISING); 

  //Serial.begin(9600);
  POUTPUTLN((F("START")));
  float cpuTemp = getTempCPU();
  POUTPUT((F(" Temp-> ")));
  POUTPUTLN((cpuTemp));

  POUTPUT(F(" Battery Voltage "));
  volts = readVcc();
  POUTPUTLN((volts));
  if (volts <= MIN_VOLTAGE)
  sleep();

  digitalWrite(DBGPIN, HIGH);

  gpsStartTime = millis();

}

long loopi = 0;
int loopj = 0;
bool rfpinon = false;
void loop()
{
  gpsOn();

  if (gpsGetInfo() == false)
    sleep(); // did not sync with sats

  gpsOff();
  POUTPUTLN((F(" Starting Transmit Logic")));
  SendMessages();
  sleep();

}

bool clockSet = false, locSet = false, altitudeSet = false, speedSet = false;
int wiringCounter = 1;

bool gpsGetInfo()
{

  clockSet = false, locSet = false, altitudeSet = false, speedSet = false;
  ss.begin(GPSBaud);
  unsigned long millsTime = 0;
  gpsStartTime = millis();
  millsTime = millis();
  POUTPUTLN((millsTime));
  POUTPUTLN((gpsTimeout));
  while (millis() < gpsStartTime + gpsTimeout)
  {
    
    while (ss.available() > 0)
      gps.encode(ss.read()); 

    if (gps.time.isUpdated() && gps.satellites.value() > 0)
    {
      if (SetCPUClock( gps))
      {
        clockSet = true;
      }

    }
    if (gps.altitude.isUpdated())
     {
       gpsAltitude = gps.altitude.meters();
       altitudeSet=true;
     }
    if (gps.speed.isUpdated())
     {
        gpsSpeed = gps.speed.kmph();
        speedSet = true;
     }
    if (gps.location.isUpdated())
     {
       latitude = gps.location.lat();
       longitude = gps.location.lng();
       locSet = true;
     }
       altitudeSet=true;
     if( locSet && speedSet && altitudeSet && clockSet )
      return true;

    loopi++;

    if (gps.charsProcessed() < 10 && millis() % 1500 < 5)
    {
      
      POUTPUTLN((F("WARNING: No GPS data.  Check wiring.")));
      // blink moris code "w" for wiring
      digitalWrite(DBGPIN, HIGH);
      delay(50);
      digitalWrite(DBGPIN, LOW);
      delay(50);
      digitalWrite(DBGPIN, HIGH);
      delay(150);
      digitalWrite(DBGPIN, LOW);
      delay(50);
      digitalWrite(DBGPIN, HIGH);
      delay(150);
      digitalWrite(DBGPIN, LOW);
      wiringCounter ++;
      if (wiringCounter > 15)
      { 
        wiringCounter = 0;
        #ifdef DEBUG
          return true;
        #endif
        gpsBounce();
      }
    }

  }
  POUTPUTLN((F(" GPS Timeout ")));
  if(clockSet==true)
  {
    // Send report anyway if only the clock has been set
    clockSet = false; // needed for testing only
    locSet = false;
    altitudeSet = false;
    speedSet = false;
    return true;
  }
  else
    return false;
}
