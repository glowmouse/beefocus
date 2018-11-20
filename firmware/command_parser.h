#ifndef __COMMAND_PARSER_H__
#define __COMAMND_PARSER_H__

#include "basic_types.h"
#include "hardware_interface.h"
#include "debug_interface.h"

class NetInterface;

namespace CommandParser {

  /// @brief Get commands from the network interface
  ///
  /// TODO - refactor the interface.  When I try to describe
  /// what the arguments do it comes out sounding silly
  ///
  void checkForCommands( 
    DebugInterface& log,
    NetInterface& netInterface,
    int focuser_position,  
    const char *state, 
    int state_arg,  
    int &new_speed,  
    int &new_position,  
    bool &new_abort,  
    bool &new_home  
  );

};


#endif

