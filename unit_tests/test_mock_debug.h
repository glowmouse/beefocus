///
/// @brief Simple no-op debug interface
///

#ifndef __TEST_MOCK_DEBUG__
#define __TEST_MOCK_DEBUG__

#include "debug_interface.h"

///
/// @brief Simple no-op debug interface
///
class DebugInterfaceIgnoreMock: public DebugInterface
{
  public:
  ///
  /// @brief No-op write for debug interface
  ///
  /// @param[in] bytes     Data to be written
  /// @param[in] numBytes  The number of bytes
  ///
  /// No-op.  Does nothing
  ///
  void rawWrite( const char* bytes, std::size_t numBytes ) override
  {
  }

  std::streamsize write( const char_type* s, std::streamsize n ) override
  {
    return n;
  }
};

#endif

