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
  /// @param[in] s         Data to be written
  /// @param[in] n         The number of bytes
  ///
  /// @return n - number of bytes not writte (No-op.  Does nothing)
  ///
  std::streamsize write( const char_type* s, std::streamsize n ) override
  {
    return n;
  }
  
  void disable() override 
  {
    // Can't disable what we're ignoring.
  }

};

#endif

