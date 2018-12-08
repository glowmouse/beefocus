#ifndef __HARDWARE_ARDUINO_H__
#define __HARDWARE_ARDUINO_H__

#include "hardware_interface.h"

class HardwareESP8266: public HardwareInterface
{
  public:

  void DigitalWrite( Pin pin, int state ) override;
  void PinMode( Pin pin, int state ) override;
  int  DigitalRead( Pin pin) override;

  private:
  
  // @brief map generic pins to build specific pins
  int mapPins( Pin pin);
};

#endif

