#include <gtest/gtest.h>

#include "command_parser.h"

TEST( COMMAND_PARSER, should_process_int )
{
  ASSERT_EQ( CommandParser::process_int( std::string("123") , 0 ), 123 );
  ASSERT_EQ( CommandParser::process_int( std::string("123") , 1 ), 23 );
  ASSERT_EQ( CommandParser::process_int( std::string("123") , 2 ), 3 );
  ASSERT_EQ( CommandParser::process_int( std::string("123 "), 2 ), 3 );
  ASSERT_EQ( CommandParser::process_int( std::string("4567890") , 0 ), 4567890);
  ASSERT_EQ( CommandParser::process_int( std::string("cheese") , 0 ), 0 );
  ASSERT_EQ( CommandParser::process_int( std::string("ABS_POS=500") , 8 ), 500 );
}

