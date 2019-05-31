///
/// @brief Testing Mock for hardware events
/// 

#ifndef __TEST_MOCK_HARDWARE__
#define __TEST_MOCK_HARDWARE__

#include "hardware_interface.h"
#include "test_mock_event.h"

///
/// @brief Testing Mock for hardware events
/// 
/// HWMockTimed implements a mock Hardware Interface (HWI) that's used in
/// unit testing.  The class implements testing mocks for the interfaces
/// required by the HWI class.  In addition to that, the class does the
/// following:
///
/// - Maintain Time.  
///     The class simulates the passage of time.  advanceTime is called to 
///     "move" time forward
/// - Record Output.  
///     Whenever an output pin is changed on the hardware mock the event and
///     event time are recorded.  Tests can use this to verify that the 
///     output matches a golden result.
/// - Simulate Input.
///     On class construction, the caller can specify a series of input
///     events and the time those events occur at.  i.e.,  the caller can
///     see "at time 20ms the HOME input will change state to active"
/// 
class HWMockTimed: public HWI
{
  public:

  /// @brief Class Constructor 
  ///
  /// @param[in] HWTimedEvents - Simulated Input Events.  A vector of
  ///   input events and they time they take place at.  See 
  ///   HWTimedEvents for examples. 
  ///
  HWMockTimed( const HWTimedEvents& hwIn ) : 
      time{ 0 }, 
      inEvents{ hwIn },
      nextInputEvent{inEvents.begin()}
  {
    // Advance time by 0.  This causes any input events at "time 0"
    // to be processed and recorded in the inputStates map.
    advanceTime(0);
  }

  // delete unused operators for safety
  HWMockTimed() = delete;
  HWMockTimed( const HWMockTimed& ) = delete;
  HWMockTimed& operator=( const HWMockTimed& ) = delete;
  virtual ~HWMockTimed() {}

  ///
  /// @brief Mock DigitalWrite hardware interface
  ///
  /// @param[in] pin      - The pin being written to
  /// @param[in] pinState - The pin's new state
  ///
  /// Records the event and time for golden result comparison.
  /// 
  void DigitalWrite( Pin pin, PinState state ) override
  {
    outEvents.emplace_back( HWTimedEvent( time, HWEvent( pin, state ))); 
  }

  ///
  /// @brief Mock PinMode hardware interface
  ///
  /// @param[in] pin  - The pin being written to
  /// @param[in] mode - The pin's new mode (i.e., output or input)
  ///
  /// Records the event and time for golden result comparison.
  /// 
  void PinMode( Pin pin, PinIOMode mode ) override
  {
    outEvents.emplace_back( HWTimedEvent( time, HWEvent( pin, mode ))); 
  }

  ///
  /// @brief Mock DigitalRead hardware interface
  ///
  /// @param[in] pin  - The pin being read from
  /// @return         - The pin's current state
  ///
  /// Returns the current state of the pin.  Any pin used by this interface
  /// has to have it's state defined at class construction time.  i.e.
  ///
  /// @code
  ///   HWTimedEvents hwInput= {
  ///     { 0,   { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  ///     { 10,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  ///   };
  /// 
  ///   HWMockTimed hw( hwInput ) : 
  /// 
  ///   hw.DigitalRead( HWI::Pin::HOME )  // Will return HOME_INACTIVE
  ///   hw.advanceTime( 5 )               // Time = 5
  ///   hw.DigitalRead( HWI::Pin::HOME )  // Will still return HOME_INACTIVE
  ///   hw.advanceTime( 5 )               // Time = 10
  ///   hw.DigitalRead( HWI::Pin::HOME )  // Will now return HOME_ACTIVE
  /// @endcode
  ///
  PinState DigitalRead( Pin pin ) override
  {
    assert( inputStates.find( pin ) != inputStates.end() );
    return inputStates.at( pin );
  }

  ///
  /// @brief Advance simulated time
  ///
  /// param[in] ticks - Time to advance in ms
  /// 
  /// Does the following:
  /// 
  /// 1. Advance the official time of the hardware mock by tick ms.
  /// 2. Process all input events up to the new time. 
  /// 
  void advanceTime( int ticks )
  {
    // 1. Advances the official time of the hardware mock by tick ms.
    time+=ticks;

    // 2. Process all input events up to the new time. 
    while ( nextInputEvent != inEvents.end() && 
            nextInputEvent->time <= time )
    {
      const HWEvent& event = nextInputEvent->event;
      inputStates[ event.getPin() ] = event.getIO();
      ++nextInputEvent;
    }
  }

  ///
  /// @brief  Get output events for golden result comparison
  ///
  /// @return All output events that were recorded on the hardware
  ///
  /// Example:
  ///
  /// @code
  ///   HWTimedEvents hwInput;  // No input
  ///   HWMockTimed hw( hwInput ) : 
  ///   // Set Stepper Pin to Output
  ///   hw.PinMode( HWI::Pin::STEP,   HWI::PinIOMode::M_OUTPUT );
  ///   hw.PinMode( HWI::Pin::STEP,   HWI::PinState::STEP_INACTIVE);
  ///
  ///   // Do a step
  ///   hw.advanceTime( 10 )
  ///   hw.PinMode( HWI::Pin::STEP,   HWI::PinState::STEP_ACTIVE);
  ///   hw.advanceTime( 1 )
  ///   hw.PinMode( HWI::Pin::STEP,   HWI::PinState::STEP_INACTIVE);
  /// 
  ///   // Golden Results
  ///   HWTimedEvents goldenHW = {
  ///     { 0,  {HWI::Pin::Step,   HWI::PinIOMode::M_OUTPUT      } },
  ///     { 0,  {HWI::Pin::Step,   HWI::PinState::STEP_INACTIVE  } },
  ///     { 10, {HWI::Pin::Step,   HWI::PinState::STEP_ACTIVE } },
  ///     { 11, {HWI::Pin::Step,   HWI::PinState::STEP_ACTIVE } }
  ///   }
  ///   // Compare
  ///   ASSERT_EQ( goldenHw, hw.getOutEvents()); 
  /// @endcode
  ///
  const HWTimedEvents& getOutEvents() const { 
    return outEvents; 
  } 

  private:

  /// @brief  Current tIme
  int time;
  /// @brief  Recorded output events
  HWTimedEvents outEvents;
  /// @brief  Input events to be sent back to the caller
  const HWTimedEvents inEvents;
  /// @brief  Next input event that needs to be processed.
  HWTimedEvents::const_iterator nextInputEvent;
  /// @brief  The current state of each input pin.
  std::unordered_map<Pin,PinState,EnumHash> inputStates;
};

#endif

