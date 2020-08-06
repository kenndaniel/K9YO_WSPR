
uint8_t db[] = {0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40, 43, 47, 50, 53, 57, 60};

uint8_t codeAltitude(double altitude)
{ // altitude in meters which will become the dbm for standard call
    int a;
    a = altitude / 800;
    if (a <= 0)
        a = 0;
    if (a > 18)
        a = 18;
    return db[a];
}

// uint8_t code_db(int measurement, int min, int max)
// { // code pressure in millibars which will become the dbm for telemetry call
//     int a;   
//     a = 18.*(measurement-min)/(max-min);  
//     if (a <= 0)
//         a = 0;
//     if (a > 18)
//         a = 18;
//     return db[a];
// }

uint8_t codeRadiation(int radiation, int min, int max)
{ // code pressure in millibars which will become the dbm for telemetry call
    int a;   
    a = 18.*(radiation-min)/(max-min);  
    if (a <= 0)
        a = 0;
    if (a > 18)
        a = 18;
    return db[a];
}

const char let[] = {"0123456789"};

char codePos2(float altitude)
{ // The last three digits 0-799 of altitude - Round the 0-999 to 0-9
    float a;
    a = (int)altitude % 800;
    int i ;
    i = (int)(a)/100 ; 
    if(i > 9) i = 9;
    if(i< 0) i = 0;
    return let[i];
}

// char codePos2Measurement(float measurement, int min, int max)
// { // The last three digits 0-799 of altitude - Round the 0-999 to 0-9
//     int a;
//     a = int(10 * (float)(value - min) / (float)(max - min)); 
//     if(i > 9) i = 9;
//     if(i< 0) i = 0;
//     return let[i];
// }

const char slet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char codePos456(int min, int max, int value)
{
    value = int(26 * (float)(value - min) / (float)(max - min));
    if (value < 0)
        value = 0;
    if (value > 25)
        value = 25;

    return slet[value];
}



// uint8_t codeDBM(float solarV, int numSat)
// {
//     float v[] = {.2, .4, .6, .8, 1.0, 1.2};
//     int i;
//     for (i = 0; i < 6; i++)
//     {
//         if (solarV <= v[i])
//             break;
//     }
//     if (i == 6) i=5;
//     int sat = 0;
//     if (numSat >= 4 && numSat < 7)
//         sat = 1;
//     else if (numSat >= 7)
//         sat = 2;

//     int index = i * 3 + sat;
//     return db[index];
// }

// int codeBatVolt(float volts)
// {
//     int vi = 11;
//     float v[] = {3.,3.1, 3.2,3.3, 3.4,3.5, 3.6, 3.7,3.8, 4.0, 4.2};
//     int i;
//     for (i = 0; i < vi; ++i)
//     {
//         if (volts <= v[i])
//             break;
//     }
//     if (i == vi ) i = vi -1;
//     return i;
// }

void madenhead(double flat, double flon, char MH[])
{
    long lon = (flon * 100000) + 18000000L;
    long lat = (flat * 100000) + 9000000L;

    //POUTPUT((F(" Lat Long :")));
    //POUTPUT((lat));
    //POUTPUT((F("**")));
    //POUTPUT((lon));
    //POUTPUT((F("**")));

    MH[0] += lon / 2000000;
    MH[1] += lat / 1000000;
    MH[2] += (lon % 2000000) / 200000;
    MH[3] += (lat % 1000000) / 100000;

    MH[4] += ((lon % 200000) / 8333);
    MH[5] += ((lat % 100000) / 4166);
    MH[6] += ((lon % 200000) % 8333) / 833;
    MH[7] += ((lat % 100000) % 4166) / 416;


}

void code_telem_callsign(char Callsign[], float altitude, int volts,  float gpsSpeed , float tempOutside)
{
        Callsign[0] = telemID[0]; // first part of index/channel e.g. T
        Callsign[1] = telemID[1]; // second part of index/channel e.g. 1

        Callsign[2] = codePos2(altitude); //Altitude fine
        
        Callsign[3] = codePos456(2000, 5000, int(volts));
        Callsign[4] = codePos456(-45, 10, int(tempOutside));
        Callsign[5] = codePos456(0, 200, int(gpsSpeed));
        Callsign[6] = '\0';

}