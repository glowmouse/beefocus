#include <iostream>
#include <memory>
#include <unistd.h>
#include <gtest/gtest.h>

#include "focuser_state.h"
#include "hardware_interface.h"

class HWMockTimed: public HWI
{
  public:

  HWMockTimed( const HWTimedEvents& hwIn ) : 
      time{ 0 }, 
      inEvents{ hwIn },
      nextInputEvent{inEvents.begin()}
  {
    advanceTime(0);
  }
  HWMockTimed() = delete;
  HWMockTimed( const HWMockTimed& ) = delete;
    
  void DigitalWrite( Pin pin, PinState state ) override
  {
    outEvents.emplace_back( HWTimedEvent( time, HWEvent( pin, state ))); 
  }

  void PinMode( Pin pin, PinIOMode mode ) override
  {
    outEvents.emplace_back( HWTimedEvent( time, HWEvent( pin, mode ))); 
  }

  void advanceTime( int ticks )
  {
    time+=ticks;

    while ( nextInputEvent != inEvents.end() && 
            nextInputEvent->time <= time )
    {
      const HWEvent& event = nextInputEvent->event;
      inputStates[ event.getPin() ] = event.getIO();
      ++nextInputEvent;
    }
  }

  PinState DigitalRead( Pin pin ) override
  {
    assert( inputStates.find( pin ) != inputStates.end() );
    return inputStates.at( pin );
  }

  const HWTimedEvents& getOutEvents() const { return outEvents; } 

  private:

  int time;
  HWTimedEvents outEvents;
  HWTimedEvents inEvents;
  HWTimedEvents::iterator nextInputEvent;

  std::unordered_map<Pin,PinState,EnumHash> inputStates;
};


