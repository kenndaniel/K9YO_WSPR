void PPSinterrupt()
{
  // Calibration function that counts SI5351 clock 2 for 40 seconds
  // Called on every pulse per second after gps sat lock
  if (CalibrationDone == true) return;
  tcount++;
  if (tcount == 4)  // Start counting the 2.5 MHz signal from Si5351A CLK0
  {
    TCCR1B = 7;    //Count on rising edge of pin 5
    TCNT1  = 0;    //Reset counter to zero
    mult=0;
  }
  else if (tcount == 14)  //The 10 second counting time has elapsed - stop counting
  {     
    TCCR1B = 0;                                  //Turn off counter
    // XtalFreq = overflow count + current count
    XtalFreq = (mult * 0x10000 + TCNT1)-5;  // Actual crystal frequency
    correction = 25000000./(float)XtalFreq;
    // I found that adjusting the transmit freq gives a cleaner signal

    mult = 0;
    tcount = 0;                              //Reset the seconds counter
    CalibrationDone = true;                 
  }
}

// Timer 1 overflow intrrupt vector.
ISR(TIMER1_OVF_vect) 
{ // This executes when the count of cycles on pin 5 overflows
  // pin 5 is connected to clock 2 of si5351
  mult++;                                          //Increment multiplier
  TIFR1 = (1<<TOV1);                               //Clear overlow flag 
}