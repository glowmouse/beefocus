#include <gtest/gtest.h>

#include "command_parser.h"

TEST( COMMAND_PARSER, should_process_int )
{
  std::string basic_int = "100";
  ASSERT_EQ( CommandParser::process_int( basic_int, 0 ), 100 );
}

