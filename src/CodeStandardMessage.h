
// ------ Message initialize functions -----


#include "Sensors.h"






void code_location()
{

  // calculate madenhead
    POUTPUT((F(" Latitude =")));
    POUTPUTLN((latitude, 6));
    POUTPUT((F(" Longitude =")));
    POUTPUTLN((longitude, 6));
  double lon = longitude ;
  double lat = latitude ;
  //double lon = EEprom.getLongitude();
  //double lat = EEprom.getLatitude();
  char MH[9] = {'A', 'A', '0', '0', 'A', 'A', '0', '0', '\0'}; // Buffer for locator string
  madenhead(lat, lon, MH);
  charArrayCpy(loc4, MH, 4);
  // POUTPUTLN((loc4));
  charArrayCpy(loc6, MH, 6);
  POUTPUT((F(" Grid Square =")));
  POUTPUTLN((loc6));
  charArrayCpy(loc8, MH, 2);
}

void code_standard_power()
{
  dbm_standard = codeAltitude(gpsAltitude);
}


