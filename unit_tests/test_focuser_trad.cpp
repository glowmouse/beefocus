#include <gtest/gtest.h>

#include "focuser_state.h"
#include "test_mock_debug.h"
#include "test_mock_event.h"
#include "test_mock_hardware.h"
#include "test_mock_net.h"

HWTimedEvents goldenHWStart = {
  {  0, { HWI::Pin::STEP,       HWI::PinIOMode::M_OUTPUT     } },
  {  0, { HWI::Pin::DIR,        HWI::PinIOMode::M_OUTPUT     } },
  {  0, { HWI::Pin::MOTOR_ENA,  HWI::PinIOMode::M_OUTPUT     } },
  {  0, { HWI::Pin::HOME,       HWI::PinIOMode::M_INPUT      } },

  {  0, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_ON      } },
  {  0, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD   } },
  {  0, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE } },
};

std::unique_ptr<FS::Focuser> make_focuser( 
  const TimedStringEvents& wifiIn,
  const HWTimedEvents& hwIn,
  NetMockSimpleTimed* &net_interface,
  HWMockTimed* &hw_interface
)
{
  std::unique_ptr<NetMockSimpleTimed> wifi( new NetMockSimpleTimed( wifiIn ));
  std::unique_ptr<DebugInterfaceIgnoreMock> debug( new DebugInterfaceIgnoreMock);
  std::unique_ptr<HWMockTimed> hardware( new HWMockTimed( hwIn ));
  
  net_interface = wifi.get();
  hw_interface = hardware.get();

  FS::BuildParams params( FS::Build::UNIT_TEST_TRADITIONAL_FOCUSER );

  auto focuser = std::unique_ptr<FS::Focuser>(
     new FS::Focuser(
        std::move(wifi),
        std::move(hardware),
        std::move(debug),
        params )
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
  FS::Focuser* focuser,
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

TEST( FOCUSER_STATE, allCommandsHaveInterruptStatus)
{
  for ( CommandParser::Command c = CommandParser::Command::StartOfCommands;
        c < CommandParser::Command::EndOfCommands; ++c )
  {
    ASSERT_NE( FS::doesCommandInterrupt.find( c ),
               FS::doesCommandInterrupt.end());
  }
}

namespace FS {

TEST( FOCUSER_STATE, allCommandsHaveImplementations )
{
  for ( CommandParser::Command c = CommandParser::Command::StartOfCommands;
        c < CommandParser::Command::EndOfCommands; ::operator++(c) )
  {
    ASSERT_NE( 
      Focuser::commandImpl.find( c ),
      Focuser::commandImpl.end());
  }
}
 
TEST( FOCUSER_STATE, allStatesHaveImplementations )
{
  for ( FS::State s = FS::State::START_OF_STATES;
        s < FS::State::END_OF_STATES; ++s )
  {
    ASSERT_NE( 
      FS::Focuser::stateImpl.find( s ),
      FS::Focuser::stateImpl.end());
  }
} 
}

TEST( FOCUSER_STATE, allStatesHaveDebugNames )
{
  for ( FS::State s = FS::State::START_OF_STATES;
        s < FS::State::END_OF_STATES; ++s )
  {
    ASSERT_NE( 
      FS::stateNames.find( s ),
      FS::stateNames.end());
  }
} 


/// @brief Init the focuser
///
TEST( FOCUSER_STATE, init_Focuser )
{
  TimedStringEvents netInput;

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    { 0, "# Motor set on" },
    { 0, "# Focuser is up" },
  };

  ASSERT_EQ( goldenNet, wifiAlias->getOutput() );
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}


/// @brief Init the focuser and pass in some status commands
///
TEST( FOCUSER_STATE, run_status)
{
  TimedStringEvents netInput = {
    { 0, "mstatus" },     // status  @ Time 0
    { 50, "pstatus" },    // pstatus @ Time 50 ms
    { 75, "mstatus" },    // mstatus @ Time 70 (!epoch) ms
    { 76, "pstatus" }     // check pstatus 1 ms later (!epoch)
  };
  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "State: ACCEPTING_COMMANDS NoArg"},
    { 50, "Position: 0" },
    { 80, "State: ACCEPTING_COMMANDS NoArg"},
    { 80, "Position: 0" },
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}

///
/// @brief A basic absolute position command test
///
/// Moves the focuser to position 3 and checks to see if stepper
/// 'clicks' 3 times.
/// 
TEST( FOCUSER_STATE, run_abs_pos )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=3" },      // status  @ Time 0
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet;

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

