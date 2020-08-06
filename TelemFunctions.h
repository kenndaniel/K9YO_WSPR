/*
   Telemetry functions
*/


#define DEBUG

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
{
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
  //freq = WSPR_FREQ; 
  memset(tx_buffer, 0, 165); // Clears Tx buffer from previous operation.
  POUTPUT((call));
  POUTPUT((F(" ")));
  POUTPUT((loc4));
  POUTPUT((F(" ")));
  POUTPUTLN((dbm_telemetry));
  POUTPUT((F(" ")));
  POUTPUTLN((symbol_count));
  jtencode.wspr_encode(call, loc4, dbm_telemetry, tx_buffer);
}

void setModeWSPR_telem()
{
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
  //freq = WSPR_FREQ; 
  memset(tx_buffer, 0, 165); // Clears Tx buffer from previous operation.
  POUTPUT((call_telemetry));
  POUTPUT((F(" ")));
  POUTPUT((loc4));
  POUTPUT((F(" ")));
  POUTPUTLN((dbm_telemetry));
  jtencode.wspr_encode(call_telemetry, loc4, dbm_telemetry, tx_buffer);
}



/*
   Message encoding
*/
void encode() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  digitalWrite(RFPIN, HIGH);
  si5351.output_enable(SI5351_CLK0, 1); // Reset the tone to the base frequency and turn on the output
  const unsigned long period = tone_delay;
  unsigned long time_now = 0;

  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = millis();
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0);
    while(millis() < time_now + period)  // Found to be more accruate
    {}
    //delay(tone_delay);

  }

  si5351.output_enable(SI5351_CLK0, 0); // Turn off the output
     
  digitalWrite(RFPIN, LOW);
}


void rf_on()
{
  digitalWrite(RFPIN, HIGH);
  //delay(2000);
  POUTPUT((F(" SI Init ")));
  bool siInit = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);    // TCXO 25MHz
  delay(2000);
  POUTPUT((F(" SI5355 Init Success")));
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
  si5351.output_enable(SI5351_CLK0, 0);                 // Disable the clock initially
}

void rf_off()
{
  digitalWrite(RFPIN, LOW);
}


