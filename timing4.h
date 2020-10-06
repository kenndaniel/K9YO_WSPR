/*
   Timing Schedule - See readme file
*/
#include "./src/telemetry.h"
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


void SendMessages() // Timing
{

  // run additional scripts here to generate data prior to TX if there is a large delay involved.

 
  rf_on();  // turn on SI5351 and let it warm up

  code_telemetry_location();  // Get position and update 4-char locator, 6-char locator and last 2 chars of 8-char locator
  POUTPUTLN((F("Coded Location")));
  code_standard_power(); // Update WSPR power level (standard message -> coded altitude)
  POUTPUTLN((F("Coded stand Pwr")));
  //telemetry_set = true;

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

  while (!((int)minute() % 2 == 0 && (int)second() < 2))
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
      }
      else
      {
        digitalWrite(DBGPIN, LOW);
      }
  }

  encode();      // begin radio transmission
  
  //wdt_reset();
  code_telemetry_callsign();    // Update WSPR telemetry callsign 
  code_telemetry_power();       // change power to solar radiation
  POUTPUTLN((F("Wait msg2 ")));
  POUTPUT((minute()));
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
  POUTPUTLN((F("Sndng telem msg")));

  encode();            // begin radio transmission
  rf_off();
  
  sleep();
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
