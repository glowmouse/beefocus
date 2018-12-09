#include <gtest/gtest.h>

#include "command_parser.h"
#include "test_mocks.h"

namespace CommandParser
{

TEST( COMMAND_PARSER, should_process_int )
{
  ASSERT_EQ( process_int( std::string("123") , 0 ), 123 );
  ASSERT_EQ( process_int( std::string("123") , 1 ), 23 );
  ASSERT_EQ( process_int( std::string("123") , 2 ), 3 );
  ASSERT_EQ( process_int( std::string("123 "), 2 ), 3 );
  ASSERT_EQ( process_int( std::string("4567890") , 0 ), 4567890);
  ASSERT_EQ( process_int( std::string("cheese") , 0 ), 0 );
  ASSERT_EQ( process_int( std::string("ABS_POS=500") , 8 ), 500 );
  ASSERT_EQ( process_int( std::string("ABS_POS") , 8 ), 0 );
  ASSERT_EQ( process_int( std::string("ABS_POS") , 7 ), 0 );
}

TEST( COMMAND_PARSER, checkForCommands)
{
  DebugInterfaceIgnoreMock dbgmock;

  NetMockEmpty noInput;
  ASSERT_EQ( checkForCommands( dbgmock, noInput ), CommandPacket() );

  NetMockSimple junk("junk");
  ASSERT_EQ( checkForCommands(dbgmock, junk), CommandPacket());

  NetMockSimple ping("PING");
  ASSERT_EQ( checkForCommands(dbgmock, ping), CommandPacket( Command::Ping ));

  NetMockSimple abort("abort");
  ASSERT_EQ( checkForCommands(dbgmock, abort), CommandPacket( Command::Abort));

  NetMockSimple home("hOmE");
  ASSERT_EQ( checkForCommands(dbgmock, home), CommandPacket( Command::Home));

  NetMockSimple status("status with trailing garbage");
  ASSERT_EQ( checkForCommands(dbgmock, status), CommandPacket( Command::Status));

  NetMockSimple pstatus("PStatus");
  ASSERT_EQ( checkForCommands(dbgmock, pstatus), CommandPacket( Command::PStatus ));

  NetMockSimple sstatus("sstatus");
  ASSERT_EQ( checkForCommands(dbgmock, sstatus), CommandPacket( Command::SStatus ));

  // Argument defaults to 0 if not given
  NetMockSimple abs_pos0("ABS_POS");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos0), CommandPacket( Command::ABSPos, 0));

  NetMockSimple abs_pos1("ABS_POS=100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos1), CommandPacket( Command::ABSPos, 100));

  NetMockSimple abs_pos2("ABS_POS 100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos2), CommandPacket( Command::ABSPos, 100));

  // Sadly, whitespace matters
  NetMockSimple abs_pos3("ABS_POS  100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos3), CommandPacket( Command::ABSPos, 0));

  // Negatives not supported
  NetMockSimple abs_pos4("ABS_POS -100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos4), CommandPacket( Command::ABSPos, 0));

  NetMockSimple sleep("sleep");
  ASSERT_EQ( checkForCommands(dbgmock, sleep), CommandPacket( Command::Sleep ));

  NetMockSimple wake("wake");
  ASSERT_EQ( checkForCommands(dbgmock, wake), CommandPacket( Command::Wake ));


}

}
