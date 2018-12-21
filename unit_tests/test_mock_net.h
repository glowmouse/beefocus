#ifndef __TEST_MOCK_NET_H__
#define __TEST_MOCK_NET_H__

#include "net_interface.h"
#include "test_mock_event.h"

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
    : inputEvents{ TimedStringEvents( {{ 0, std::string( string ) }}) },
      actualTime{0},
      nextInputEvent{inputEvents.begin()},
      lastOutputEvent{outputEvents.end()}
  {
  }

  NetMockSimpleTimed()
    : inputEvents{},
      actualTime{0},
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
  const TimedStringEvents inputEvents;
  TimedStringEvents::const_iterator nextInputEvent;
  TimedStringEvents outputEvents;
  TimedStringEvents::iterator lastOutputEvent; 
  int actualTime;
};

#endif

