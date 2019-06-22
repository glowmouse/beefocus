#ifndef __WIFI_DEBUG_OSTREAM__
#define __WIFI_DEBUG_OSTREAM__

#include "simple_ostream.h"
#include "net_interface.h"
#include "debug_interface.h"

/// @brief Wifi target debug ostream
///
class WifiDebugOstream	
{
  public:

  struct category: beefocus_tag {};
  using char_type = char;

  WifiDebugOstream( DebugInterface* serialDebugArg, NetInterface* wifiDebugArg  )
    : m_wifiDebug{ wifiDebugArg }, 
      m_serialDebug{ serialDebugArg},
      m_lastWasNewline{ true }
  {
  }

  std::streamsize write( const char_type* s, std::streamsize n )
  {
    for ( std::streamsize i = 0; i < n; ++i )
    {
      onechar( s[i] );
    }
    return n;
  }

  private:

  void onechar(char c )
  { 
    m_serialDebug->write( &c, 1 );
    bool isNewLine = ( c == '\n' );
    if ( m_lastWasNewline && !isNewLine )
    {
      (*m_wifiDebug) << "# ";
    }
    m_wifiDebug->write( &c, 1 );
    m_lastWasNewline = isNewLine;
  }


  NetInterface* m_wifiDebug;
  DebugInterface* m_serialDebug;
  bool m_lastWasNewline;
};

#endif

