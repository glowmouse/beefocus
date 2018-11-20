#ifndef __SIMPLE_OSTREAM__
#define __SIMPLE_OSTREAM__

#include <cstddef>  // for std::size_t
#include <string.h>
#include <string>
#include "basic_types.h"  // for BeeFocus::IpAddress.

/// @brief Output a buffer
template <class T>
void rawWrite( T& ostream,  const char *bytes, std::size_t numBytes )
{
   for ( int i = 0; i < numBytes; ++i )
   {
      ostream << bytes[i];
   }
}

/// @brief Output a WIFI IP address
template <class T>
T& operator<<( T& ostream, const BeeFocus::IpAddress& address )
{
  ostream << address[0] << "." << address[1] << "." << address[2] << "." << address[3];
  return ostream;
}

/// @brief Output a C style string.
template <class T>
T& operator<<( T& ostream, const char* string )
{
  std::size_t length = strlen( string );
  rawWrite( ostream, string, length );
  return ostream;
} 

/// @brief Output an unsigned number of a SIMPLE_ISTREAM.
template <class T>
T& operator<<( T& ostream, unsigned int i )
{
  // Handle digits that aren't the lowest digit (if any)
  if ( i >= 10 )
  {
    ostream << i/10;
  }

  // Handle the lowest digit
  ostream << (char) ('0' + (i % 10));
  return ostream;
}


/// @brief Output an signed number of a SIMPLE_ISTREAM.
template<class T>
T& operator<<( T& ostream, int i )
{
  // Handle -
  if ( i < 0 )
  {
    ostream << "-";   
    i = -i;
  }

  ostream << (unsigned int) i;
  return ostream;
}  

/// @brief Output an std::string
template<class T>
T& operator<<( T& ostream, const std::string& string )
{
  ostream << string.c_str();
  return ostream;
}

#endif

