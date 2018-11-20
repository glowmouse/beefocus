#include <ESP8266WiFi.h>
#include "hardware_esp8266.h"

void HardwareESP8266::DelayMicroseconds( int usecs ) 
{
  delayMicroseconds( usecs );
}

void HardwareESP8266::Delay( int secs )
{
  delay( secs );
}

void HardwareESP8266::DigitalWrite( int pin, int state )
{
  digitalWrite( pin, state==low ? LOW : HIGH );
}

void HardwareESP8266::PinMode( int pin, int state )
{
  pinMode( pin, state == output ? OUTPUT : INPUT );
}

int HardwareESP8266::DigitalRead( int pin)
{
  return digitalRead( pin );
}

