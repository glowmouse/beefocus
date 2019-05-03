#ifndef __TEST_MOCK_EVENTS__
#define __TEST_MOCK_EVENTS__

#include <iostream>
#include <gtest/gtest.h>

#include "hardware_interface.h"

/// @brief a Hardware Event
///
/// Hardware events present a change of some kind in to the hardware 
/// interface.  They're used to either mock events in a test,  like the 
/// home switch activating,  or to describe events that we expec to 
/// occur,  like the stepper motor pin being asked to go in reverse.
///
/// Examples:
///
/// Event   : HWEvent( HWI::Pin::DIR,   HWI::PinState::Dir_FIRWARD )
/// Meaning : The stepper motor direction pin is set to "go forward"
/// 
/// Event   : HWEvent( HWI::Pin::DIR,   HWI::PinIOMode::M_OUTPUT )
/// Meaning : The stepper motor direction GPIO is set to output mode
/// 
/// Event   : HWEvent( HWI::Pin::HOME,  HWI::PinIOMode::M_INPUT )
/// Meaning : The stepper motor home GPIO is set to input mode
/// 
/// Event   : HWEvent( HWI::Pin::HOME,  HWI::PinState::HOME_ACTIVE )
/// Meaning : The home pin's input is now active (i.e., the focuser
///           is in the home position & the home switch was activated).
///
class HWEvent
{
  public:

  ///
  /// @brief Default Constructor (deleted)
  ///
  HWEvent() = delete;

  ///
  /// @brief Constructor for Pin IO Read or Write event
  ///
  /// @param[in]  The pin that the read or write takes place on
  /// @param[in]  The new pin state.
  /// 
  HWEvent( HWI::Pin pinRHS, HWI::PinState stateRHS ) :
    pin{ pinRHS },
    type{ Type::DIGITAL_IO },
    state{ stateRHS }
  {
  }
 
  /// 
  /// @brief Constructor for setting a pin's GPIO to an input or output
  ///
  /// @param[in]  The pin that we're setting the GPIO mode on
  /// @param[in]  The new mode (i.e.,  output or input )
  /// 
  HWEvent( HWI::Pin pinRHS, HWI::PinIOMode modeRHS) :
    pin{ pinRHS },
    type{ Type::PIN_MODE },
    mode{ modeRHS }
  {
  }

  /// @brief Equality operator
  ///
  /// @param[in] rhs =  The other event to compare to
  ///
  bool operator==( const HWEvent& rhs ) const 
  {
    return ( pin == rhs.pin ) &&
      type == Type::DIGITAL_IO ? ( state == rhs.state ) :
        ( mode == rhs.mode ); 
  }

  /// @brief Is the event an IO read or write event
  bool isIO() const { return type == Type::DIGITAL_IO; }

  /// @brief Is the event a set GPIO mode to Input or Output event.
  bool isMode() const { return type == Type::PIN_MODE ; }

  /// @brief Get the new state for an IO read or write event
  HWI::PinState getIO() const
  {
    assert( isIO() );
    return state;
  }

  /// @brief Get the new mode for a set GPIO mode event.
  HWI::PinIOMode getMode() const
  {
    assert( isMode() );
    return mode;
  }
 
  /// @brief Get the pin
  HWI::Pin getPin() const { return pin; }

  private:

  enum class Type
  {
    DIGITAL_IO,   // A GPIO Read or Write event
    PIN_MODE,     // An event where the GPIO is set to Input or Output
  };

  HWI::Pin pin;
  Type type;

  union {
    HWI::PinState state;
    HWI::PinIOMode mode;
  };
};

///
/// @brief Output stream operator for Hardware Events 
///
/// Used by gtest to output useful information when a test fails.
/// 
/// @param[out] stream     The stream to outputting to
/// @param[in] event       The hardware event being outputting.
///
inline std::ostream& operator<<(
  std::ostream& stream, 
  const HWEvent& event) 
{
  stream << "{ PIN: " << HWI::pinNames.at( event.getPin() );
  if ( event.isIO() )
  {
    stream << " IO:    " << HWI::pinStateNames.at(event.getIO() ); 
  }
  else
  {
    stream << " MODE:  " << HWI::pinIOModeNames.at(event.getMode() ); 
  }
  stream << " }";
  return stream;
}


/// @brief A Timed Event of some kind for unit testing
///
/// 
template< class Event > class TimedEvent
{
  public:

  /// @brief Default Constructor (deleted)
  //
  TimedEvent() = delete;

  /// @brief Constructor
  ///
  /// @param[in] timeRHS   The time the event occurs at (ms)
  /// @param[in] eventRHS  The nature of the event.
  ///
  TimedEvent( int timeRHS, const Event& eventRHS ) :
    time{ timeRHS }, event{ eventRHS }
  {
  }

  /// @brief Equality Operator
  ///
  /// @param[in] rhs      The other timed event to compare to
  ///
  bool operator==( const TimedEvent& rhs ) const 
  {
    return time == rhs.time && event == rhs.event;
  }

  /// @brief The time the event occurs at (ms)
  int time;

  /// @brief The nature of the event
  Event event;
};

/// @brief std::ostream << operator for a timed event
///
/// Used by gtest to output useful information when a test fails.
/// 
/// @param[out] stream     The stream to outputting to
/// @param[in] event       The hardware event being outputting.
///
template< class Event > std::ostream& operator<<(
  std::ostream& stream, 
  const TimedEvent<Event>& timedEvent ) 
{
  stream << "Time: " << timedEvent.time << " " << timedEvent.event << "\n";
  return stream;
}

///
/// @brief A timed string event (i.e., network input or output)
///
using TimedStringEvent = TimedEvent<std::string>;

///
/// @brief A vector of timed string events
///
using TimedStringEvents = std::vector<TimedStringEvent>;
 
///
/// @brief A timed hardware event (i.e., a state change on a pin) 
///
using HWTimedEvent = TimedEvent<HWEvent>;
 
///
/// @brief A vector of timed hardware events
///
using HWTimedEvents = std::vector<HWTimedEvent>;

#endif

