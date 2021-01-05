/*
   Timing Schedule - See readme file
*/
#include "./src/CodeMessages.h"

void SendMessages() // Timing
{
  // run additional scripts here to generate data prior to TX if there is a large delay involved.

  code_telemetry_location();  // Get position and update 4-char locator, 6-char locator and last 2 chars of 8-char locator
  POUTPUTLN((F("Coded Location")));
  code_standard_power(); // Update WSPR power level (standard message -> coded altitude)
  POUTPUTLN((F("Coded stand Pwr")));

  setModeWSPR(); // set WSPR standard message mode

  POUTPUT((F("Wait msg1 ")));
  POUTPUT((minute()));
  POUTPUTLN(());
  POUTPUT(("-"));
  POUTPUT((minute()));
  POUTPUT((":"));
  POUTPUTLN(((int)second()));
  int stopSecond = 0;
  int curSecond = 0;
  int it = 0;
  const unsigned long period = 50;
  unsigned long time_now = 0;
// Wait for the beginning of the even minute after xtal calibration is completed
  while (!((int)minute() % 2 == 0 && (int)second() < 2 && CalibrationDone == true))
  {
    time_now = millis();
    while(millis() < time_now + period){
        //wait 50 ms
    }
    it++;
    //if (it % 20 == 0)
    //{
      curSecond = (int)second();

      if (curSecond % 2 == 0 && curSecond != stopSecond)
      {
        digitalWrite(DBGPIN, HIGH);
        stopSecond = curSecond;
        POUTPUT(("-"));
        POUTPUT((minute()));
        POUTPUT((":"));
        POUTPUTLN(((int)second()));

        POUTPUT(F(" Xtal Correction Count "));
        POUTPUTLN((tcount));

      }
      else
      {
        digitalWrite(DBGPIN, LOW);
      }
  }

  detachInterrupt(digitalPinToInterrupt(ppsPin)); // Disable the gps pps interrupt

  rf_on(FreqCorrection_ppb);  // calibrate and turn on transmitter
  POUTPUT(F(" Xtal Correction ppb "));
  POUTPUTLN((FreqCorrection_ppb));
  POUTPUTLN(F(" Sending Standard Message "));
  encode();      // begin radio transmission
  
  //wdt_reset();
  code_telemetry_callsign();    // Update WSPR telemetry callsign 
  code_telemetry_power();       // change power to solar radiation
  POUTPUTLN((F("Waiting for Telemetry Message ")));
  POUTPUTLN(());
  setModeWSPR_telem(); // set WSPR telemetry message mode

  it = 0;
  while (!((int)minute() % 2 == 0 && (int)second() < 2))
  {
    time_now = millis();
    while(millis() < time_now + period){
        //wait 50 ms
    }

      curSecond = (int)second();

      if (curSecond % 2 == 0 && curSecond != stopSecond)
      {
        digitalWrite(DBGPIN, LOW);
        stopSecond = curSecond;
        POUTPUT(("-"));
        POUTPUT((minute()));
        POUTPUT((":"));
        POUTPUTLN(((int)second()));
      }
      else
      {
        digitalWrite(DBGPIN, HIGH);
      }
    
  }
  POUTPUTLN((F("Sending Telemetry Message ")));

  encode();            // begin radio transmission
  rf_off();
  
  sleep();
}

void(* resetFunc) (void) = 0; // Reset the Arduino

void sleep()  // Reset the system by turning the power off
{
    // shut off the power
    POUTPUTLN((F("Sleep Pwr off")));
    gpsOff();
    digitalWrite(SLEEP_PIN, HIGH);  // This will turn off the timer when using a battery
    unsigned long duration = (unsigned long)(SEND_INTERVAL*60000);
    delay(duration);  // Wait until it is time for the next transmission
    resetFunc();  // Reset Arduino
    //delay(6000);  // for testing purposes only
}



void txTest()
{
 
  loc4[0] = 'E';
  loc4[1] = 'N';
  loc4[2] = '6';
  loc4[3] = '4';
  loc4[4] = '\0';
  code_standard_power(); // Update WSPR power level (standard message -> coded altitude)
  POUTPUTLN((F("Coded stand Pwr")));
  //telemetry_set = true;

  setModeWSPR(); // set WSPR standard mode
  encode();      // begin radio transmission
}
