#ifndef __SIMPLE_OSTREAM__
#define __SIMPLE_OSTREAM__

#include <cstddef>  // for std::size_t
#include <string.h>
#include <string>
#include <ios>      // for std::streamsize
#include <type_traits>
#include "basic_types.h"  // for BeeFocus::IpAddress.

//
// Like std::enable_if_t, but works in C++ 11.
//
// B = True  : std::enable_if<B,T>::type is defined to T
// B = False : std::enable_if<B,T>::type is undefined
//
// B = True  : typename my_enable_if_t<B,T> is T
// B = False : typename my_enable_if_t<B,T> is undefined
//
template< bool B, class T = void >
using my_enable_if_t = typename std::enable_if<B,T>::type;

//
// Create a tag for my IO sinks.  I'll be using (roughly) the boost
// sink model.
// 
struct beefocus_tag {};

// Test a class T to see if it has a tag (i.e., does T inherit beefocus_tag)
template< class T >
struct has_beefocus_trait: std::is_convertible< T, beefocus_tag > {};

template< class T >
using inttype_if_beefocus = my_enable_if_t<has_beefocus_trait<T>::value, int >;

// Is this a beefocus sink?  Default to false.
template< class T, class U = int>
struct is_beefocus_sink: std::false_type {};

//
// Specialization to detect the beefocus sink trait...
//
// 1. The specialization that succeeds is
//    struct is_beefocus_sink< T, int > : std::true_type{}
// 2. For that to succeed, decltype( ... ) has to return an integer type
// 3. inttype_if_beefocus<typename T::category> will evaluate to into if 
//    T::category has the beefocus_trait.  If T::category isn't a 
//    beefocus_trait no type is returned.
// 3. inttype_if_beefocus<typename T::category>{0} is 0 if 
//    inttype_if_beefocus<typename T::category>{0} is an int type
// 4. decltype( 0 ) gets us an int type (or close enough).
//
template< class T >
struct is_beefocus_sink< T, decltype(
  inttype_if_beefocus<typename T::category>{0}) > : std::true_type {};

/// @brief Output a WIFI IP address
template <class T,
  typename = my_enable_if_t<is_beefocus_sink<T>::value>>
T& operator<<( T& sink, const BeeFocus::IpAddress& address )
{
  sink << address[0] << "." << address[1] << "." << address[2] << "." << address[3];
  return sink;
}

/// @brief Output a C style string.
template <class T,
  typename = my_enable_if_t<is_beefocus_sink<T>::value>>
T& operator<<( T& sink, const char* string )
{
  std::size_t length = strlen( string );
  sink.write( string, length );
  return sink;
} 

/// @brief Output an unsigned number of a SIMPLE_ISTREAM.
template <class T,
  typename = my_enable_if_t<is_beefocus_sink<T>::value>>
T& operator<<( T& sink, unsigned int i )
{
  // Handle digits that aren't the lowest digit (if any)
  if ( i >= 10 )
  {
    sink << i/10;
  }

  // Handle the lowest digit
  char c = '0' + (i % 10);
  sink.write( &c, 1 );

  return sink;
}


/// @brief Output an signed number of a SIMPLE_ISTREAM.
template<class T,
  typename = my_enable_if_t<is_beefocus_sink<T>::value>>
T& operator<<( T& sink, int i )
{
  // Handle -
  if ( i < 0 )
  {
    sink << "-";   
    i = -i;
  }

  sink << (unsigned int) i;
  return sink;
}  

/// @brief Output an std::string
template<class T, 
  typename = my_enable_if_t<is_beefocus_sink<T>::value>>
T& operator<<( T& sink, const std::string& string )
{
  sink.write( string.c_str(), string.length() );
  return sink;
}

#endif

