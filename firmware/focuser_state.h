#ifndef __FOCUSER_STATE_H__
#define __FOCUSER_STATE_H__

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "net_interface.h"
#include "hardware_interface.h"
#include "command_parser.h"

class FocuserState 
{
  public:
 
  /// @brief Focuser State Constructor
  ///
  /// @param[in] netArg       - Interface to the network
  /// @param[in] hardwareArg  - Interface to the Hardware
  /// @param[in] debugArg     - Interface to the debug logger.
  ///
  FocuserState( 
		std::unique_ptr<NetInterface> netArg,
		std::unique_ptr<HWI> hardwareArg,
		std::unique_ptr<DebugInterface> debugArg
	);

  /// @brief Deleted copy constructor
  FocuserState( const FocuserState& other ) = delete;
  /// @brief Deleted default constructor
  FocuserState() = delete;
  /// @brief Deleted assignment operator
  FocuserState& operator=( const FocuserState& ) = delete;
  
  ///
  /// @brief Update the Focuser's State
  ///
  /// @return The amount of time the caller should wait (in microsecodns)
  ///         before calling loop again.
  ///
  unsigned int loop();

  /// @brief Focuser's State Enum
  ///
  /// TODO - describe state machine operation in one place, probably
  ///        in the FocuserState class description
  ///
  enum class State {
    START_OF_STATES = 0,
    ACCEPT_COMMANDS = 0,
    DO_STEPS,
    STEPPER_INACTIVE_AND_WAIT,
    STEPPER_ACTIVE_AND_WAIT,
    SET_DIR,
    MOVING,
    STOP_AT_HOME,
    LOW_POWER,
    AWAKEN,
    ERROR_STATE,
    END_OF_STATES
  };

  /// @brief Debug names for each state
  const static std::unordered_map< State, const std::string, EnumHash > stateNames;

  ///
  /// @brief Does a particular incoming command interrupt the current state
  ///
  /// Example 1.  A "Status" Command will not interrupt a move sequence
  /// Example 2.  A "Home" Command will interrupt a focuser's move sequence
  ///
  const static std::unordered_map< CommandParser::Command, bool, EnumHash > 
      doesCommandInterrupt;

  using ptrToMember = unsigned int ( FocuserState::*) ( void );

  static const std::unordered_map< State, ptrToMember, EnumHash > stateImpl;

  private:

  class COMMAND_PACKET {
    public:
    COMMAND_PACKET( State arg_state, int arg_arg0 ) : state{ arg_state }, arg0{ arg_arg0 } {}
    State state;   
    int arg0; 
  };

  void hard_reset_state( State, int argument );
  void pushState( State new_state, int arg0 = -1 );
  void processCommand( CommandParser::CommandPacket cp );

  /// @brief Wait for commands from the network interface
  unsigned int stateAcceptCommands( void ); 
  /// @brief Move to position @arg0
  unsigned int stateMoving( void );
  /// @brief Move the stepper @arg0 steps 
  unsigned int stateDoingSteps( void );
  /// @brief If needed, Change the state of the direction pin and pause
  unsigned int stateSetDir( void );
  /// @brief Set the Stepper to active (i.e., start step) and wait
  unsigned int stateStepActiveAndWait( void );
  /// @brief Set the Stepper to inactive (i.e., finish step) and wait
  unsigned int stateStepInactiveAndWait( void );

  unsigned int state_check_for_abort( void );
  unsigned int state_stop_at_home( void );
  unsigned int state_low_power( void );
  unsigned int state_awaken( void );
  unsigned int stateError( void );

  COMMAND_PACKET& top( void );

  std::unique_ptr<NetInterface> net;
  std::unique_ptr<HWI> hardware;
  std::unique_ptr<DebugInterface> debugLog;
  
  enum class Dir {
    FORWARD,
    REVERSE
  };

  /// @brief What direction are we going? 
  ///
  /// FORWARD = counting up.
  /// REVERSE = counting down.
  ///
  Dir dir;

  enum class MotorState {
    ON,
    OFF
  };

  /// @brief Is the Stepper Motor On or Off. 
  MotorState motorState;

  /// @brief What is the focuser's position of record
  int focuserPosition;

  /// @brief The focuser's state
  std::vector< COMMAND_PACKET > stateStack;
};

/// @brief Increment operator for State enum
inline FocuserState::State& operator++( FocuserState::State &s )
{
  return BeeFocus::advance< FocuserState::State, FocuserState::State::END_OF_STATES>(s);
}


#endif

