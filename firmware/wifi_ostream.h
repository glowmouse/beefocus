#ifndef __WifiOstream_H__
#define __WifiOstream_H__

#include "simple_ostream.h"

class WifiOstream {
  public:

  struct category: public beefocus_tag {};
  using char_type = char;

  WifiOstream() : m_connectedClient{nullptr}
  {
  }

  ~WifiOstream()
  {
    m_connectedClient = nullptr;
  }

  void setConnectedClientAlias( NetConnection* connectedClient )
  {
    m_connectedClient = connectedClient;
  }

  private:
  void onechar( char c )
  {
    if ( m_connectedClient )
    {
      *m_connectedClient << c;
    }
  }

    NetConnection* m_connectedClient;
};

#endif

