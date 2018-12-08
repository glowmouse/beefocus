#include <ESP8266WiFi.h>
#include "hardware_esp8266.h"

int HardwareESP8266::mapPins( Pin pin )
{
  switch( pin )
  {
    case Pin::STEP:
      return 4;
    case Pin::DIR:
      return 5;
    case Pin::MOTOR_ENA:
      return 14; 
    case Pin::HOME:
      return 13; 
    case Pin::END_OF_PINS:
      return -1;    // oh oh. todo, how to handle
  }
  return -1;    // oh oh, todo, how to handle
}

void HardwareESP8266::DigitalWrite( Pin pin, int state )
{
  digitalWrite( mapPins(pin), state==low ? LOW : HIGH );
}

void HardwareESP8266::PinMode( Pin pin, int state )
{
  pinMode( mapPins(pin), state == output ? OUTPUT : INPUT );
}

int HardwareESP8266::DigitalRead( Pin pin)
{
  return digitalRead( mapPins(pin) );
}

