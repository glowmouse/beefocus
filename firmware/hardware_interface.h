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

  enum class PinState {
    START_OF_PIN_STATES = 0,
    STEP_HIGH = 0,
    STEP_LOW,
    DIR_FORWARD,
    DIR_BACKWARD,
    MOTOR_ON,       // 0 on the nema build
    MOTOR_OFF,      // 1 on the nema build
    HOME_ACTIVE,    // 0 on the nema build
    HOME_INACTIVE,  // 1 on the nema build
    END_OF_PIN_STATES
  };

  const static std::unordered_map<Pin,std::string,EnumHash> pinNames;
  const static std::unordered_map<PinState,std::string,EnumHash> pinStateNames;

  virtual void DigitalWrite( Pin pin, PinState state ) = 0;
  virtual void PinMode( Pin pin, int mode ) = 0;
  virtual PinState DigitalRead( Pin pin) = 0;

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

// @brief Increment operator for Hardware Interface Pin State
//
// @param[in] pinState - The pin state to increment. 
// @return - The next value in the Enum.
// 
// Todo - Template? Better pattern.
//
inline HWI::PinState& operator++( HWI::PinState &pinState ) 
{
  if ( pinState != HWI::PinState::END_OF_PIN_STATES )
  {
    // still wee hacky
    pinState = static_cast<HWI::PinState>( static_cast<int>(pinState) + 1 );
  }
  return pinState;
}

#endif

