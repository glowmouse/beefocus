#include <gtest/gtest.h>

#include "command_parser.h"
#include "test_mock_debug.h"
#include "test_mock_event.h"
#include "test_mock_hardware.h"
#include "test_mock_net.h"

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

  NetMockSimpleTimed empty;
  ASSERT_EQ( checkForCommands(dbgmock, empty), CommandPacket() );

  NetMockSimpleTimed junk("junk");
  ASSERT_EQ( checkForCommands(dbgmock, junk), CommandPacket());

  NetMockSimpleTimed abort("abort");
  ASSERT_EQ( checkForCommands(dbgmock, abort), CommandPacket( Command::Abort));

  NetMockSimpleTimed home("hOmE");
  ASSERT_EQ( checkForCommands(dbgmock, home), CommandPacket( Command::Home));

  NetMockSimpleTimed pstatus("PStatus");
  ASSERT_EQ( checkForCommands(dbgmock, pstatus), CommandPacket( Command::PStatus ));

  NetMockSimpleTimed pstatus2("pStatus with training garbage");
  ASSERT_EQ( checkForCommands(dbgmock, pstatus2), CommandPacket( Command::PStatus ));


  NetMockSimpleTimed sstatus("sstatus");
  ASSERT_EQ( checkForCommands(dbgmock, sstatus), CommandPacket( Command::SStatus ));

  // Argument defaults to 0 if not given
  NetMockSimpleTimed abs_pos0("ABS_POS");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos0), CommandPacket( Command::ABSPos, 0));

  NetMockSimpleTimed abs_pos1("ABS_POS=100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos1), CommandPacket( Command::ABSPos, 100));

  NetMockSimpleTimed abs_pos2("ABS_POS 100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos2), CommandPacket( Command::ABSPos, 100));

  // Sadly, whitespace matters
  NetMockSimpleTimed abs_pos3("ABS_POS  100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos3), CommandPacket( Command::ABSPos, 0));

  // Negatives not supported
  NetMockSimpleTimed abs_pos4("ABS_POS -100");
  ASSERT_EQ( checkForCommands(dbgmock, abs_pos4), CommandPacket( Command::ABSPos, 0));

}

TEST( COMMAND_PARSER, testGot)
{
  DebugInterfaceIgnoreMock dbgmock;

  TimedStringEvents input = {
    { 0, "abort" },   // Sleep @ Time 0
    { 2, "abs_pos=100" }     // Wake @ Time 2;
  };

  // Time 0, should be a sleep
  NetMockSimpleTimed netMock( input );
  ASSERT_EQ( checkForCommands(dbgmock, netMock ), CommandPacket( Command::Abort));
  // Time 1, should be nothing
  netMock.advanceTime(1);
  ASSERT_EQ( checkForCommands(dbgmock, netMock ), CommandPacket());
  // Time 2, should be a wake.
  netMock.advanceTime(1);
  ASSERT_EQ( checkForCommands(dbgmock, netMock ), CommandPacket( Command::ABSPos, 100 ));
 
  // Golden output - should have messages saying that the firmware
  // got the sleep and wake command
  TimedStringEvents golden = {
    { 0, "# Got: abort" }, 
    { 2, "# Got: abs_pos=100" } 
  };

  // Compare.
  ASSERT_EQ( golden, netMock.getOutput() ); 
}

}

