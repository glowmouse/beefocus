#ifndef __FOCUSER_STATE__
#define __FOCUSER_STATE__

#include <vector>
#include <memory>
#include "net_interface.h"
#include "hardware_interface.h"

class FOCUSER_STATE 
{
  public:
 
  FOCUSER_STATE( 
		std::unique_ptr<NetInterface> netArg,
		std::unique_ptr<HardwareInterface> hardwareArg,
		std::unique_ptr<DebugInterface> debugArg
	)
  {
    std::swap( net, netArg );
    std::swap( hardware, hardwareArg );
    std::swap( debugLog, debugArg );
  }
  FOCUSER_STATE( const FOCUSER_STATE& other ) = delete;

  void setup( );
  void loop();

  private:

  // The focuser state's expressed as a set of state machines.  There's a simple
  // stack that can be used to push and pop state.
  
  typedef enum {
    E_CHECK_FOR_ABORT,
    E_ACCEPT_COMMANDS,
    E_ERROR_STATE,  
    E_DO_STEPS,
    E_SET_DIR,
    E_MOVING,
    E_STOP_AT_HOME,
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
  void check_for_commands( bool accept_only_abort );

  void state_check_for_abort( void );
  void state_accept_commands( void );  
  void state_error( void );
  void state_moving( void );
  void state_doing_steps( void );
  void state_set_dir( void );
  void state_stop_at_home( void );

  COMMAND_PACKET& get_current_command( void );

  std::unique_ptr<NetInterface> net;
  std::unique_ptr<HardwareInterface> hardware;
  std::unique_ptr<DebugInterface> debugLog;
  
  int focuser_position;       // Stepper Motor Position
  int focuser_speed;          // For the time being, Stepper Motor Positions/ Second
  bool dir;                   // Direction,  true = up,  false = down
  
  std::vector< COMMAND_PACKET > state_stack;
  const char *state_names[ E_END ];  

  const int dirPin = 5; // GPIO5 of ESP8266
  const int stepPin = 4; // GPIO4 of ESP8266
  const int enaPin = 6;
  const int homePin = 13;
  const int steps_per_rotation = 200;
  const int max_rotations_per_second = 2;
  const int NO_VALUE = -1;

};

#endif

