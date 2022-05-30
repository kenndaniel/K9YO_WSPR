/*
   Functions that interface with the SI5351 Transmitter
*/
/*
   Mode defines
*/

#define JT9_TONE_SPACING        174           // ~1.74 Hz
#define WSPR_TONE_SPACING       146           // ~1.46 Hz

#define JT9_DELAY               576          // Delay value for JT9
#define WSPR_DELAY              683          // Delay value for WSPR

#define JT9_FREQ        14000000UL

void setModeJT9_1()
{
  
  symbol_count = JT9_SYMBOL_COUNT;
  tone_spacing = JT9_TONE_SPACING;
  tone_delay = JT9_DELAY;
  memset(tx_buffer, 0, symbol_count); // Clears Tx buffer from previous operation.
  jtencode.jt9_encode(message1, tx_buffer);
}

void setModeJT9_2()
{
  
  symbol_count = JT9_SYMBOL_COUNT;
  tone_spacing = JT9_TONE_SPACING;
  tone_delay = JT9_DELAY;
  memset(tx_buffer, 0, symbol_count); // Clears Tx buffer from previous operation.
  jtencode.jt9_encode(message2, tx_buffer);
}

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

/*
   Message encoding
*/
void encode() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  digitalWrite(RFPIN, HIGH);
  si5351.output_enable(SI5351_CLK1, 1); // Reset the tone to the base frequency and turn on the output
  const unsigned long period = tone_delay;
  unsigned long time_now = 0;

  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = millis();
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK1);
    while(millis() < time_now + period)  // Found to be more accruate
    {}
    //delay(tone_delay);

  }

  si5351.output_enable(SI5351_CLK1, 0); // Turn off the output
     
  digitalWrite(RFPIN, LOW);
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
  si5351.set_freq(calfreq*100, SI5351_CLK2);  // set calibration count frequency to 2.5 MHz
  si5351.output_enable(SI5351_CLK2, 1);   // Enable output  
}


void rf_on(int32_t freqCorrection)
{
  // freqCorrection correction in ppb ( differece in Hz time 10)
  digitalWrite(RFPIN, HIGH);
  si5351.set_correction(freqCorrection, SI5351_PLL_INPUT_XO);
  si5351.output_enable(SI5351_CLK2, 0); // Disable calibration signal
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
  si5351.output_enable(SI5351_CLK1, 0);                 // Disable the clock initially
}



void rf_off()
{
  digitalWrite(RFPIN, LOW);
}


