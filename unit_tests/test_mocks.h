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


class DebugInterfaceIgnoreMock: public DebugInterface
{
  public:
  void rawWrite( const char* bytes, std::size_t numBytes ) override
  {
    // Do Nothing;
  }
};



