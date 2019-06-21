#ifndef __DEBUG_ESP8266_H__
#define __DEBUG_ESP8266_H__

#include "debug_interface.h"

class DebugESP8266: public DebugInterface
{
	public:

  struct category: beefocus_tag {};
  using char_type = char;

	DebugESP8266();
	DebugESP8266( DebugESP8266& other ) = delete;

	void rawWrite( const char *bytes, std::size_t numByes ) override;

  std::streamsize write( const char_type* s, std::streamsize n ) override
  {
    for ( std::streamsize i = 0; i < n; ++i )
    {
      (*this) << s[i];
    }
    return n;
  }

};

#endif

