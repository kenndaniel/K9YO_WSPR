
int lastSensorReading = 0;

int readSensor()
{ 	// sensorValue is between 0 and 1024
	// This is called after the standard message is sent.
	// Note - If this routine takes more than 3 sec, the telemetry message will be delayed 2 minutes
	// Use this functin to call one or more sensors
	unsigned int sensorValue = analogRead(SENSOR_PIN);
	delay(500);
    sensorValue += analogRead(SENSOR_PIN);
	delay(500);
    sensorValue += analogRead(SENSOR_PIN);
	delay(500);
    sensorValue += analogRead(SENSOR_PIN);
	delay(500);
    sensorValue += analogRead(SENSOR_PIN);
    sensorValue = ((float)sensorValue / 5.) ;
    
	lastSensorReading = sensorValue;
	return sensorValue;
}


int getSensorValue()
{   // get the value when converting data to callsign in CodeTelemetryMessage.h
	return lastSensorReading;
}