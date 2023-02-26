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
//#define DEBUG_SI5351  // Uncomment when testing without the GPS module attached.
const char call[] = "K9IQP";     // Amateur callsign
const char std_telemID[] = "07";  // Standard telemetry prefix e.g. Q1 is Qx1xxx
const int send_time_slot = 0;   // the time slot for the telemetry transmission : 0,2,4,6 or 8
// Typically nothing to be changed below this point

// WSPR Band Center Frequencies (Do not change)
#define WSPR_30m      10140200UL  // Center of WSPR 30m band - actual frequency is random each transmission
#define WSPR_20m      14097100UL  // Center of WSPR 20m band - actual frequency is random each transmission
#define WSPR_17m      18106100UL  // Center of WSPR 17m band - actual frequency is random each transmission
#define WSPR_15m      21096100UL  // Center of WSPR 15m band - actual frequency is random each transmission
#define WSPR_12m      24926100UL  // Center of WSPR 12m band - actual frequency is random each transmission
#define WSPR_10m      28126100UL  // Center of WSPR 10m band - actual frequency is random each transmission

// Technician license holders should set both WSPR_FREQ to WSPR_10m
// First band to transmit on
#define WSPR_FREQ1      WSPR_17m  

// Second band to transmit on
#define WSPR_FREQ2      WSPR_17m  
// Note that the standard telemetry time slot for FREQ2 will be send_time_slot + 3 for WSPR_FREQ2
// Information for the custom telemetry message
const char telemID[] = "T1";   // Not implemented -- optional non-standard telemetry for additional sensors
// Custom telemetry message code modifications are required for additonal sensors
// The user is reguired to look into the code and make custom modifications. Start with the bottom of SendMessages.h
// call prefix  T1 is T1xxx  Note that no non standard telemetry transmission is implemented in the this code
// These provide slightly less space for data encoding because it must be of the format e.g. T1abc.  There are only three 
// positions available for encoding.
//If you go to Wikipedia and look up ITU prefix you will find that there are many more prefixes available. 
//For example "any letter other than A,B,F,G,I,K,M,N,W,R, + 1", "X + any number", E8, E9,J9, " letter O + any number" , T9, "U + any number"
// End of user definable information