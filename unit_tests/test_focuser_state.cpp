#include <gtest/gtest.h>

#include "focuser_state.h"
#include "test_mocks.h"

std::unique_ptr<FOCUSER_STATE> make_focuser( 
  const NetMockSimpleTimed::TimedStringEvents& wifiIn,
  NetMockSimpleTimed* &net_interface
)
{
  std::unique_ptr<NetMockSimpleTimed> wifi( new NetMockSimpleTimed( wifiIn ));
  std::unique_ptr<DebugInterfaceIgnoreMock> debug( new DebugInterfaceIgnoreMock);
  std::unique_ptr<HWMockTimed> hardware( new HWMockTimed );
  
  net_interface = wifi.get();

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
    lastMockAdvanceTime = mockAdvanceTime;
  }
}

/// @brief Init the focuser and pass in some status commands
///
TEST( COMMAND_PARSER, init_Focuser )
{
  NetMockSimpleTimed::TimedStringEvents input = {
    { 0, "status" },      // status @ Time 0
    { 50, "pstatus" }     // status @ Time 1000 ms
  };

  NetMockSimpleTimed* wifiAlias;
  auto focuser = make_focuser( input, wifiAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, 1000 );

  NetMockSimpleTimed::TimedStringEvents golden = {
    {  0, "# Got: status" },
    {  0, "Position: 0" },
    {  0, "State: ACCEPTING_COMMANDS 0"},
    { 50, "# Got: pstatus" },
    { 50, "Position: 0" }
  };

  ASSERT_EQ( golden, wifiAlias->getOutput() );

}

