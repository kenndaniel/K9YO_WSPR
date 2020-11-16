#define DEBUG // Debug output is generated if DEBUG is defined
//#define GPS_CHARGE  // When defined will keep the gps on all the time
const char call[] = "K9YO"; // Ameture callsign
const char telemID[] = "T1"; // Telemetry call prefix
//If you go to Wikipedia and look up ITU prefix you will find that there are many more prefixes available. 
//For example "any letter other than A,K,W,R,M,B,F,G,I,N, + 1", "X + any number", E8, E9,J9, " letter O + any number" , T9, "U + any number"
#define SEND_INTERVAL 1 // The number of minutes between transmissions
#define WSPR_FREQ       14096500UL  // Center of WSPR 20m band
#define WSPR_FREQ_OFFSET  400   // Added to the WSPR FREQ to get the final frequency

#include <Time.h>
#include <TimeLib.h>

//#pragma GCC diagnostic error "-Wconversion"
/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/
#include <avr/wdt.h>
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
unsigned long freq = WSPR_FREQ + WSPR_FREQ_OFFSET;

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
void rf_on();
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

#define RADIATION_PIN A7 // Analog radiation sensor
#define RFPIN 9 // Not used
#define SLEEP_PIN 2
#define DBGPIN 13
#define GPS_POWER 6 // Pull down to turn on GPS module
static const int RxPin = 4;
static const int TxPin = 3;
static const uint32_t GPSBaud = 9600;

#include "GPS.h"            // code to set U-Blox GPS into airborne mode
#include "ModeDef.h"        // JT mode definitions
#include "TelemFunctions.h" // Sends messages using SI5351
#include "timing4.h"        // scheduling

TinyGPSPlus gps;
SoftwareSerial ss(RxPin, TxPin);
// gps must lock within 1.5 minutes or system will sleep or use the default location
const unsigned long gpsTimeout = 180000; 
unsigned long gpsStartTime = 0;
void setup()
{
  pinMode(RADIATION_PIN, INPUT);
  pinMode(RFPIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  pinMode(GPS_POWER, OUTPUT);
  digitalWrite(RFPIN, LOW);
  digitalWrite(SLEEP_PIN, LOW);

  gpsOn();

#ifdef DEBUG
  Serial.begin(9600);
#endif
  //Serial.begin(9600);
  POUTPUTLN((F("START")));
  // If it is night do not attempt to transmit
  float cpuTemp = getTempCPU();
  POUTPUT((F(" Temp-> ")));
  POUTPUTLN((cpuTemp));
  // if it isn't daytime, go back to sleep


  POUTPUT(F(" Battery Voltage "));
  volts = readVcc();
  POUTPUTLN((volts));
  if (volts <= MIN_VOLTAGE)
  sleep();

  //ss.begin(GPSBaud);
  //POUTPUTLN(F(" GPS begin"));
  digitalWrite(DBGPIN, HIGH);
  //setGPS_AirBorne(); // Set GPS into airborne mode
  //delay(15000);
  gpsStartTime = millis();

  //gps_set_max_performance_mode(); // Set GPS into high performance mode
  //delay(500);
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
  //Serial.print(hour()); Serial.print(F(":"));Serial.println(minute());
  sleep();
  // Below is only executed when the timer is disconnected for development

}

bool clockSet = false, locSet = false, altitudeSet = false, speedSet = false;
int wiringCounter = 1;

bool gpsGetInfo()
{
  // TinyGPSPlus gps;
  // SoftwareSerial ss(RxPin, TxPin);
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
