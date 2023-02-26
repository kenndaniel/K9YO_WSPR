
#include "CustomSensor.h"



void code_std_telem_characters(char Callsign[],  float volts , float temp, int sats)
{ // compose the standard WB8ELK telemtry callsign -- convert values to characters and numbers
        Callsign[0] = std_telemID[0]; // first part of telem call e.g. Q
        Callsign[1] = codeStdPosition2(int(temp),sats);
        Callsign[2] = std_telemID[1]; // second part of telem call e.g. 1
        // Standard WB8ELK uses 3.3v to 5.8v for coding. 
        Callsign[3] = codeCharacterField(33, 58, int((volts/100.))); 
        Callsign[4] = loc6[4];  // 5th character of grid square
        Callsign[5] = loc6[5];  // 6th character of grid square
        Callsign[6] = '\0';
}

void code_standard_telemetry_callsign()
{
  float tempCPU = getTempCPU();
  float volts = readVcc();
  // code telemetry callsign
  code_std_telem_characters(call_telemetry, volts, tempCPU, satellites);
  
}

void code_telemety_loc()
{  // loc4_telemetry is coded into the 4 characters of the location field e.g. EN62
  // This can be changed to code sensor data.
  // The letters can take the value A-R the numbers 0-9
    charArrayCpy(loc4_telemetry, loc4, 4);
}

void code_telemetry_power()
{  // Altitude coded into the power field of the telemetry field
  
  dbm_telemetry = codeFineAltitude(gpsAltitude);
}

void code_characters(char Callsign[],  float gpsSpeed )
{ // compose the custom telemtry callsign and convert values to characters and number
        Callsign[0] = telemID[0]; // first part of telem call  e.g. T
        Callsign[1] = telemID[1]; // second part of telem call e.g. 1
        // The remainder of this message can be customized for different sensors
        int fineSpeed = int(gpsSpeed) % 8;
        Callsign[2] = codeNumberField(0, 9, fineSpeed);  // third position must be a number
        Callsign[3] = codeCharacterField(0, 200, int(gpsSpeed));
        
        Callsign[4] = loc6[4];  // Example 5th character of grid square
        Callsign[5] = loc6[5];  // Example 6th character of grid square
        Callsign[6] = '\0';
        Serial.println(Callsign);

}

void code_custom_telemetry_callsign()
{
  float tempCPU = getTempCPU();

  // code telemetry callsign
  code_characters(call_telemetry, gpsSpeed);
  
}

void code_custom_telemetry_power()
{  // Altitude 0-60 is coded into the power field of the telemetry field

  dbm_telemetry = codeFineFineAltitude(gpsAltitude);
}
