#include <ESP8266WiFi.h>
#include "debug_esp8266.h"

DebugESP8266::DebugESP8266()
{
	Serial.begin( 115200 );
}

void DebugESP8266::disable()
{
  isDisabled = true;
}

std::streamsize DebugESP8266::write( const char_type* s, std::streamsize n )
{
  if ( !isDisabled ) { Serial.write( (uint8 *) s, n ); }
  return n;
}

