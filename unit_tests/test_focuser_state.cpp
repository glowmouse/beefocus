#include <gtest/gtest.h>

#include "focuser_state.h"
#include "test_mocks.h"

HWOutTimedEvents goldenHWStart = {
  {  0, { HWI::Pin::STEP,       HWI::PinIOMode::M_OUTPUT   } },
  {  0, { HWI::Pin::DIR,        HWI::PinIOMode::M_OUTPUT   } },
  {  0, { HWI::Pin::MOTOR_ENA,  HWI::PinIOMode::M_OUTPUT   } },
  {  0, { HWI::Pin::HOME,       HWI::PinIOMode::M_INPUT    } },

  {  0, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_ON    } },
  {  0, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD } },
  {  0, { HWI::Pin::STEP,       HWI::PinState::STEP_LOW    } },
};

std::unique_ptr<FOCUSER_STATE> make_focuser( 
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

  auto focuser = std::unique_ptr<FOCUSER_STATE>(
     new FOCUSER_STATE(
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
  FOCUSER_STATE* focuser,
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
    {  0, "# Got: status" },
    {  0, "Position: 0" },
    {  0, "State: ACCEPTING_COMMANDS 0"},
    { 50, "# Got: pstatus" },
    { 50, "Position: 0" },
    { 70, "# Got: sstatus" },
    { 70, "State: ACCEPTING_COMMANDS 0"},
  };

  ASSERT_EQ( goldenNet, wifiAlias->getOutput() );
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

  NetMockSimpleTimed::TimedStringEvents goldenNet = {
    { 10, "# Got: abs_pos=3" },
    { 20, "# Moving 0"},
    { 37, "# Moving 3"},
  };

  HWOutTimedEvents goldenHW = {
    { 20, { HWI::Pin::STEP,       HWI::PinState::STEP_LOW} },
    { 21, { HWI::Pin::STEP,       HWI::PinState::STEP_HIGH} },
    { 22, { HWI::Pin::STEP,       HWI::PinState::STEP_LOW} },
    { 23, { HWI::Pin::STEP,       HWI::PinState::STEP_HIGH} },
    { 25, { HWI::Pin::STEP,       HWI::PinState::STEP_LOW} },
    { 26, { HWI::Pin::STEP,       HWI::PinState::STEP_HIGH} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, wifiAlias->getOutput() );
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