///
/// @brief Basic relative position command check
///
/// Issue a sync command to set the official focuser position to 5
/// Then issue a relative move to go back 3 spots.
///
/// The result should be 5 backwards to get rid of backlash and 2
/// forwards ( 5 - 5 + 2 = 5 - 3 )
/// 
TEST( FOCUSER_STATE, run_rel_pos)
{
  TimedStringEvents netInput = {
    { 0,  "sync=5" },
    { 10, "rel_pos=-3" },
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet;

  // Go back to 0 to correct backlash then forward to 2 ( 5 - 3 = 2 )
  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 18, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 19, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 20, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 21, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD } },
    { 22, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 23, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 24, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 25, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, run_abs_pos_negative )
{
  TimedStringEvents netInput = {
    { 0,  "sync=1" },    
    { 10, "abs_pos=-3" },
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;  
  HWMockTimed* hwMockAlias;       
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  // Should go back 1, to 0, then stop.  -3 is right now.
  TimedStringEvents goldenNet;
  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());   
  
  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
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
TEST( FOCUSER_STATE, run_abs_pos_double_forward )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=3" },      // status  @ Time 0
    { 50, "abs_pos=5" },      // status  @ Time 5
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet;

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 50, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 51, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 52, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 53, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

///
/// @brief Test double abs_pos command with backlash correct
///
/// Focuser should advance to 3, then rewind to 2 on the next
/// 'accept commands' tick after 50ms.  The rewind will got 0
/// 0 to try to clear backlash and then forward to 2.
///
TEST( FOCUSER_STATE, run_abs_pos_with_backlash_correction )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=3" },      // status  @ Time 0
    { 50, "abs_pos=2" },      // status  @ Time 5
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet;

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 50, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 51, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 52, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 53, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 54, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 55, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 56, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 57, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD } },
    { 58, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 59, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 60, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 61, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, ignore_home )
{
  TimedStringEvents netInput = {
    { 0,  "sstatus" },        // Make sure we're not synched 
    { 10, "home" },           // issue home command
    { 11, "sstatus" },        // Home is ignored
    { 20, "sstatus" },        // Home is ignored
    { 30, "abs_pos=1" },      // Move back to 1
    { 40, "sstatus" },        // Still ignored
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 18, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Synched: NO" },
    { 20, "Synched: NO" },
    { 20, "Synched: NO" },
    { 40, "Synched: NO" },
  };

  HWTimedEvents goldenHW = {
    { 30, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 31, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, sync_focuser )
{
  TimedStringEvents netInput = {
    { 0,  "sstatus" },        // Make sure we're not synched
    { 10, "sync=100" },       // issue a sync comand
    { 10, "sstatus" },        // Should now be synced
    { 10, "pstatus" },        // Poisition should be 100
  };

  HWTimedEvents hwInput;
  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Synched: NO" },
    { 10, "Synched: YES" },
    { 10, "Position: 100" }
  };

  HWTimedEvents goldenHW = goldenHWStart;

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, mstatus_while_moving )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=7" },        // Start the focuser moving
    { 13, "mstatus" },          // Ask for state status while moving
    { 15, "sstatus" },          // Ask for synch status while moving
    { 17, "pstatus" },          // Ask for position status while moving
  };

  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 18, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 19, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 20, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 21, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 22, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 23, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    { 14, "State: MOVING 7" },
    { 18, "Synched: NO" },
    { 22, "Position: 6" }
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, abort_while_moving )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=7" },        // Start the focuser moving
    { 15, "abort" },            // Nope!
    { 16, "mstatus" },          // What state are we in?
    { 20, "pstatus" },          // Where did we end up?
  };

  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    { 18, "State: ACCEPTING_COMMANDS NoArg" },
    { 20, "Position: 4" }
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, sync_while_moving )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=7" },        // Start the focuser moving
    { 15, "sync=100" },         // Actually, just synch it
    { 16, "mstatus" },          // What state are we in?
    { 20, "pstatus" },          // Where did we end up?
  };

  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    { 18, "State: ACCEPTING_COMMANDS NoArg" },
    { 20, "Position: 100" }
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, abort_on_ignored_home )
{
  TimedStringEvents netInput = {
    { 0,  "sstatus" },        // Make sure we're not synched
    { 10, "home" },           // issue home command (ignored)
    { 11, "sstatus" },        // Should not be synched
    { 16, "abort" },          // Random abort of nothing
    { 40, "sstatus" },        // Should still not be synched
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 25, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Synched: NO" },
    { 20, "Synched: NO" },
    { 40, "Synched: NO" },
  };

  HWTimedEvents goldenHW = {
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}


TEST( FOCUSER_STATE, new_move_while_moving )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=7" },        // Start the focuser moving
    { 15, "abs_pos=5" },        // On second thought...
    { 30, "pstatus" },          // Where did we land?
  };

  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 18, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 19, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    { 30, "Position: 5" }
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, move_after_ignored_home )
{
  TimedStringEvents netInput = {
    { 0,  "sstatus" },        // Make sure we're not synched
    { 10, "home" },           // issue home command
    { 11, "sstatus" },        // Should not be synched - home ignored
    { 16, "abs_pos=1" },      // Now move
    { 40, "sstatus" },        // Should still be synched
    { 41, "pstatus" },        // Should still be synched.
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 20, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Synched: NO" },
    { 20, "Synched: NO" },
    { 40, "Synched: NO" },
    { 50, "Position: 1" },
  };

  HWTimedEvents goldenHW = {
    { 20, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 21, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}


TEST( FOCUSER_STATE, mostly_ignore_home_while_moving )
{
  TimedStringEvents netInput = {
    {  0, "sstatus" },          // Should not start synched
    { 10, "abs_pos=99" },       // Start the focuser moving
    { 13, "pstatus" },          // Where are we?
    { 15, "home" },             // Home will actually stop us moving 
    { 17, "mstatus" },          // What are we doing now?
    { 30, "pstatus" },          // Where did we land?
    { 32, "sstatus" },          // Are we homed?  Should be no.
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 22, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    {  0,  "Synched: NO" },
    { 14,  "Position: 2" },
    { 18,  "State: ACCEPTING_COMMANDS NoArg" },
    { 30,  "Position: 4"},
    { 40,  "Synched: NO"},
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, ignore_multiple_homes )
{
  TimedStringEvents netInput = {
    {  0, "sstatus" },          // Are we synched?
    { 10, "home" },             // Should ignore
    { 13, "pstatus" },          // Where are we?
    { 15, "home" },             // Should still ignore
    { 17, "sstatus" },          // What are we doing now?
    { 30, "pstatus" },          // Where did we land?
    { 32, "sstatus" },          // Are we synched? should be no.
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 22, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  HWTimedEvents goldenHW = {
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    {  0,  "Synched: NO" },
    { 20,  "Position: 0" },
    { 20,  "Synched: NO" },
    { 30,  "Position: 0"},
    { 40,  "Synched: NO"},
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, interrupt_during_backlash_correction )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=5" },       // Forward to 5
    { 50, "abs_pos=4" },       // Backwards, should trigger a rewind
    { 53, "abort" },           // Nope!
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet;

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 17, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 18, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 19, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 50, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 51, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 52, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 53, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 54, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, enterSleepModeAndWake )
{
  TimedStringEvents netInput = {
    { 50,   "abs_pos=1" },        // Forward to 1
    { 50,   "mstatus" },          // What are we doing?
    { 60,   "mstatus" },          // What are we doing now?
    { 1059, "mstatus" },          // Right before sleep mode
    { 1060, "mstatus" },          // Will process mstatus before sleeping
    { 1061, "mstatus" },          // Now we'll be in sleep mode
    { 1062, "pstatus" },          // Check timing, should happen at 1500
    { 1500, "sstatus" },          // Should also happen at 1500
    { 1501, "mstatus" },          // Should happen at 2000
    { 2001, "abs_pos=2" },        // Should happen at 2500
    { 2500, "mstatus" },
    { 4200, "abort" },            // should cause a wake @ 4500
    { 5600, "home" },             // Even though home isn't supported, will wake
  };

  HWTimedEvents hwInput= {
    { 0,    { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 6206, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 10000 );

  TimedStringEvents goldenNet = {
    { 50,   "State: MOVING 1"},
    { 60,   "State: ACCEPTING_COMMANDS NoArg" },
    { 1060, "State: ACCEPTING_COMMANDS NoArg" },
    { 1060, "State: ACCEPTING_COMMANDS NoArg" },
    { 1500, "State: LOW_POWER NoArg" },
    { 1500, "Position: 1" },
    { 1500, "Synched: NO" },
    { 2000, "State: LOW_POWER NoArg" },
    { 2700, "State: MOVING 2" },
  };

  HWTimedEvents goldenHW = {
    { 50,   { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 51,   { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 1060, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_OFF} },
    { 2500, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_ON} },
    { 2700, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 2701, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 3510, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_OFF} },
    { 4500, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_ON} },
    { 5510, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_OFF} },
    { 6000, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_ON} },
    { 7010, { HWI::Pin::MOTOR_ENA,  HWI::PinState::MOTOR_OFF} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
}

TEST( FOCUSER_STATE, getFirmwareAndCaps )
{ 
  TimedStringEvents netInput = {
    { 0, "firmware" },     // Firmware version @ Time 0
    { 50, "caps" },        // Caps request @ Time 50ms
  };
  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    { 0,  "Firmware: 1.0"},
    { 50, "MaxPos: 5000"},
    { 50, "CanHome: NO" }
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, doesNotGoBeyoundMax)
{
  TimedStringEvents netInput = {
    { 0,      "abs_pos=17000" },
    { 60000,  "pstatus" },
  };
  
  HWTimedEvents hwInput;
  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 70000 );
    
  TimedStringEvents goldenNet = {
    { 60000,  "Position: 5000" },
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
} 


