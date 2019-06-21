#ifndef __DEBUG_INTERFACE_H__
#define __DEBUG_INTERFACE_H__

#include "simple_ostream.h"

class DebugInterface
{
	public:

  struct category: virtual beefocus_tag {};
  using char_type = char;

  virtual void rawWrite( const char *bytes, std::size_t numBytes ) = 0;
  virtual std::streamsize write( const char_type* s, std::streamsize n ) = 0;

  virtual ~DebugInterface() {}
};

inline DebugInterface& operator<<( DebugInterface& stream, char c )
{
	stream.rawWrite( &c, 1 );
  return stream;
}

inline void rawWrite( DebugInterface& stream, const char *bytes, std::size_t numBytes )
{
	stream.rawWrite( bytes, numBytes );
}

#endif

