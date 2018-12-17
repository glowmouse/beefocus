
#include <iterator>
#include <vector>
#include <string>
#include <memory>
#include "command_parser.h"
#include "wifi_debug_ostream.h"
#include "focuser_state.h"

const std::unordered_map<FocuserState::State,const std::string,EnumHash> 
  FocuserState::stateNames = 
{
  { State::ACCEPT_COMMANDS,               "ACCEPTING_COMMANDS" },
  { State::DO_STEPS,                      "DO_STEPS"           },
  { State::STEPPER_INACTIVE_AND_WAIT,     "STEPPER_INACTIVE"   },
  { State::STEPPER_ACTIVE_AND_WAIT,       "STEPPER_ACTIVE"     },
  { State::SET_DIR,                       "SET_DIR"            },
  { State::MOVING,                        "MOVING"             },
  { State::STOP_AT_HOME,                  "STOP_AT_HOME"       },
  { State::LOW_POWER,                     "LOW_POWER"          },
  { State::AWAKEN,                        "AWAKEN"             },
  { State::ERROR_STATE,                   "ERROR ERROR ERROR"  },
};

const std::unordered_map<CommandParser::Command,bool,EnumHash> 
  FocuserState::doesCommandInterrupt= 
{
  { CommandParser::Command::Ping,          false  },
  { CommandParser::Command::Abort,         true   },
  { CommandParser::Command::Home,          true   },
  { CommandParser::Command::Status,        false  },
  { CommandParser::Command::PStatus,       false  },
  { CommandParser::Command::SStatus,       false  },
  { CommandParser::Command::ABSPos,        true   },
  { CommandParser::Command::Sleep,         true   },
  { CommandParser::Command::Wake,          true   },
  { CommandParser::Command::NoCommand,     false  },
};

FocuserState::FocuserState(
    std::unique_ptr<NetInterface> netArg,
    std::unique_ptr<HWI> hardwareArg,
    std::unique_ptr<DebugInterface> debugArg
)
{
  std::swap( net, netArg );
  std::swap( hardware, hardwareArg );
  std::swap( debugLog, debugArg );
  
  DebugInterface& log = *debugLog;
  log << "Bringing up net interface\n";
  
  // Bring up the interface to the controlling computer

  net->setup( log );

  focuserPosition = 0;

  // No setup right now,  so accept commands
  
  hard_reset_state( State::ACCEPT_COMMANDS, 0 );  

  //
  // Set the pin modes 
  //
  hardware->PinMode(HWI::Pin::STEP, HWI::PinIOMode::M_OUTPUT );  
  hardware->PinMode(HWI::Pin::DIR,  HWI::PinIOMode::M_OUTPUT );  
  hardware->PinMode(HWI::Pin::MOTOR_ENA,  HWI::PinIOMode::M_OUTPUT );  
  hardware->PinMode(HWI::Pin::HOME, HWI::PinIOMode::M_INPUT ); 
 
  //
  // Set the output pin defaults and internal state
  //
  motorState = MotorState::ON;
  hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );
        
  dir = Dir::FORWARD;
  hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_FORWARD); 
       
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_INACTIVE );

  log << "FocuserState is up\n";
}

void FocuserState::hard_reset_state( State new_state, int arg )
{
    while ( !stateStack.empty() )
      stateStack.pop_back();
    stateStack.push_back( COMMAND_PACKET(new_state, arg ) );
}

void FocuserState::pushState( State new_state, int arg0 )
{
    stateStack.push_back( COMMAND_PACKET(new_state, arg0 ));
}

FocuserState::COMMAND_PACKET& FocuserState::top( void ) 
{
  if ( stateStack.empty() )
    hard_reset_state( State::ERROR_STATE, __LINE__ );   // bug,  should never happen :)  
  return stateStack.back();
}

void FocuserState::processCommand( CommandParser::CommandPacket cp )
{
  DebugInterface& log = *debugLog;

  // Status was originally before abort.  TODO, refactor this mess.

  if ( cp.command == CommandParser::Command::Status )
  {
    State state = top().state;
    int arg0 = top().arg0;

    log << "Processing pstatus request\n";
    *net << "Position: " << focuserPosition << "\n";
    *net << "State: " << stateNames.at(state) << " " << arg0 << "\n";
    return;
  }

  if ( cp.command == CommandParser::Command::PStatus )
  {
    log << "Processing pstatus request\n";
    *net << "Position: " << focuserPosition << "\n";
    return;
  }

  if ( cp.command == CommandParser::Command::SStatus )
  {
    State state = top().state;
    int arg0 = top().arg0;

    log << "Processing sstatus request\n";
    *net << "State: " << stateNames.at(state) << " " << arg0 << "\n";
    return;
  }

  if ( cp.command == CommandParser::Command::Abort ) {
    // Abort command, unroll the state stack and start accepting commands.
    hard_reset_state( State::ACCEPT_COMMANDS, 0 );
    return;
  }

  if ( cp.command == CommandParser::Command::Home ) {
    hard_reset_state( State::ACCEPT_COMMANDS, 0 );
    pushState( State::STOP_AT_HOME );
    return;
  }  

  if ( cp.command == CommandParser::Command::ABSPos ) {
    pushState( State::MOVING, cp.optionalArg );
    int new_position = cp.optionalArg;

    if ( new_position < focuserPosition )
    {
      int backtrack = new_position - 500;
      backtrack = backtrack < 0 ? 0 : backtrack;
      pushState( State::MOVING, backtrack );
    }
  }

  if ( cp.command == CommandParser::Command::Sleep ) {
    pushState( State::LOW_POWER, 0 );
	}
  if ( cp.command == CommandParser::Command::Wake )
  {
    hard_reset_state( State::ACCEPT_COMMANDS, 0 );
    pushState( State::AWAKEN );
  }
}

