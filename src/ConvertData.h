

void madenhead(double flat, double flon, char MH[])
{   // Convert Latitude and Longitude to grid square
    // flat Latitude
    // flong Longitude
    long lon = (flon * 100000) + 18000000L;
    long lat = (flat * 100000) + 9000000L;

    // calculate grid square
    MH[0] += lon / 2000000;
    MH[1] += lat / 1000000;
    MH[2] += (lon % 2000000) / 200000;
    MH[3] += (lat % 1000000) / 100000;

    MH[4] += ((lon % 200000) / 8333);
    MH[5] += ((lat % 100000) / 4166);
    MH[6] += ((lon % 200000) % 8333) / 833;
    MH[7] += ((lat % 100000) % 4166) / 416;

}

const char let[] = {"0123456789"};
const char slet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char codeNumberField(int min, int max, int value)
{ // Convert a value to a digit
   value = int(10 * (float)(value - min) / (float)(max - min));
    if (value < 0)
        value = 0;
    if (value > 9)
        value = 9;

    return let[value];
}

char codeCharacterField(int min, int max, int value)
{
    value = int(26. * (float)(value - min) / (float)(max - min));
    if (value < 0)
        value = 0;
    if (value > 25)
        value = 25;
    return slet[value];

}

char codeStdPosition2(int temp ,int satellites)
{ 
    char dlet[36];
    for(int i = 0; i < 10; ++i)
    {
        dlet[i] = let[i];
    }

    for(int i = 10; i < 36; ++i)
    {
        dlet[i] = slet[i-10];
    }

    int value = int(36 * (float)(temp + 30) / (float)(25 + 30));
    value = value - value%3;
    if (satellites <= 5)
        {value = value;}
    else if (satellites >= 6  && satellites <= 9)
        {value = value +1;}
    else 
        {value = value + 2;}

    if (value < 0)
        value = 0;
    if (value > 35)
        value = 35;
    
    return dlet[value];
}

uint8_t db[] = {0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40, 43, 47, 50, 53, 57, 60};

uint8_t code_dbField( int min, int max, int sensorValue)
{ // code sensor which will become the dbm for telemetry call instead of fine altitude
    int a;   
    a = 18.*(sensorValue-min)/(max-min);  
    if (a <= 0)
        a = 0;
    if (a > 18)
        a = 18;
    return db[a];
}

uint8_t codeAltitude(double altitude)
{ // altitude in meters which will become the dbm for standard call
    int a;
    a = altitude / 1000;
    if (a <= 0)
        a = 0;
    if (a > 18)
        a = 18;
    return db[a];
}

uint8_t codeFineAltitude(double altitude)
{ // fine altitude coded db position
    int a;
    a = (int)altitude % 1000;
    a = (int)(a)/60 ; 

    if (a <= 0)
        a = 0;
    if (a > 18)
        a = 18;
    return db[a];
}

uint8_t codeFineFineAltitude(double altitude)
{ // fine altitude coded db position
    int a;
    a = (int)altitude % 1000;
    a = (int)(a)%60 ; 

    if (a <= 0)
        a = 0;
    if (a > 18)
        a = 18;
    return db[a];
}

char codePos2(float altitude)
{ // The last three digits 0-1000 of altitude - Round the 0-999 to 0-9
    float a;
    a = (int)altitude % 1000;
    int i ;
    i = (int)(a)/100 ; 
    if(i > 9) i = 9;
    if(i< 0) i = 0;
    return let[i];
}



