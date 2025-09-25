/*
   Low level functions that perform the final message encoding and interface with the SI5351 Transmitter
*/
/*
   Mode defines
*/

#define WSPR_TONE_SPACING       146           // ~1.46 Hz

#define WSPR_DELAY              683          // Delay value for WSPR



void setModeWSPR()
{ // Sends the info to the si5351 for the standard message
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
  //freq = WSPR_FREQ; 
  memset(tx_buffer, 0, 165); // Clears Tx buffer from previous operation.
  POUTPUT((call));
  POUTPUT((F(" ")));
  POUTPUT((loc4));
  POUTPUT((F(" ")));
  POUTPUTLN((dbm_standard));  // rough altitude
  jtencode.wspr_encode(call, loc4, dbm_standard, tx_buffer);
}

void setModeWSPR_telem()
{  // sends the info to the si5351 for the telemetry message
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
  //freq = WSPR_FREQ; 
  memset(tx_buffer, 0, 165); // Clears Tx buffer from previous operation.
  POUTPUT((call_telemetry));
  POUTPUT((F("-")));
  POUTPUT((loc4_telemetry));
  POUTPUT((F("-")));
  POUTPUTLN((dbm_telemetry));
  jtencode.wspr_encode(call_telemetry, loc4_telemetry, dbm_telemetry, tx_buffer);
}


void si5351_calibrate_init()
{
  // Initialize SI5351 for gps calibration
  digitalWrite(RFPIN, HIGH);
  //delay(2000);
  POUTPUTLN((F(" SI5351 Initialized ")));
  bool siInit = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);    //  25MHz
  delay(2000);
  if (siInit == false) {Serial.println(" XXXXXXXXX Si5351 init failure XXXXXX");}
  //else {POUTPUTLN((F(" SI5355  Init Success")));}
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA); //  Check datasheet.
  unsigned long calfreq = 2500000UL;
  si5351.set_freq(calfreq*100, SI5351_CLK2);  // set calibration frequency to 2.5 MHz
  si5351.output_enable(SI5351_CLK2, 1);   // Enable output  
}

/*
   Message encoding
*/
#define XMIT_CLOCK0 SI5351_CLK0
#define XMIT_CLOCK1 SI5351_CLK1
bool twoChanel = true; // set true to use two channel inverted output, set false to use only one chanel

void rf_on()
{ 

  delay(10);

  si5351.set_ms_source(XMIT_CLOCK0, SI5351_PLLA);
  si5351.drive_strength(XMIT_CLOCK0, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
  si5351.drive_strength(XMIT_CLOCK1, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.

  si5351.set_clock_fanout(SI5351_FANOUT_MS, 1);
  si5351.set_clock_source(XMIT_CLOCK1, SI5351_CLK_SRC_MS0); // clock 1 gets freq from clock 0
  si5351.set_clock_invert(XMIT_CLOCK1, 1);

  si5351.output_enable(XMIT_CLOCK0, 1); 
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 1);
  
}

void rf_off()
{
  //Disable output
  si5351.output_enable(XMIT_CLOCK0, 0); 
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 0);
}


void transmit() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  rf_on();
  POUTPUTLN((F(" SI5351 Start Transmission ")));
  const unsigned long period = tone_delay;
  unsigned long time_now = 0;

  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = millis();
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), XMIT_CLOCK0); // clock 1 will follow this
    
    while (millis() < time_now + period) // Found to be more accruate than delay()
    {
    }
  }
  // Turn off the output
  rf_off();
}
// void transmit() // Loop through the string, transmitting one character at a time
// { 
//   bool twoChanel = true;  // set true to use channel 0 and 1 set false to use only channel 0
//   uint8_t i;
//   digitalWrite(RFPIN, HIGH);
//   si5351.output_enable(SI5351_CLK0, 1); // Reset the tone to the base frequency and turn on the output
//   if(twoChanel) si5351.output_enable(SI5351_CLK1, 1);
//   const unsigned long period = tone_delay;
//   unsigned long time_now = 0;
//   uint8_t one = 1;

//   for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
//   {
//     time_now = millis();
//     si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0); // not needed for inverted output on CLK!
//     if(twoChanel) si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK1);
//     if(twoChanel) si5351.set_clock_invert(SI5351_CLK1,one);
//     if(twoChanel) si5351.pll_reset(SI5351_PLLA);
//     while(millis() < time_now + period)  // Found to be more accruate than delay()
//     {}

//   }

//   si5351.output_enable(SI5351_CLK0, 0); // Turn off the output
//   if(twoChanel) si5351.output_enable(SI5351_CLK1, 0);   
//   digitalWrite(RFPIN, LOW);
// }


// void rf_on()
// {
//   digitalWrite(RFPIN, HIGH);

//   si5351.output_enable(SI5351_CLK2, 0); // Disable calibration signal
//   si5351.pll_reset(SI5351_PLLA);
//   si5351.pll_reset(SI5351_PLLB);

//   si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
//   si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
//   //uint8_t one = 1;
//   //uint8_t zero = 0;
//   //si5351.set_clock_invert(SI5351_CLK1,one);  // make one clock 180 deg out of phase from the other
//   //si5351.set_clock_invert(SI5351_CLK0,zero);

//   si5351.output_enable(SI5351_CLK0, 0);                 // Disable the clock initially
//   si5351.output_enable(SI5351_CLK1, 0); 
// }


// void rf_off()
// {
//   digitalWrite(RFPIN, LOW);
// }

void setToFrequency2()
{
  si5351.pll_reset(SI5351_PLLA);
  si5351.pll_reset(SI5351_PLLB);
  freq = (unsigned long) (WSPR_FREQ2*(correction));
  float randomChange = random(-15,15);
  freq = freq +(unsigned long) randomChange;
  POUTPUT(F(" Random Change from Band Center "));
  POUTPUTLN((randomChange));
  POUTPUT(F(" Xtal Corrections "));
  POUTPUTLN((XtalFreq));
  POUTPUT(F(" Frequency "));
  float dialFreq = WSPR_FREQ2+randomChange;
  POUTPUTLN((dialFreq));
}


void setToFrequency1()
{

    freq = (unsigned long) (WSPR_FREQ1*(correction));
    // random number to create random frequency -spread spectrum
    float randomChange = random(-15,15);
    freq = freq +(unsigned long) randomChange;  // random freq in middle 150 Hz of wspr band

      POUTPUT(F(" Random Change from Band Center "));
      POUTPUTLN((randomChange));
      POUTPUT(F(" Xtal Corrections "));
      POUTPUTLN((XtalFreq));
      POUTPUT(F(" Frequency "));
      float dialFreq = WSPR_FREQ1+randomChange;
      POUTPUTLN((dialFreq));


}
