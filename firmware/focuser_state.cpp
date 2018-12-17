
#include <iterator>
#include <vector>
#include <string>
#include <memory>
#include "command_parser.h"
#include "wifi_debug_ostream.h"
#include "focuser_state.h"

constexpr int steps_per_rotation = 200;
constexpr int max_rotations_per_second = 2;
constexpr int NO_VALUE = -1;

const std::unordered_map<FocuserState::State,const std::string,EnumHash> 
FocuserState::stateNames = {
  { State::ACCEPT_COMMANDS,           "ACCEPTING_COMMANDS" },
  { State::DO_STEPS,                  "DO_STEPS"           },
  { State::STEPPER_INACTIVE_AND_WAIT, "STEPPER_INACTIVE"   },
  { State::STEPPER_ACTIVE_AND_WAIT,   "STEPPER_ACTIVE"     },
  { State::SET_DIR,                   "SET_DIR"            },
  { State::MOVING,                    "MOVING"             },
  { State::STOP_AT_HOME,              "STOP_AT_HOME"       },
  { State::LOW_POWER,                 "LOW_POWER"          },
  { State::AWAKEN,                    "AWAKEN"             },
  { State::ERROR_STATE,               "ERROR ERROR ERROR"  },
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

  focuser_position = 0;

  // rotate 1 turn / second for now
  
  focuser_speed = steps_per_rotation * 2;  

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
    while ( !state_stack.empty() )
      state_stack.pop_back();
    state_stack.push_back( COMMAND_PACKET(new_state, arg ) );
}

void FocuserState::push_state( State new_state, int arg0,  int arg1 )
{
    state_stack.push_back( COMMAND_PACKET(new_state, arg0,  arg1 ) );
}

FocuserState::COMMAND_PACKET& FocuserState::top( void ) 
{
  if ( state_stack.empty() )
    hard_reset_state( State::ERROR_STATE, __LINE__ );   // bug,  should never happen :)  
  return state_stack.back();
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
    *net << "Position: " << focuser_position << "\n";
    *net << "State: " << stateNames.at(state) << " " << arg0 << "\n";
    return;
  }

  if ( cp.command == CommandParser::Command::PStatus )
  {
    log << "Processing pstatus request\n";
    *net << "Position: " << focuser_position << "\n";
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
    push_state( State::STOP_AT_HOME );
    return;
  }  

  if ( cp.command == CommandParser::Command::ABSPos ) {
    push_state( State::MOVING, cp.optionalArg );
    int new_position = cp.optionalArg;

    if ( new_position < focuser_position )
    {
      int backtrack = new_position - 500;
      backtrack = backtrack < 0 ? 0 : backtrack;
      push_state( State::MOVING, backtrack );
    }
  }

  if ( cp.command == CommandParser::Command::Sleep ) {
    push_state( State::LOW_POWER, 0 );
	}
  if ( cp.command == CommandParser::Command::Wake )
  {
    hard_reset_state( State::ACCEPT_COMMANDS, 0 );
    push_state( State::AWAKEN );
  }
}

unsigned int FocuserState::state_accept_commands()
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

unsigned int FocuserState::state_set_dir()
{
  Dir desiredDir = top().arg0 ? Dir::FORWARD : Dir::REVERSE;

  state_stack.pop_back();

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

unsigned int FocuserState::state_step_inactive_and_wait()
{
  int delay = 1000000 / focuser_speed / 2;
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_INACTIVE );
  state_stack.pop_back();
  return delay;
}

unsigned int FocuserState::state_step_active_and_wait()
{
  int delay = 1000000 / focuser_speed / 2;
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_ACTIVE );
  state_stack.pop_back();
  return delay;
}

unsigned int FocuserState::state_doing_steps()
{
  if ( top().arg0 == 0 )
  {
    // We're done at 0
    state_stack.pop_back();
    return 0;
  }
  top().arg0--;

  push_state( State::STEPPER_INACTIVE_AND_WAIT );
  push_state( State::STEPPER_ACTIVE_AND_WAIT );

  return 0;  
}

unsigned int FocuserState::state_moving()
{
  if ( motorState != MotorState::ON )
  {
    hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );
    return 100000;    // .1s pause to power things up.
  }
          
  WifiDebugOstream log( debugLog.get(), net.get() );
  log << "Moving " << focuser_position << "\n";
  
  if ( top().arg0 == focuser_position ) {
    // We're at the target,  exit
    state_stack.pop_back();
    return 0;    
  }

  bool next_dir;
  int steps = top().arg0 - focuser_position;

  if (  steps > focuser_speed / 2 )
    steps = focuser_speed / 2;
  if (  steps < -focuser_speed / 2 )
    steps = -focuser_speed / 2;

  focuser_position = focuser_position + steps;

  if ( steps < 0 ) {
    next_dir = false;
    steps = -steps;
  }
  else {
    next_dir = true;
  }
  
  push_state( State::DO_STEPS, steps );
  push_state( State::SET_DIR, next_dir );
  return 0;        
}

unsigned int FocuserState::state_stop_at_home()
{
  WifiDebugOstream log( debugLog.get(), net.get() );

  if ( (focuser_position % 100) == 0 )
  {
    log << "Homing " << focuser_position << "\n";
  }
  if ( hardware->DigitalRead( HWI::Pin::HOME ) == HWI::PinState::HOME_ACTIVE ) 
  {
    log << "Hit home at position " << focuser_position << "\n";
    log << "Resetting position to 0\n";
    focuser_position = 0;
    state_stack.pop_back();
    return 0;        
  }
  push_state( State::DO_STEPS, 1 );
  push_state( State::SET_DIR, 0 );
  focuser_position--;
  return 0;        
}

unsigned int FocuserState::state_awaken()
{
  hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );        
  state_stack.pop_back();
  return 0;
}

unsigned int FocuserState::loop(void)
{
  State next_state = top().state;

  switch ( next_state ) {
    case State::ACCEPT_COMMANDS:
      return state_accept_commands();
      break;
    case State::DO_STEPS:
      return state_doing_steps();
      break;
    case State::SET_DIR:
      return state_set_dir();
      break;      
    case State::MOVING:
      return state_moving();
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
      return state_step_active_and_wait();
      break;
    case State::STEPPER_INACTIVE_AND_WAIT:      
      return state_step_inactive_and_wait();
      break;
    default: 
      // should never happen.
      WifiDebugOstream log( debugLog.get(), net.get() );
      log << "hep hep hep - Unhandled case statement in focuser_state";
      return 1000*1000; // 1 second.
  }
  return 10*1000;   // 10 microseconds
}

