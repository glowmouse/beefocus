#include <gtest/gtest.h>

#include "focuser_state.h"
#include "test_mocks.h"

HWOutTimedEvents goldenHWStart = {
  {  0, { HWI::Pin::STEP,       HWI::PinIOMode::M_OUTPUT     } },
  {  0, { HWI::Pin::DIR,        HWI::PinIOMode::M_OUTPUT     } },
  {  0, { HWI::Pin::MOTOR_ENA,  HWI::PinIOMode::M_OUTPUT     } },
  {  0, { HWI::Pin::HOME,       HWI::PinIOMode::M_INPUT      } },

  {  0, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_ON      } },
  {  0, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD   } },
  {  0, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE } },
};

std::unique_ptr<FocuserState> make_focuser( 
  const NetMockSimpleTimed::TimedStringEvents& wifiIn,
  NetMockSimpleTimed* &net_interface,
  HWMockTimed* &hw_interface
)
{
  std::unique_ptr<NetMockSimpleTimed> wifi( new NetMockSimpleTimed( wifiIn ));
  std::unique_ptr<DebugInterfaceIgnoreMock> debug( new DebugInterfaceIgnoreMock);
  std::unique_ptr<HWMockTimed> hardware( new HWMockTimed );
  
  net_interface = wifi.get();
  hw_interface = hardware.get();

  auto focuser = std::unique_ptr<FocuserState>(
     new FocuserState(
        std::move(wifi),
        std::move(hardware),
        std::move(debug) )
  );

  return focuser;
}

/// @brief Simulate the focuser
///
/// @param[out] Focuser  - A pointer to the focuser.
/// @param[in] wifiAlisa - A pointer to the WIFI/ Network Mock
/// @param[in] end_time  - How long (in MS) to run the focuser for.
/// 
void simulateFocuser( 
  FocuserState* focuser,
  NetMockSimpleTimed* wifiAlias,
  HWMockTimed* hwMockAlias,
  unsigned int endTime
)
{
  unsigned int time = 0;
  unsigned int lastMockAdvanceTime = 0;
  while ( time < endTime*1000 )
  {
    time = time + focuser->loop();
    unsigned int mockAdvanceTime = time/1000;
    wifiAlias->advanceTime( mockAdvanceTime - lastMockAdvanceTime );
    hwMockAlias->advanceTime( mockAdvanceTime - lastMockAdvanceTime );
    lastMockAdvanceTime = mockAdvanceTime;
  }
}

/// @brief Init the focuser
///
TEST( COMMAND_PARSER, init_Focuser )
{
  NetMockSimpleTimed::TimedStringEvents input;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( input, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  NetMockSimpleTimed::TimedStringEvents goldenNet;

  ASSERT_EQ( goldenNet, wifiAlias->getOutput() );
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}


/// @brief Init the focuser and pass in some status commands
///
TEST( COMMAND_PARSER, run_status)
{
  NetMockSimpleTimed::TimedStringEvents input = {
    { 0, "status" },      // status  @ Time 0
    { 50, "pstatus" },    // pstatus @ Time 50 ms
    { 70, "sstatus" }     // sstatus @ Time 70 ms
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( input, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  NetMockSimpleTimed::TimedStringEvents goldenNet = {
    {  0, "Position: 0" },
    {  0, "State: ACCEPTING_COMMANDS 0"},
    { 50, "Position: 0" },
    { 70, "State: ACCEPTING_COMMANDS 0"},
  };

  ASSERT_EQ( goldenNet, wifiAlias->getFilteredOutput() );
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}

TEST( COMMAND_PARSER, run_abs_pos )
{
  NetMockSimpleTimed::TimedStringEvents input = {
    { 10, "abs_pos=3" },      // status  @ Time 0
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( input, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  NetMockSimpleTimed::TimedStringEvents goldenNet;

  HWOutTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, wifiAlias->getFilteredOutput() );
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

///
/// @brief Test double abs_pos command
///
/// Focuser should advance to 3, then advance to 5 on the next
/// 'accept commands' tick after 50ms
///
/// TODO - modify accept for commands so it tries to sync up with
///        10ms epochs to make unit testing easier.
///
TEST( COMMAND_PARSER, run_abs_pos_double_forward )
{
  NetMockSimpleTimed::TimedStringEvents input = {
    { 10, "abs_pos=3" },      // status  @ Time 0
    { 50, "abs_pos=5" },      // status  @ Time 5
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( input, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  NetMockSimpleTimed::TimedStringEvents goldenNet;

  HWOutTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 57, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 58, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 60, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 61, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, wifiAlias->getFilteredOutput() );
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

///
/// @brief Test double abs_pos command with backlash correct
///
/// Focuser should advance to 3, then rewind to 2 on the next
/// 'accept commands' tick after 50ms.  The rewind will got 0
/// 0 to try to clear backlash and then forward to 2.
///
TEST( COMMAND_PARSER, run_abs_pos_with_backlash_correction )
{
  NetMockSimpleTimed::TimedStringEvents input = {
    { 10, "abs_pos=3" },      // status  @ Time 0
    { 50, "abs_pos=2" },      // status  @ Time 5
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( input, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  NetMockSimpleTimed::TimedStringEvents goldenNet;

  HWOutTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 57, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 58, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 59, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 61, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 62, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 63, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 64, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 66, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD } },
    { 67, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 68, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 69, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 70, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, wifiAlias->getFilteredOutput() );
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}


