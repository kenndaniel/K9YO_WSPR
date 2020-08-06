
// ------ Message initialize functions -----

#include "code_message.h"
#include "measurement.h"


void madenhead(double flat, double flon, char MH[]);

void code_telemetry_callsign()
{
  float tempCPU = getTempCPU();

  // code telemetry callsign
  code_telem_callsign(call_telemetry, gpsAltitude, volts, gpsSpeed, tempCPU);
  //code_telem_callsign(call_telemetry, gpsAltitude, radiation, tempCPU, tempOutside);
}

charArrayCpy(char dest[], char src[], int cnt)
{
  for (int i = 0; i< cnt; i++)
  {
    dest[i]= src[i];
    if(src[i]== '\0') return;
  }
  dest[cnt] = '\0';
}

void code_telemetry_location()
{
  //EEProm EEprom;
  // calculate madenhead
    POUTPUTLN((latitude, 6));
    POUTPUT((F(" Long=")));
    POUTPUTLN((longitude, 6));
  double lon = longitude ;
  double lat = latitude ;
  //double lon = EEprom.getLongitude();
  //double lat = EEprom.getLatitude();
  POUTPUT((F(" Lat Long :")));
  POUTPUT((lat));
  POUTPUT((F("**")));
  POUTPUT((lon));
  POUTPUT((F("**")));
  char MH[9] = {'A', 'A', '0', '0', 'A', 'A', '0', '0', '\0'}; // Buffer for locator string
  madenhead(lat, lon, MH);
  charArrayCpy(loc4, MH, 4);
  POUTPUTLN((loc4));
  POUTPUTLN((MH));
  charArrayCpy(loc6, MH, 6);
  charArrayCpy(loc8, MH[6], 2);
}

void code_standard_power()
{
  dbm_telemetry = codeAltitude(gpsAltitude);
}

void code_telemetry_power()
{
  int radiation = getRadiation();
  dbm_telemetry = codeRadiation(radiation, 0, 950);
}
