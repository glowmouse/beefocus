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

  std::streamsize write( const char_type* s, std::streamsize n ) override;
  void disable() override; 

private:
  bool isDisabled = false;
};

#endif

