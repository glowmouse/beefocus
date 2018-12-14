#include <iostream>
#include <memory>
#include <unistd.h>

#include "focuser_state.h"
#include "hardware_interface.h"

class NetInterfaceMockBase: public NetInterface 
{
  public:

  void reset() override
  {
  }
  void setup( DebugInterface& debugLog ) override
  {
  }
  virtual bool getString( WifiDebugOstream& log, std::string& input ) override = 0;
  virtual NetInterface& operator<<( char c ) override = 0;
};

class NetMockEmpty: public NetInterfaceMockBase
{
  public:

  bool getString( WifiDebugOstream& log, std::string& input ) override
  {
    return false;
  }
  NetInterface& operator<<( char c ) override 
  {
  }
};

class NetMockSimple: public NetInterfaceMockBase
{
  public:

  NetMockSimple( const char* string ): payload{ std::string(string) }
  {
  }

  bool getString( WifiDebugOstream& log, std::string& input ) override
  {
    input = payload;
    return true;
  }
  NetInterface& operator<<( char c ) override 
  {
  }

  private:
  std::string payload;
};

template< class Event >
using OptionalEvent = std::pair<bool, Event>;

template< class Event > class TimedEvent
{
  public:

  using EventType = Event;

  TimedEvent() : time{0}
  {
  }
  TimedEvent( int timeRHS, const Event& eventRHS ) :
    time{ timeRHS }, event{ eventRHS }
  {
  }
  bool operator==( const TimedEvent& rhs ) const 
  { 
    return time==rhs.time && event==rhs.event;
  }

  int time;
  Event event;
};

class NetMockSimpleTimed: public NetInterfaceMockBase
{
  public:

  using TimedStringEvent = TimedEvent<std::string>;
  using TimedStringEvents = std::vector<TimedStringEvent>; 

  NetMockSimpleTimed( const TimedStringEvents& inputEventsArg)
    : inputEvents{inputEventsArg}, 
      actualTime{0},
      nextInputEvent{inputEvents.begin()},
      lastOutputEvent{outputEvents.end()}
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

  private:
  TimedStringEvents inputEvents;
  TimedStringEvents::iterator nextInputEvent;
  TimedStringEvents outputEvents;
  TimedStringEvents::iterator lastOutputEvent; 
  int actualTime;
};


class DebugInterfaceIgnoreMock: public DebugInterface
{
  public:
  void rawWrite( const char* bytes, std::size_t numBytes ) override
  {
    // Do Nothing;
  }
};



