#ifndef __TEST_MOCK_DEBUG__
#define __TEST_MOCK_DEBUG__

#include "debug_interface.h"

class DebugInterfaceIgnoreMock: public DebugInterface
{
  public:
  void rawWrite( const char* bytes, std::size_t numBytes ) override
  {
    // Do Nothing;
  }
};

#endif

