#ifndef __COMMAND_PARSER_H__
#define __COMAMND_PARSER_H__

#include "basic_types.h"
#include "hardware_interface.h"
#include "debug_interface.h"

class NetInterface;

namespace CommandParser {

	class Deltas {
		public:

		Deltas() :
			position_changed{false},
			position_changed_arg{0},
			new_abort{false},
			new_home{false},
			new_sleep{false},
			new_awaken{false} 
		{	}

		public:

    /// @brief true if the client requested a position change, false otherwise
		bool position_changed;
    /// @brief if position_change is true, position_change_arg is the new
    ///        position.
		int  position_changed_arg;
    /// @brief True if the client requested an abort, false otherwise.
		bool new_abort;
    /// @brief True if the client requested a home, false otherwise
		bool new_home;
    /// @brief True if the client wants to go into low power mode
		bool new_sleep;
    /// @brief True if the client wants to go to wake from low power mode
		bool new_awaken;
	};

  /// @brief Get commands from the network interface
  ///
  /// @param[in] log          - Debug Log stream
  /// @param[in] netInterface - The network interface that we'll query
  ///            for the command.
  /// @return    New requests from netInterface that need to be acted
  ///            on.
  ///
  /// TODO 
  /// - Error handling (has none).
  /// - Move extra parameters used by the STATUS command.
  ///
  const Deltas checkForCommands( 
    DebugInterface& log,				// Input: Debug Log Strem
    NetInterface& netInterface,	// Input: Network Interface
    int focuser_position,  			// Input: For STATUS command.  TODO, move
    const char *state, 					// Input: For STATUS command.  TODO, move
    int state_arg  							// Input: For STATUS command.  TODO, move
  );

};


#endif

