
#define RADIATION_MEASURE_POINT 200 // determines mid morning and mid afternoon

int lastRadiationReading = 0;

int readRadiation()
{ 	// sensorValue is between 0 and 1024
	unsigned int sensorValue = analogRead(RADIATION_PIN);
	delay(500);
    sensorValue += analogRead(RADIATION_PIN);
	delay(250);
    sensorValue += analogRead(RADIATION_PIN);
	delay(250);
    sensorValue += analogRead(RADIATION_PIN);
	delay(500);
    sensorValue += analogRead(RADIATION_PIN);

	// scale sensor value
    sensorValue = ((float)sensorValue / 5.) ;
	lastRadiationReading = sensorValue;
	return sensorValue;
}


int getRadiation()
{
	return lastRadiationReading;
}


float getBatteryVolt()
{ 
    return 3.8;// not used
}

float getTempCPU()
{  // processor internal temperature
    int wADC;

    ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
    ADCSRA |= _BV(ADEN);
    delay(20);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    wADC = ADCW;
    float temp = (wADC - 322.2) / 1.43;
    return temp;
}

long readVcc() 
{  //Read voltage 5000 = 5v
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
