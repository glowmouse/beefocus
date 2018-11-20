#include "net_interface.h"
#include "debug_interface.h"
#include "command_parser.h"
#include "wifi_debug_ostream.h"

namespace CommandParser
{

static int process_int( const std::string& string,  size_t pos )
{
  int end = string.length();
  int result = 0;
  for ( int iter = pos; iter != end; iter++ ) {
    char current = string[ iter ];
    if ( current >= '0' && current <= '9' )
      result = result * 10 + ( current - '0' );
    else
      break;
  }
  return result;
}

void checkForCommands( DebugInterface& serialLog, NetInterface& wifi, int focuser_position,  const char *state, int state_arg, int &new_speed,  int &new_position,  bool &new_abort, bool &new_home  ) 
{

  // Set defualt inits,  We'll do nothing unless we get a command

  new_speed = BeeFocus::noValue;
  new_position = BeeFocus::noValue;
  new_abort = false;
  new_home = false;

  
  WifiDebugOstream log( &serialLog, &wifi );

  // Read the first line of the request.  

  static std::string command;
  bool dataReady = wifi.getString( log, command );
  if ( !dataReady )
  {
    return;
  }

  log << "Got: " << command << "\n";

  if ( command.find("PING") == 0)
  {
    log << "Got Ping,  Sent Pong\n";
    wifi << "PONG\n";             
  }

  if ( command.find("ABORT") == 0 )
  {
    log << "Queued abort for processing\n"; 
    new_abort = true;
  }

  if ( command.find( "HOME") == 0)
  {
    log << "ACK_HOME\n";
    new_home = true;
  }

  if ( command.find( "STATUS") == 0 )
  {
    log << "Processing pstatus request\n";
    wifi << "Position: " << focuser_position << "\n";
    wifi << "State: " << state << " " << state_arg << "\n";

  }  
  if ( command.find( "PSTATUS") == 0 )
  {
    log << "Processing pstatus request\n";    
    wifi << "Position: " << focuser_position << "\n";  
  }

  if ( command.find( "SSTATUS") == 0 )
  {
    log << "Processing sstatus request\n";    
    wifi << "State: " << state << " " << state_arg << "\n";     
  }  

  if ( command.find( "ABS_POS=" ) == 0 )
  {
    new_position = process_int( command,  8 );
    log << "ACK_ABS_POS " << new_position << "\n";       
  }

#ifdef TODO
  if ( has

  int speed_index = request.indexOf("SPEED=");
  if (speed_index != -1 )
    new_speed = process_int( request,  speed_index+6 );

  int abs_pos_index = request.indexOf("ABS_POS=");
  if (abs_pos_index != -1 ) {
    // multiply by 32 to make sure we don't micro-step.  Holding a
    // micro-step seems to eat power.
    new_position = process_int( request,  abs_pos_index+8 ) * 32;
    log.print("CFC RAW_ABS_POS=");
    log.println(new_position);    
    if ( new_position < 1 ) new_position = NO_VALUE;
    log.print("CFC ABS_POS=");
    log.println(new_position);
  }

  
  int rel_pos_index = request.indexOf("REL_POS=");
  if (rel_pos_index != -1 )
    new_position = process_int( request,  rel_pos_index+8 );
    if ( new_position + focuser_position < 0 ) new_position = NO_VALUE;

#endif


}


}

