
#include <iterator>
#include <vector>
#include <string>
#include <memory>
#include "command_parser.h"
#include "wifi_debug_ostream.h"
#include "focuser_state.h"
#include "stdexcept"

constexpr int steps_per_rotation = 200;
constexpr int max_rotations_per_second = 2;
constexpr int NO_VALUE = -1;

FOCUSER_STATE::FOCUSER_STATE(
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
  
  hard_reset_state( E_ACCEPT_COMMANDS, 0 );  

  // Book keeping for the states.

  for ( int i = 0; i < E_END; ++i )
    state_names[ i ] = "UNDEFINED";
  state_names[ E_CHECK_FOR_ABORT ] = "CHECK_FOR_ABORT";
  state_names[ E_ACCEPT_COMMANDS ] = "ACCEPTING_COMMANDS";  
  state_names[ E_DO_STEPS ] = "DOING_STEPS";
  state_names[ E_SET_DIR ] = "SET_DIRECTION";
  state_names[ E_MOVING ] = "MOVING";
  state_names[ E_ERROR_STATE ] = "ERROR";

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
       
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_LOW );

  log << "FOCUSER_STATE is up\n";
}

void FOCUSER_STATE::hard_reset_state( STATE new_state, int arg )
{
    while ( !state_stack.empty() )
      state_stack.pop_back();
    state_stack.push_back( COMMAND_PACKET(new_state, arg ) );
}

void FOCUSER_STATE::push_state( STATE new_state, int arg0,  int arg1 )
{
    state_stack.push_back( COMMAND_PACKET(new_state, arg0,  arg1 ) );
}

FOCUSER_STATE::COMMAND_PACKET& FOCUSER_STATE::get_current_command( void ) 
{
  if ( state_stack.empty() )
    hard_reset_state( E_ERROR_STATE, __LINE__ );   // bug,  should never happen :)  
  return state_stack.back();
}


void FOCUSER_STATE::processCommand( CommandParser::CommandPacket cp )
{
  DebugInterface& log = *debugLog;

  // Status was originally before abort.  TODO, refactor this mess.

  if ( cp.command == CommandParser::Command::Status )
  {
    STATE state = get_current_command().state;
    int arg0 = get_current_command().arg0;

    log << "Processing pstatus request\n";
    *net << "Position: " << focuser_position << "\n";
    *net << "State: " << state_names[state] << " " << arg0 << "\n";
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
    STATE state = get_current_command().state;
    int arg0 = get_current_command().arg0;

    log << "Processing sstatus request\n";
    *net << "State: " << state_names[state] << " " << arg0 << "\n";
    return;
  }

  if ( cp.command == CommandParser::Command::Abort ) {
    // Abort command, unroll the state stack and start accepting commands.
    hard_reset_state( E_ACCEPT_COMMANDS, 0 );
    return;
  }

  if ( cp.command == CommandParser::Command::Home ) {
    hard_reset_state( E_ACCEPT_COMMANDS, 0 );
    push_state( E_STOP_AT_HOME );
    return;
  }  

  if ( cp.command == CommandParser::Command::ABSPos ) {
    push_state( E_MOVING, cp.optionalArg );
    int new_position = cp.optionalArg;

    if ( new_position < focuser_position )
    {
      int backtrack = new_position - 500;
      backtrack = backtrack < 0 ? 0 : backtrack;
      push_state( E_MOVING, backtrack );
    }
  }

  if ( cp.command == CommandParser::Command::Sleep ) {
    push_state( E_LOW_POWER, 0 );
	}
  if ( cp.command == CommandParser::Command::Wake )
  {
    hard_reset_state( E_ACCEPT_COMMANDS, 0 );
    push_state( E_AWAKEN );
  }
}

unsigned int FOCUSER_STATE::state_check_for_abort() 
{
  DebugInterface& log = *debugLog;

  state_stack.pop_back();
  auto cp = CommandParser::checkForCommands( log, *net );

  if ( cp.command != CommandParser::Command::NoCommand )
  {
    processCommand( cp );
    return 0;
  }
  return 0;
}
  
unsigned int FOCUSER_STATE::state_accept_commands()
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

unsigned int FOCUSER_STATE::state_low_power()
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

unsigned int FOCUSER_STATE::state_set_dir()
{
  FOCUSER_STATE::COMMAND_PACKET& state = get_current_command();
  Dir desiredDir = state.arg0 ? Dir::FORWARD : Dir::REVERSE;
  if ( desiredDir != dir )
  {
    dir = desiredDir;
    if ( dir == Dir::FORWARD )
      hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_FORWARD); 
    if ( dir == Dir::REVERSE )       
      hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_BACKWARD );
  }    

  state_stack.pop_back();
  return 0;
}

unsigned int FOCUSER_STATE::state_step_low_and_wait()
{
  int delay = 1000000 / focuser_speed / 2;
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_LOW );
  state_stack.pop_back();
  return delay;
}

unsigned int FOCUSER_STATE::state_step_high_and_wait()
{
  int delay = 1000000 / focuser_speed / 2;
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_HIGH );
  state_stack.pop_back();
  return delay;
}

unsigned int FOCUSER_STATE::state_doing_steps()
{
  FOCUSER_STATE::COMMAND_PACKET& state = get_current_command();
    
  if ( state.arg0 == 0 )
  {
    // We're done at 0
    state_stack.pop_back();
    return 0;
  }

  push_state( E_STEPPER_HIGH_AND_WAIT );
  push_state( E_STEPPER_LOW_AND_WAIT );

  state.arg0--;
  return 0;  
}

unsigned int FOCUSER_STATE::state_moving()
{
  if ( motorState != MotorState::ON )
  {
    hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );
  }
          
  WifiDebugOstream log( debugLog.get(), net.get() );
  log << "Moving " << focuser_position << "\n";
  
  FOCUSER_STATE::COMMAND_PACKET& state = get_current_command();

  if ( state.arg0 == focuser_position ) {
    // We're at the target,  exit
    state_stack.pop_back();
    return 0;    
  }

  bool next_dir;
  int steps = state.arg0 - focuser_position;
  
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
  
  push_state( E_CHECK_FOR_ABORT );
  push_state( E_DO_STEPS, steps );
  push_state( E_SET_DIR, next_dir );
  return 0;        
}

unsigned int FOCUSER_STATE::state_stop_at_home()
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
  push_state( E_DO_STEPS, 1 );
  push_state( E_SET_DIR, 0 );
  focuser_position--;
  return 0;        
}

unsigned int FOCUSER_STATE::state_awaken()
{
  hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );        
  state_stack.pop_back();
  return 0;
}

unsigned int FOCUSER_STATE::loop(void)
{
  STATE next_state = get_current_command().state;

  switch ( next_state ) {
    case E_CHECK_FOR_ABORT:
      return state_check_for_abort();
      break;
    case E_ACCEPT_COMMANDS:
      return state_accept_commands();
      break;
    case E_DO_STEPS:
      return state_doing_steps();
      break;
    case E_SET_DIR:
      return state_set_dir();
      break;      
    case E_MOVING:
      return state_moving();
      break;
    case E_STOP_AT_HOME:
      return state_stop_at_home();
      break;      
    case E_LOW_POWER:
      return state_low_power();
      break;      
    case E_AWAKEN:
      return state_awaken();
      break;
    case E_STEPPER_LOW_AND_WAIT:      
      return state_step_low_and_wait();
      break;
    case E_STEPPER_HIGH_AND_WAIT:      
      return state_step_high_and_wait();
      break;
    default: 
      throw std::out_of_range("bad case value");
  }
  return 10*1000;   // 10 microseconds
}

