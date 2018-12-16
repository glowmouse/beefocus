#ifndef __FOCUSER_STATE__
#define __FOCUSER_STATE__

#include <vector>
#include <memory>
#include "net_interface.h"
#include "hardware_interface.h"
#include "command_parser.h"

class FOCUSER_STATE 
{
  public:
 
  FOCUSER_STATE( 
		std::unique_ptr<NetInterface> netArg,
		std::unique_ptr<HWI> hardwareArg,
		std::unique_ptr<DebugInterface> debugArg
	);
  FOCUSER_STATE( const FOCUSER_STATE& other ) = delete;

  // @brief Update State
  //
  // @return The amount of time the caller should wait (in microsecodns)
  //         before calling loop again.
  unsigned int loop();

  private:

  // The focuser state's expressed as a set of state machines.  There's a simple
  // stack that can be used to push and pop state.
  
  typedef enum {
    E_CHECK_FOR_ABORT,
    E_ACCEPT_COMMANDS,
    E_ERROR_STATE,  
    E_DO_STEPS,
    E_STEPPER_INACTIVE_AND_WAIT,
    E_STEPPER_ACTIVE_AND_WAIT,
    E_SET_DIR,
    E_MOVING,
    E_STOP_AT_HOME,
    E_LOW_POWER,
    E_AWAKEN,
    E_END          
  } STATE;

  class COMMAND_PACKET {
    public:
    COMMAND_PACKET( STATE arg_state, int arg_arg0, int arg_arg1 ) : state{ arg_state }, arg0{ arg_arg0 }, arg1{ arg_arg1} {}
    COMMAND_PACKET( STATE arg_state, int arg_arg0 ) : state{ arg_state }, arg0{ arg_arg0 }, arg1{ -1 } {}
    COMMAND_PACKET() : state{ E_ERROR_STATE}, arg0{ __LINE__ }, arg1{ -1 } {}
    STATE state;   
    int arg0; 
    int arg1;
  };

  void hard_reset_state( STATE, int argument );
  void push_state( STATE new_state, int arg0 = -1,  int arg1 = -1 );
  void processCommand( CommandParser::CommandPacket cp );

  unsigned int state_check_for_abort( void );
  unsigned int state_accept_commands( void );  
  unsigned int state_moving( void );
  unsigned int state_doing_steps( void );
  unsigned int state_set_dir( void );
  unsigned int state_stop_at_home( void );
  unsigned int state_low_power( void );
  unsigned int state_awaken( void );
  unsigned int state_step_active_and_wait( void );
  unsigned int state_step_inactive_and_wait( void );

  COMMAND_PACKET& get_current_command( void );

  std::unique_ptr<NetInterface> net;
  std::unique_ptr<HWI> hardware;
  std::unique_ptr<DebugInterface> debugLog;
  
  enum class Dir {
    FORWARD,
    REVERSE
  };

  /// @brief What direction are we going? FORWARD = counting up.
  Dir dir;

  enum class MotorState {
    ON,
    OFF
  };

  /// @brief Is the Stepper Motor On or Off. 
  MotorState motorState;

  int focuser_position;       // Stepper Motor Position
  int focuser_speed;          // For the time being, Stepper Motor Positions/ Second
  bool motor_on;              // Is the stepper motor powered

  std::vector< COMMAND_PACKET > state_stack;
  const char *state_names[ E_END ];  

  const int steps_per_rotation = 200;
  const int max_rotations_per_second = 2;
  const int NO_VALUE = -1;

};

#endif

