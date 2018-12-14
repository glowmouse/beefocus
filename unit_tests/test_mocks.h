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

class NetMockSimpleTimed: public NetInterfaceMockBase
{
  public:

  using TimedString = std::pair<unsigned int, std::string>;
  using TimedStrings = std::vector<TimedString>;

  NetMockSimpleTimed( const TimedStrings& inputEventsArg)
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
    if ( nextInputEvent->first > actualTime )
      return false;

    input = nextInputEvent->second;
    nextInputEvent++;

    return true;
  }

  NetInterface& operator<<( char c ) override 
  {
    if ( lastOutputEvent == outputEvents.end() )
    {
      outputEvents.emplace_back(TimedString(actualTime, "" ));
      lastOutputEvent = outputEvents.begin() + outputEvents.size() - 1;
    }
    if ( c == '\n' )
      ++lastOutputEvent;
    else
      lastOutputEvent->second += c;
  }

  const TimedStrings& getOutput() 
  {
    return outputEvents;
  }

  private:
  TimedStrings inputEvents;
  TimedStrings::iterator nextInputEvent;
  TimedStrings outputEvents;
  TimedStrings::iterator lastOutputEvent; 
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