unsigned int FocuserState::stateAcceptCommands()
{
  DebugInterface& log = *debugLog;
  auto cp = CommandParser::checkForCommands( log, *net );

  if ( cp.command != CommandParser::Command::NoCommand )
  {
    processCommand( cp );
    return 0;
  }
  return 10*1000;
}

unsigned int FocuserState::state_low_power()
{
  if ( motorState != MotorState::OFF )
  {
    hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_OFF ); 
    motorState = MotorState::OFF;
  }
       
  DebugInterface& log = *debugLog;
  auto cp = CommandParser::checkForCommands( log, *net );

  if ( cp.command != CommandParser::Command::NoCommand )
  {
    processCommand( cp );
    return 0;
  }
  return 100*1000;
}

unsigned int FocuserState::stateSetDir()
{
  Dir desiredDir = top().arg0 ? Dir::FORWARD : Dir::REVERSE;

  stateStack.pop_back();

  if ( desiredDir != dir )
  {
    dir = desiredDir;
    if ( dir == Dir::FORWARD )
      hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_FORWARD); 
    if ( dir == Dir::REVERSE )       
      hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_BACKWARD );
    // Trigger a 1ms pause so the stepper motor controller sees the
    // state change before we try to do anything.
    return 1000;
  }    

  return 0;
}

unsigned int FocuserState::stateStepInactiveAndWait()
{
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_INACTIVE );
  stateStack.pop_back();
  return 1000;
}

unsigned int FocuserState::stateStepActiveAndWait()
{
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_ACTIVE );
  stateStack.pop_back();
  return 1000;
}

unsigned int FocuserState::stateDoingSteps()
{
  if ( top().arg0 == 0 )
  {
    // We're done at 0
    stateStack.pop_back();
    return 0;
  }
  top().arg0--;

  pushState( State::STEPPER_INACTIVE_AND_WAIT );
  pushState( State::STEPPER_ACTIVE_AND_WAIT );

  focuserPosition += (dir == Dir::FORWARD) ? 1 : -1;

  return 0;  
}

unsigned int FocuserState::stateMoving()
{
  if ( motorState != MotorState::ON )
  {
    hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );
    return 100000;    // .1s pause to power things up.
  }
          
  WifiDebugOstream log( debugLog.get(), net.get() );
  log << "Moving " << focuserPosition << "\n";
  
  if ( top().arg0 == focuserPosition ) {
    // We're at the target,  exit
    stateStack.pop_back();
    return 0;    
  }

  const int  steps        = top().arg0 - focuserPosition;
  const bool nextDir      = steps > 0;    // TODO, enum, !bool
  const int  absSteps     = steps > 0 ? steps : -steps;
  const int  clippedSteps = absSteps > 50 ? 50 : absSteps;

  pushState( State::DO_STEPS, clippedSteps );
  pushState( State::SET_DIR,  nextDir );
  return 0;        
}

unsigned int FocuserState::state_stop_at_home()
{
  WifiDebugOstream log( debugLog.get(), net.get() );

  if ( (focuserPosition % 100) == 0 )
  {
    log << "Homing " << focuserPosition << "\n";
  }
  if ( hardware->DigitalRead( HWI::Pin::HOME ) == HWI::PinState::HOME_ACTIVE ) 
  {
    log << "Hit home at position " << focuserPosition << "\n";
    log << "Resetting position to 0\n";
    focuserPosition = 0;
    stateStack.pop_back();
    return 0;        
  }
  pushState( State::DO_STEPS, 1 );
  pushState( State::SET_DIR, 0 );
  focuserPosition--;
  return 0;        
}

unsigned int FocuserState::state_awaken()
{
  hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );        
  stateStack.pop_back();
  return 0;
}

unsigned int FocuserState::loop(void)
{
  State next_state = top().state;

  switch ( next_state ) {
    case State::ACCEPT_COMMANDS:
      return stateAcceptCommands();
      break;
    case State::DO_STEPS:
      return stateDoingSteps();
      break;
    case State::SET_DIR:
      return stateSetDir();
      break;      
    case State::MOVING:
      return stateMoving();
      break;
    case State::STOP_AT_HOME:
      return state_stop_at_home();
      break;      
    case State::LOW_POWER:
      return state_low_power();
      break;      
    case State::AWAKEN:
      return state_awaken();
      break;
    case State::STEPPER_ACTIVE_AND_WAIT:      
      return stateStepActiveAndWait();
      break;
    case State::STEPPER_INACTIVE_AND_WAIT:      
      return stateStepInactiveAndWait();
      break;
    default: 
      // should never happen.
      WifiDebugOstream log( debugLog.get(), net.get() );
      log << "hep hep hep - Unhandled case statement in focuser_state";
      return 1000*1000; // 1 second.
  }
  return 10*1000;   // 10 microseconds
}

