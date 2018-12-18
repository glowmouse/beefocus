#include <iostream>
#include <memory>
#include <unistd.h>
#include <gtest/gtest.h>

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

template< class Event >
std::ostream& operator<<(std::ostream& stream, const TimedEvent<Event>& timedEvent ) 
{
  stream << "Time: " << timedEvent.time << " " << timedEvent.event << "\n";
  return stream;
}

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


class DebugInterfaceIgnoreMock: public DebugInterface
{
  public:
  void rawWrite( const char* bytes, std::size_t numBytes ) override
  {
    // Do Nothing;
  }
};

class HWEvent
{
  public:

  enum class Type
  {
    DIGITAL_IO,
    PIN_MODE,
  };

  HWEvent( HWI::Pin pinRHS, HWI::PinState stateRHS ) :
    pin{ pinRHS },
    state{ stateRHS },
    type{ Type::DIGITAL_IO }
  {
  }
 
  HWEvent( HWI::Pin pinRHS, HWI::PinIOMode modeRHS) :
    pin{ pinRHS },
    mode{ modeRHS },
    type{ Type::PIN_MODE }
  {
  }
 
  bool operator==( const HWEvent& rhs ) const 
  {
    return ( pin == rhs.pin ) &&
      type == Type::DIGITAL_IO ? ( state == rhs.state ) :
        ( mode == rhs.mode ); 
  }

  HWI::Pin pin;
  Type type;
  union {
    HWI::PinState state;
    HWI::PinIOMode mode;
  };
};

inline std::ostream& operator<<(std::ostream& stream, const HWEvent& event) 
{
  stream << "{ PIN: " << HWI::pinNames.at( event.pin );
  if ( event.type ==  HWEvent::Type::DIGITAL_IO )
  {
    stream << " IO:    " << HWI::pinStateNames.at(event.state); 
  }
  else
  {
    stream << " MODE:  " << HWI::pinIOModeNames.at(event.mode); 
  }
  stream << " }";
  return stream;
}

using HWTimedEvent = TimedEvent<HWEvent>; 
using HWTimedEvents = std::vector<HWTimedEvent>;

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
      assert( event.type == HWEvent::Type::DIGITAL_IO );
      inputStates[ event.pin ] = event.state;
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

  std::unordered_map<Pin,PinState> inputStates;
};


