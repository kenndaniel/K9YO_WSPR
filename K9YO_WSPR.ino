/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   Modified to be simpler and work on standard Arduino by K9YO Chicago IL 2019 - March 2022
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/

// Modify the callsign and telemetry channel info in this file
#include "config.h" 

#define SEND_INTERVAL 1 // The minimum number of minutes between transmissions

#define SENSOR_PIN A7 // Generic analog sensor - A7 can be changed to any unused pin
    
// Variables needed for SI5351 calibration processing
volatile bool CalibrationDone = false; 
volatile unsigned long mult=0;
volatile unsigned int tcount=0;
volatile unsigned long XtalFreq=25000000UL;
volatile int32_t FreqCorrection_ppb = 0;
volatile float correction =0;
unsigned long freq = (unsigned long) (WSPR_FREQ);


#include <Time.h>
#include <avr/interrupt.h> 
#include <avr/io.h> 
#include <avr/wdt.h>
#include <si5351.h>
#include <JTEncode.h>
#include <SoftwareSerial.h>
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


#define MIN_VOLTAGE 2.6

//float tempOutside, pressure; // set once in tempPress.h
int volts = 0;
double gpsAltitude = 0; // Testing value
double gpsSpeed = 0.;       // Testing values
char call_telemetry[7];     // WSPR telemetry callsign
char loc_telemetry[5];      // WSPR telemetry locator
uint8_t dbm_telemetry;      // WSPR telemetry dbm
uint8_t dbm_standard;      // WSPR telemetry dbm

char message1[14] = ""; // Message1 (13 char limit) for JT9
char message2[14] = ""; // Message2 (13 char limit) for JT9

char loc4[5]; // 4 digit grid square locator
char loc4_telemetry[5]; // 4 digit grid square used for the telemetry message
char loc6[7]; // 6 digit grid square locator
char loc8[3]; // Last 2 digits of the 8-digit locator

byte Hour, Minute, Second; // used for timing
long lat, lon;             // used for location
//uint8_t tx_buffer[255];            // WSPR Tx buffer
uint8_t tx_buffer[165];
uint8_t symbol_count = WSPR_SYMBOL_COUNT; 
uint16_t tone_delay, tone_spacing;        // for digital encoding
int satellites = 0;
int alt_meters = 0;
bool telemetry_set = false;
int Sats = 0;
int gps_speed = 0;
double latitude = 10.;
double longitude = 10.;

// Function prototypes below
void sleep();

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
#define RANDOM_PIN 0 // used to generate a seed for the random number gerator
#define ppsPin  2 // GPS pulse per second for transmit frequency calibration (interrupt)
#define RFPIN 9 // Can be used to turn off RF see rf_off() and sleep()
#define SLEEP_PIN 7 // Not used - can be used to turn off system between transmissions see sleep()
#define DBGPIN 13
#define GPS_POWER 6 // Pull down to turn on GPS module (not used) see sleep()
#define  XtalCalibratCtPIN 5   // SI5351 calibration signal (counter)
static const int RxPin = 4;  // for serial communication with gps
static const int TxPin = 3;  // for serial communication with gps
static const uint32_t GPSBaud = 9600;

SoftwareSerial ss(RxPin, TxPin);
#include "./src/GPS.h"            // code to set U-Blox GPS into airborne mode
#include "./src/SI5351Interface.h" // Sends messages using SI5351
#include "SendMessages.h"        // schedules the sending of messages

TinyGPSPlus gps;

// gps must lock position within 15 minutes or system will sleep or use the default location if the clock was set
const unsigned long gpsTimeout = 900000; // in milliseconds
unsigned long gpsStartTime = 0;

#include "./src/FrequencyCorrection.h"
void setup()
{
  wdt_enable(WDTO_8S);
    //Set up Timer1 as a frequency counter - input at pin 5
  TCCR1B = 0;                                    //Disable Timer5 during setup
  TCCR1A = 0;                                    //Reset
  TCNT1  = 0;                                    //Reset counter to zero
  TIFR1  = 1;                                    //Reset overflow
  TIMSK1 = 1;  
  uint8_t randomPin = RANDOM_PIN;                              //Turn on overflow flag
  randomSeed(analogRead( RANDOM_PIN ));  // Initialize random number generator
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RFPIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  pinMode(GPS_POWER, OUTPUT);
  digitalWrite(RFPIN, LOW);
  digitalWrite(SLEEP_PIN, LOW);

  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  POUTPUTLN((F("STARTING")));
  si5351_calibrate_init();

    // Inititalize GPS pps input
  pinMode(ppsPin, INPUT_PULLUP);

  // Set 1PPS pin 2 for external interrupt input
  attachInterrupt(digitalPinToInterrupt(ppsPin), PPSinterrupt, RISING); 

  float cpuTemp = getTempCPU();
  POUTPUT((F(" Temperture ")));
  POUTPUTLN((cpuTemp));

  POUTPUT(F(" Voltage "));
  volts = readVcc();
  POUTPUTLN((volts));
  //if (volts <= MIN_VOLTAGE) sleep();

  digitalWrite(DBGPIN, HIGH);

  gpsStartTime = millis();

}

long loopi = 0;
int loopj = 0;
bool rfpinon = false;
void loop()
{
  bool getInfo = gpsGetInfo();
  wdt_disable();
  if ( getInfo == false)
    sleep(); // did not sync with sats

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
  bool hiAltitudeSet = false;
  POUTPUTLN((F("Waiting for GPS to find satellites - 5-10 min")));
  while (millis() < gpsStartTime + gpsTimeout)
  {
    wdt_reset();
    while (ss.available() > 0)
      gps.encode(ss.read()); 

    if (gps.charsProcessed() > 10 && hiAltitudeSet == false)
    { // put the gps module into high altitude mode
      SetHighAltitude();
      //ss.write("$PMTK886,3*2B\r\n");
      hiAltitudeSet = true;
    }

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

     if( locSet && speedSet && altitudeSet && clockSet )
     {
      satellites = gps.satellites.value();
      POUTPUT((F(" Number of satellites found ")));
      POUTPUTLN((satellites));
      // start transmission loop
      return true;
     }

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
        // If DEBUG_SI5351 is defined, the system will transmit, but not on the correct minute
        // Use this for unit testing when there is no gps attached.
        #ifdef DEBUG_SI5351
          return true;
        #endif
      }
    }

  }
  POUTPUTLN((F(" GPS Timeout - no satellites found ")));
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
  {
    return false;
  }
}

