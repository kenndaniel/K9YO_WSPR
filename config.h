/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   Modified to be simpler and work on standard Arduino by K9YO Chicago IL 2019 - March 2022
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/
#define DEBUG // Debug output is generated if DEBUG is defined
//#define DEBUG_SI5351  // Used only for testing transmission when no gps module is attached
const char call[] = "K9YO";     // Amateur callsign
const char std_telemID[] = "Q6";  // Standard telemetry prefix e.g. Q1 is Qx1xxx
const int send_time_slot = 6;   // ten minute telemetry transmission time slot: 0,2,4,6 or 8
// Typically nothing to be changed below this point

// Center frequency of the band to transmit on
#define WSPR_FREQ       14097100  // Center of WSPR 20m band - actual frequency is random each transmission
// Information for the custom telemetry message
const char telemID[] = "T1";   // optional non-standard telemetry call prefix  T1 is T1xxx
//If you go to Wikipedia and look up ITU prefix you will find that there are many more prefixes available. 
//For example "any letter other than A,K,W,R,M,B,F,G,I,N, + 1", "X + any number", E8, E9,J9, " letter O + any number" , T9, "U + any number"
// End of user definable information