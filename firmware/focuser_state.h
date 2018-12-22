#ifndef __FOCUSER_STATE_H__
#define __FOCUSER_STATE_H__

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "net_interface.h"
#include "hardware_interface.h"
#include "command_parser.h"

namespace FS {

/// @brief Focuser's State Enum
///
/// TODO - describe state machine operation in one place, probably
///        in the Focuser class description
///
enum class State {
  START_OF_STATES = 0,        ///< Start of States
  ACCEPT_COMMANDS = 0,        ///< Accepting commands from the net interface
  DO_STEPS,                   ///< Doing n Stepper Motor Steps
  STEPPER_INACTIVE_AND_WAIT,  ///< Set Stepper to Inactive and Pause
  STEPPER_ACTIVE_AND_WAIT,    ///< Set Stepper to Active and Pause
  SET_DIR,                    ///< Set the Direction Pin
  MOVING,                     ///< Move to an absolute position
  STOP_AT_HOME,               ///< Rewind until the Home input is active
  ERROR_STATE,                ///< Error Errror Error
  END_OF_STATES               ///< End of States
};

/// @brief Increment operator for State enum
inline State& operator++( State &s )
{
  return BeeFocus::advance< State, State::END_OF_STATES>(s);
}

using StateToString = std::unordered_map< State, const std::string, EnumHash >;
using CommandToBool = std::unordered_map< CommandParser::Command, bool, EnumHash >;

extern const StateToString stateNames;
///
/// @brief Does a particular incoming command interrupt the current state
///
/// Example 1.  A "Status" Command will not interrupt a move sequence
/// Example 2.  A "Home" Command will interrupt a focuser's move sequence
///
extern const CommandToBool doesCommandInterrupt;

class CStack {
  public:

  CStack()
  {
    reset();
  }

  void reset( void )
  {
    while ( !stateStack.empty() )
    {  
      pop();
    }
    push( State::ACCEPT_COMMANDS, 0 );
  }

  State topState( void )
  {
    if ( stateStack.empty() )
    {
      // bug, should never happen.
      push( State::ERROR_STATE, __LINE__ ); 
    }     
    return stateStack.back().state;
  }
  int& topArg( void )
  {
    if ( stateStack.empty() )
    {
      // bug, should never happen.
      push( State::ERROR_STATE, __LINE__ ); 
    }     
    return stateStack.back().arg0;
  }
  void pop( void )
  {
    stateStack.pop_back();
  }
  void push( State new_state, int arg0 = -1  )
  {
    stateStack.push_back( { new_state, arg0 } );
  }  
  
  private:

  typedef struct 
  {
    State state;   
    int arg0; 
  } CommandPacket;

  std::vector< CommandPacket > stateStack;
};

class Focuser 
{
  public:
 
  /// @brief Focuser State Constructor
  ///
  /// @param[in] netArg       - Interface to the network
  /// @param[in] hardwareArg  - Interface to the Hardware
  /// @param[in] debugArg     - Interface to the debug logger.
  ///
  Focuser( 
		std::unique_ptr<NetInterface> netArg,
		std::unique_ptr<HWI> hardwareArg,
		std::unique_ptr<DebugInterface> debugArg
	);

  /// @brief Deleted copy constructor
  Focuser( const Focuser& other ) = delete;
  /// @brief Deleted default constructor
  Focuser() = delete;
  /// @brief Deleted assignment operator
  Focuser& operator=( const Focuser& ) = delete;
  
  ///
  /// @brief Update the Focuser's State
  ///
  /// @return The amount of time the caller should wait (in microsecodns)
  ///         before calling loop again.
  ///
  unsigned int loop();

  ///
  /// @brief Set the maximum number of steps we'll do at a time
  ///
  void setMaxStepsToDoAtOnce( int maxSteps )
  {
    doStepsMax = maxSteps; 
  }

  static const std::unordered_map<CommandParser::Command,
    void (Focuser::*)( CommandParser::CommandPacket),EnumHash> 
    commandImpl;


  using ptrToMember = unsigned int ( Focuser::*) ( void );

  static const std::unordered_map< State, ptrToMember, EnumHash > stateImpl;

  private:

  CStack stateStack;

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
  /// @brief Rewind the focuser until the home input is active.
  unsigned int stateStopAtHome( void );
  /// @brief If we land in this state, complain a lot.
  unsigned int stateError( void );

  void doAbort( CommandParser::CommandPacket );
  void doHome( CommandParser::CommandPacket );
  void doPStatus( CommandParser::CommandPacket );
  void doSStatus( CommandParser::CommandPacket );
  void doHStatus( CommandParser::CommandPacket );
  void doABSPos( CommandParser::CommandPacket );
  void doError( CommandParser::CommandPacket );

  std::unique_ptr<NetInterface> net;
  std::unique_ptr<HWI> hardware;
  std::unique_ptr<DebugInterface> debugLog;
  
  enum class Dir {
    FORWARD,    ///< Go Forward
    REVERSE     ///< Go Backward
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


  /// @brief The number of steps to move before we check for new status
  int doStepsMax;

  /// @brief Is the focuser homed?
  bool isHomed;
};

}

#endif

