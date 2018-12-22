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

std::unique_ptr<FocuserState> make_focuser( 
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

TEST( FOCUSER_STATE, allCommandsHaveInterruptStatus)
{
  for ( CommandParser::Command c = CommandParser::Command::StartOfCommands;
        c < CommandParser::Command::EndOfCommands; ++c )
  {
    ASSERT_NE( 
      FocuserState::doesCommandInterrupt.find( c ),
      FocuserState::doesCommandInterrupt.end());
  }
}

TEST( FOCUSER_STATE, allCommandsHaveImplementations )
{
  for ( CommandParser::Command c = CommandParser::Command::StartOfCommands;
        c < CommandParser::Command::EndOfCommands; ++c )
  {
    ASSERT_NE( 
      FocuserState::commandImpl.find( c ),
      FocuserState::commandImpl.end());
  }
}
 
 
TEST( FOCUSER_STATE, allStatesHaveImplementations )
{
  for ( FocuserState::State s = FocuserState::State::START_OF_STATES;
        s < FocuserState::State::END_OF_STATES; ++s )
  {
    ASSERT_NE( 
      FocuserState::stateImpl.find( s ),
      FocuserState::stateImpl.end());
  }
} 


TEST( FOCUSER_STATE, allStatesHaveDebugNames )
{
  for ( FocuserState::State s = FocuserState::State::START_OF_STATES;
        s < FocuserState::State::END_OF_STATES; ++s )
  {
    ASSERT_NE( 
      FocuserState::stateNames.find( s ),
      FocuserState::stateNames.end());
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

  TimedStringEvents goldenNet;

  ASSERT_EQ( goldenNet, wifiAlias->getOutput() );
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}


/// @brief Init the focuser and pass in some status commands
///
TEST( FOCUSER_STATE, run_status)
{
  TimedStringEvents netInput = {
    { 0, "sstatus" },     // status  @ Time 0
    { 50, "pstatus" },    // pstatus @ Time 50 ms
    { 70, "sstatus" }     // sstatus @ Time 70 ms
  };
  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
  };

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "State: ACCEPTING_COMMANDS 0"},
    { 50, "Position: 0" },
    { 70, "State: ACCEPTING_COMMANDS 0"},
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHWStart, hwMockAlias->getOutEvents() );
}

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
    { 56, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 57, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 58, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 59, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
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
    { 56, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 57, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 58, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 59, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 60, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 61, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 62, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 63, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD } },
    { 64, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 65, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 66, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 67, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };
  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, home_focuser )
{
  TimedStringEvents netInput = {
    { 0,  "hstatus" },        // Make sure we're not homed
    { 10, "home" },           // issue home command
    { 11, "hstatus" },        // Should not be homed during homing
    { 20, "hstatus" },        // Should now be homed
    { 30, "abs_pos=1" },      // Move back to 1
    { 40, "hstatus" },        // Should still be homed.
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 18, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Homed: NO" },
    { 13, "Homed: NO" },
    { 29, "Homed: YES" },
    { 42, "Homed: YES" },
  };

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
    { 39, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD} },
    { 40, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 41, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, sstatus_while_moving )
{
  TimedStringEvents netInput = {
    { 10, "abs_pos=7" },        // Start the focuser moving
    { 13, "sstatus" },          // Ask for state status while moving
    { 15, "hstatus" },          // Ask for home status while moving
    { 17, "pstatus" },          // Ask for position status while moving
  };

  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
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
    { 18, "Homed: NO" },
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
    { 16, "sstatus" },          // What state are we in?
    { 20, "pstatus" },          // Where did we end up?
  };

  HWTimedEvents hwInput;

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
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
    { 18, "State: ACCEPTING_COMMANDS 0" },
    { 28, "Position: 4" }
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, abort_while_homing )
{
  TimedStringEvents netInput = {
    { 0,  "hstatus" },        // Make sure we're not homed
    { 10, "home" },           // issue home command
    { 11, "hstatus" },        // Should not be homed during homing
    { 16, "abort" },          // On second thought... 
    { 40, "hstatus" },        // Should still be homed.
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 25, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Homed: NO" },
    { 13, "Homed: NO" },
    { 47, "Homed: NO" },
  };

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
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
  focuser->setMaxStepsToDoAtOnce( 2 );
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

TEST( FOCUSER_STATE, new_move_while_homing )
{
  TimedStringEvents netInput = {
    { 0,  "hstatus" },        // Make sure we're not homed
    { 10, "home" },           // issue home command
    { 11, "hstatus" },        // Should not be homed during homing
    { 16, "abs_pos=1" },      // On second thought... 
    { 40, "hstatus" },        // Should still be homed.
    { 41, "pstatus" },        // Should still be homed.
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 20, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

  TimedStringEvents goldenNet = {
    {  0, "Homed: NO" },
    { 13, "Homed: NO" },
    { 40, "Homed: NO" },
    { 50, "Position: 1" },
  };

  HWTimedEvents goldenHW = {
    { 10, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 11, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 12, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 13, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 14, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 15, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 16, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 17, { HWI::Pin::DIR,        HWI::PinState::DIR_FORWARD } },
    { 18, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 19, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}


TEST( FOCUSER_STATE, new_home_while_moving )
{
  TimedStringEvents netInput = {
    {  0, "hstatus" },          // Are we homed?
    { 10, "abs_pos=99" },       // Start the focuser moving
    { 13, "pstatus" },          // Where are we?
    { 15, "home" },             // On second thought, I forgot to home
    { 17, "sstatus" },          // What are we doing now?
    { 30, "pstatus" },          // Where did we land?
    { 32, "hstatus" },          // Are we homed?
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 22, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
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
    { 18, { HWI::Pin::DIR,        HWI::PinState::DIR_BACKWARD } },
    { 19, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 20, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
    { 21, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 22, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    {  0,  "Homed: NO" },
    { 14,  "Position: 2" },
    { 18,  "State: STOP_AT_HOME -1" },
    { 33,  "Position: 0"},
    { 33,  "Homed: YES"},
  };

  ASSERT_EQ( goldenNet, testFilterComments(wifiAlias->getOutput() ));
  ASSERT_EQ( goldenHW, hwMockAlias->getOutEvents() );
}

TEST( FOCUSER_STATE, new_home_while_homing )
{
  TimedStringEvents netInput = {
    {  0, "hstatus" },          // Are we homed?
    { 10, "home" },             // Start the focuser moving
    { 13, "pstatus" },          // Where are we?
    { 15, "home" },             // On second thought, I forgot to home
    { 17, "hstatus" },          // What are we doing now?
    { 30, "pstatus" },          // Where did we land?
    { 32, "hstatus" },          // Are we homed?
  };

  HWTimedEvents hwInput= {
    { 0,  { HWI::Pin::HOME,        HWI::PinState::HOME_INACTIVE} },
    { 22, { HWI::Pin::HOME,        HWI::PinState::HOME_ACTIVE } },
  }; 

  NetMockSimpleTimed* wifiAlias;
  HWMockTimed* hwMockAlias;
  auto focuser = make_focuser( netInput, hwInput, wifiAlias, hwMockAlias ); 
  focuser->setMaxStepsToDoAtOnce( 2 );
  simulateFocuser( focuser.get(), wifiAlias, hwMockAlias, 1000 );

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
    { 21, { HWI::Pin::STEP,       HWI::PinState::STEP_ACTIVE} },
    { 22, { HWI::Pin::STEP,       HWI::PinState::STEP_INACTIVE} },
  };

  goldenHW.insert( goldenHW.begin(), goldenHWStart.begin(), goldenHWStart.end());
  TimedStringEvents goldenNet = {
    {  0,  "Homed: NO" },
    { 13,  "Position: 0" },
    { 17,  "Homed: NO" },
    { 33,  "Position: 0"},
    { 33,  "Homed: YES"},
  };

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
  focuser->setMaxStepsToDoAtOnce( 2 );
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


