#include <gtest/gtest.h>

#include "focuser_state.h"
#include "test_mocks.h"

TEST( COMMAND_PARSER, init_Focuser )
{
  NetMockSimpleTimed::TimedStringEvents input = {
    { 0, "status" },      // status @ Time 0
    { 50, "pstatus" }    // status @ Time 1000 ms
  };

  std::unique_ptr<NetMockSimpleTimed> wifi( new NetMockSimpleTimed( input ));
  std::unique_ptr<DebugInterfaceIgnoreMock> debug( new DebugInterfaceIgnoreMock);
  std::unique_ptr<HWMockTimed> hardware( new HWMockTimed );

  auto wifiAlias = wifi.get();

  auto focuser = std::unique_ptr<FOCUSER_STATE>(
     new FOCUSER_STATE(
        std::move(wifi),
        std::move(hardware),
        std::move(debug) )
  );

  unsigned int time = 0;
  unsigned int lastMockAdvanceTime = 0;
  while ( time < 100000)
  {
    time = time + focuser->loop();
    unsigned int mockAdvanceTime = time/1000;
    wifiAlias->advanceTime( mockAdvanceTime - lastMockAdvanceTime );
    lastMockAdvanceTime = mockAdvanceTime;
  }

  NetMockSimpleTimed::TimedStringEvents golden = {
    {  0, "# Got: status" },
    {  0, "Position: 0" },
    {  0, "State: ACCEPTING_COMMANDS 0"},
    { 50, "# Got: pstatus" },
    { 50, "Position: 0" }
  };

  ASSERT_EQ( golden, wifiAlias->getOutput() );

}

