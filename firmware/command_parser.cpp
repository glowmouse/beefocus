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

const Deltas checkForCommands( 
	DebugInterface& serialLog, 
	NetInterface& wifi, 
	int focuser_position,  
	const char *state,
  int state_arg  )
{
	Deltas result;
  
  WifiDebugOstream log( &serialLog, &wifi );

  // Read the first line of the request.  

  static std::string command;
  bool dataReady = wifi.getString( log, command );
  if ( !dataReady )
  {
    return result;
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
    result.new_abort = true;
  }

  if ( command.find( "HOME") == 0)
  {
    log << "ACK_HOME\n";
    result.new_home = true;
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
    result.position_changed= true;
    result.position_changed_arg= process_int( command,  8 );
    log << "ACK_ABS_POS " << result.position_changed_arg << "\n";       
  }

  if ( command.find( "SLEEP") == 0 )
	{
    result.new_sleep = true;
		log << "Entering sleep mode\n";
	}
  if ( command.find( "WAKE") == 0 )
	{
    result.new_awaken = true;
		log << "Waking from sleep mode\n";
	}
  return result;
}


}

