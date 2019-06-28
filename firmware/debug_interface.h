#ifndef __DEBUG_INTERFACE_H__
#define __DEBUG_INTERFACE_H__

#include "simple_ostream.h"

class DebugInterface
{
	public:

  struct category: virtual beefocus_tag {};
  using char_type = char;

  virtual std::streamsize write( const char_type* s, std::streamsize n ) = 0;
  virtual void disable() = 0;

  virtual ~DebugInterface() {}
};

#endif

