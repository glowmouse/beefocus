#ifndef __HARDWARE_ARDUINO_H__
#define __HARDWARE_ARDUINO_H__

#include "hardware_interface.h"

class HardwareESP8266: public HardwareInterface
{
  public:

  void DelayMicroseconds( int usecs ) override;
  void Delay( int secs ) override;
  void DigitalWrite( int pin, int state ) override;
  void PinMode( int pin, int state ) override;
  int  DigitalRead( int pin) override;
};

#endif

