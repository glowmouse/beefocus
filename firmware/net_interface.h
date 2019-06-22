#ifndef __NetInterface_H__
#define __NetInterface_H__

#include <string>
#include "hardware_interface.h"
#include "debug_interface.h"

class WifiOstream;
class WifiDebugOstream;

/// @brief Interface to the client
///
/// This class's one job is to provide an interface to the client.
///
class NetInterface {
  public:

  struct category : public beefocus_tag {};
  using char_type = char;

  NetInterface()
  {
  }
  virtual ~NetInterface()
  {
  }

  virtual void setup( DebugInterface &debugLog ) = 0;

  virtual bool getString( WifiDebugOstream &log, std::string& string ) = 0;
  virtual std::streamsize write( const char_type* s, std::streamsize n ) = 0;

  private:
};

class NetConnection {
  public:

  struct category : public beefocus_tag {};
  using char_type = char;

  virtual bool getString( WifiDebugOstream& lot, std::string& string )=0;
  virtual operator bool( void ) = 0;
  virtual void reset( void ) = 0;
  virtual std::streamsize write( const char_type* s, std::streamsize n ) = 0;

  protected:


};

#endif

