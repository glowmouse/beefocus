
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

void FOCUSER_STATE::setup()
{  
  DebugInterface& log = *debugLog;
  log << "Bringing up net interface\n";
  
  // Bring up the interface to the controlling computer

  net->setup( log );

  focuser_position = 0;

  // rotate 1 turn / second for now
  
  focuser_speed = steps_per_rotation * 2;  

  // increase count when we rotate

  dir = true;
  
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


  
  hardware->PinMode(stepPin, HardwareInterface::output );  
  hardware->PinMode(dirPin,  HardwareInterface::output );  
  hardware->PinMode(homePin, HardwareInterface::input  );  

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



void FOCUSER_STATE::check_for_commands( bool accept_only_abort )
{
  DebugInterface& log = *debugLog;
  int new_speed = 0;
  int new_position = 0;
  bool new_abort = 0;
  bool new_home = 0;

  STATE state = get_current_command().state;
  int arg0 = get_current_command().arg0;

  CommandParser::checkForCommands( log, *net, focuser_position, state_names[ state ], arg0, new_speed, new_position, new_abort, new_home  );

  if ( new_abort ) {
    // Abort command, unroll the state stack and start accepting commands.
    hard_reset_state( E_ACCEPT_COMMANDS, 0 );
    return;
  }

  if (( new_speed != NO_VALUE || new_position != NO_VALUE ) && accept_only_abort ) {   
    return;
  }

  if ( new_home ) {
    hard_reset_state( E_ACCEPT_COMMANDS, 0 );
    push_state( E_STOP_AT_HOME );
    return;
  }  

  if ( new_speed != NO_VALUE ) {
    if ( new_speed <=1 || new_speed >= max_rotations_per_second ) {
      hard_reset_state( E_ERROR_STATE, __LINE__ );
      return;
    }
  }
  if ( new_position != NO_VALUE ) {
    push_state( E_MOVING, new_position );
        
    if ( new_position < focuser_position )
    {
      int backtrack = new_position - 500;
      backtrack = backtrack < 0 ? 0 : backtrack;
      push_state( E_MOVING, backtrack );
    }
  }
}

void FOCUSER_STATE::state_check_for_abort() 
{
  // An abort check is always a one off
  state_stack.pop_back();

  bool accept_only_abort = true;
  check_for_commands( accept_only_abort );
}
  
void FOCUSER_STATE::state_accept_commands()
{
  bool dont_accept_only_abort = false;
  check_for_commands( dont_accept_only_abort );
}

void FOCUSER_STATE::state_error()
{
  bool accept_only_abort = true;  
  check_for_commands( accept_only_abort );
}

void FOCUSER_STATE::state_set_dir()
{
  FOCUSER_STATE::COMMAND_PACKET& state = get_current_command();  
  if ( state.arg0 ) {
    dir = true;
    hardware->DigitalWrite( dirPin, HardwareInterface::high );        
  }
  else {
    dir = false;
    hardware->DigitalWrite( dirPin, HardwareInterface::low );        
  }    

  state_stack.pop_back();
}

void FOCUSER_STATE::state_doing_steps()
{
  FOCUSER_STATE::COMMAND_PACKET& state = get_current_command();
    
  if ( state.arg0 == 0 )
  {
    // We're done at 0
    state_stack.pop_back();
    return;
  }

  int delay = 1000000 / focuser_speed / 2;
  hardware->DigitalWrite( stepPin, HardwareInterface::low );
  hardware->DelayMicroseconds( delay );
  hardware->DigitalWrite( stepPin, HardwareInterface::high );
  hardware->DelayMicroseconds( delay );
  state.arg0--;  
}

void FOCUSER_STATE::state_moving()
{
  WifiDebugOstream log( debugLog.get(), net.get() );
  log << "Moving " << focuser_position << "\n";
  
  FOCUSER_STATE::COMMAND_PACKET& state = get_current_command();

  if ( state.arg0 == focuser_position ) {
    // We're at the target,  exit
    state_stack.pop_back();
    return;    
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
}

void FOCUSER_STATE::state_stop_at_home()
{
  WifiDebugOstream log( debugLog.get(), net.get() );

  if ( (focuser_position % 100) == 0 )
  {
    log << "Homing " << focuser_position << "\n";
  }
  if ( hardware->DigitalRead( homePin ) == 0 ) {
    log << "Hit home at position " << focuser_position << "\n";
    log << "Resetting position to 0\n";
    focuser_position = 0;
    state_stack.pop_back();
    return;        
  }
  push_state( E_DO_STEPS, 1 );
  push_state( E_SET_DIR, 0 );
  focuser_position--;
}

void FOCUSER_STATE::loop(void)
{
  STATE next_state = get_current_command().state;

  switch ( next_state ) {
    case E_CHECK_FOR_ABORT:
      state_check_for_abort();
      break;
    case E_ACCEPT_COMMANDS:
      state_accept_commands();
      hardware->Delay(10);
      break;
    case E_DO_STEPS:
      state_doing_steps();
      break;
    case E_SET_DIR:
      state_set_dir();
      break;      
    case E_MOVING:
      state_moving();
      break;
    case E_STOP_AT_HOME:
      state_stop_at_home();
      break;      
    case E_ERROR_STATE:
    default:    
      state_error();
      break;
  }
}

