#ifndef __HARDWARE_INTERFACE_H__
#define __HARDWARE_INTERFACE_H__

#include <unordered_map>
#include <string>

struct EnumHash
{
  // @brief Type convert any enum type to a size_t for hashing
  template <class T>
  std::size_t operator()(T enum_val) const
  {
    return static_cast<std::size_t>(enum_val);
  }
};

class HWI
{
  public:

  enum class Pin {
    START_OF_PINS = 0,
    STEP = 0,
    DIR,
    MOTOR_ENA,
    HOME,
    END_OF_PINS 
  };

  const static std::unordered_map<Pin,std::string,EnumHash> pinNames;

  virtual void DigitalWrite( Pin pin, int state ) = 0;
  virtual void PinMode( Pin pin, int state ) = 0;
  virtual int  DigitalRead( Pin pin) = 0;

  constexpr static int low = 0;
  constexpr static int high = 1;
  constexpr static int output = 0;
  constexpr static int input = 1;
};

// @brief Increment operator for Hardware Interface Pin
//
// @param[in] pin - The pin to increment. 
// @return - The next value in the Enum.
//
inline HWI::Pin& operator++( HWI::Pin &pin ) 
{
  if ( pin != HWI::Pin::END_OF_PINS )
  {
    // wee hacky
    pin = static_cast<HWI::Pin>( static_cast<int>(pin) + 1 );
  }
  return pin;
}

#endif

