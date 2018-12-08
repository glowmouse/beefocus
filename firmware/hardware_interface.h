#ifndef __HARDWARE_INTERFACE_H__
#define __HARDWARE_INTERFACE_H__

class HardwareInterface
{
  public:

  virtual void DigitalWrite( int pin, int state ) = 0;
  virtual void PinMode( int pin, int state ) = 0;
  virtual int  DigitalRead( int pin) = 0;

  constexpr static int low = 0;
  constexpr static int high = 1;
  constexpr static int output = 0;
  constexpr static int input = 1;
};

#endif

