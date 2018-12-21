#include <iostream>
#include <memory>
#include <unistd.h>
#include <gtest/gtest.h>

#include "focuser_state.h"
#include "hardware_interface.h"

class NetMockSimpleTimed: public NetInterface
{
  public:

  NetMockSimpleTimed( const TimedStringEvents& inputEventsArg)
    : inputEvents{inputEventsArg}, 
      actualTime{0},
      nextInputEvent{inputEvents.begin()},
      lastOutputEvent{outputEvents.end()}
  {
  }

  NetMockSimpleTimed( const char* string )
    : actualTime{0},
      lastOutputEvent{outputEvents.end()}
  {
    inputEvents.push_back( { 0, std::string( string ) } );
    nextInputEvent = inputEvents.begin();
  }

  NetMockSimpleTimed()
    : actualTime{0},
      nextInputEvent{inputEvents.begin()},
      lastOutputEvent{outputEvents.end()}
  {
  }

  void setup( DebugInterface& debugLog ) override
  {
  }

  void advanceTime( int ticks )
  {
    actualTime+=ticks;
  }

  bool getString( WifiDebugOstream& log, std::string& input ) override
  {
    if ( nextInputEvent == inputEvents.end() )
      return false;
    if ( nextInputEvent->time > actualTime )
      return false;

    input = nextInputEvent->event;
    nextInputEvent++;

    return true;
  }

  NetInterface& operator<<( char c ) override 
  {
    if ( lastOutputEvent == outputEvents.end() )
    {
      outputEvents.emplace_back(TimedStringEvent(actualTime, std::string("") ));
      lastOutputEvent = outputEvents.begin() + outputEvents.size() - 1;
    }
    if ( c == '\n' )
      ++lastOutputEvent;
    else
      lastOutputEvent->event += c;
  }

  const TimedStringEvents& getOutput() 
  {
    return outputEvents;
  }

  /// @brief Return network output without comments
  TimedStringEvents getFilteredOutput()
  {
    TimedStringEvents filteredEvents;
    for ( const auto& i: outputEvents )
    {
      if ( i.event[0] != '#' )
      {
        filteredEvents.push_back( i );
      }
    }
    return filteredEvents;
  }

  private:
  TimedStringEvents inputEvents;
  TimedStringEvents::iterator nextInputEvent;
  TimedStringEvents outputEvents;
  TimedStringEvents::iterator lastOutputEvent; 
  int actualTime;
};

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


