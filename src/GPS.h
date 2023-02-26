/*
   GPS functions
*/



void SetHighAltitude()
{
  // put the gps in high altitude mode
  ss.write("$PMTK886,3*2B\r\n");
}

bool SetCPUClock(TinyGPSPlus gps)
{
  // Sets the cpu clock with the latest available GPS time


    byte h, m, s, mon, d;
    int y;
    unsigned long age;
    mon = gps.date.month();
    d = gps.date.day();
    y = gps.date.year();
    h = gps.time.hour();
    m = gps.time.minute();
    s = gps.time.second();
    age = gps.time.age();  // Time since sat syncronization - add to sat time

    if (age > 1000 && age < (195000)) // limited to size of byte 255-60
    {
      s += (byte)(age / 1000);
      if (s >= 60)
      {
        byte mins = s / 60;
        s = s % 60;
        m += mins;
        if (m >= 60)
        { // not good for more than 1 hr 
          m = m - 60;
          h += 1;
          if (h > 24)
            h = 0; // day might be off, but not important
        }
      }
    }

    setTime((int)h, (int)m, (int)s, (int)d, (int)mon, (int)y);
    if (timeStatus() != timeSet)
    {
      POUTPUTLN((F(" Time not set")));
      return false;

    }
      POUTPUTLN((s));
      POUTPUTLN((F("cpu time set - waiting for position")));
      return true;
  
}



// not needed for newer gps units
void gpsOn()
{
  // not used
  //digitalWrite(GPS_POWER, LOW);
}

void gpsOff()
{
  // not used
  //digitalWrite(TxPin, LOW);
  //#ifndef GPS_CHARGE
  //digitalWrite(GPS_POWER, HIGH);
  //#endif
}

void gpsBounce()
{
  // Not used for newer gps
  //  Turn the power to GPS off and after .5 sec turn it on again
  //gpsOff();
  //delay(500);
  //gpsOn();
}