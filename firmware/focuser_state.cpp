
#include <iterator>
#include <vector>
#include <string>
#include <memory>
#include "command_parser.h"
#include "wifi_debug_ostream.h"
#include "focuser_state.h"

using namespace FS;

const StateToString FS::stateNames =
{
  { State::ACCEPT_COMMANDS,               "ACCEPTING_COMMANDS" },
  { State::DO_STEPS,                      "DO_STEPS"           },
  { State::STEPPER_INACTIVE_AND_WAIT,     "STEPPER_INACTIVE"   },
  { State::STEPPER_ACTIVE_AND_WAIT,       "STEPPER_ACTIVE"     },
  { State::SET_DIR,                       "SET_DIR"            },
  { State::MOVING,                        "MOVING"             },
  { State::STOP_AT_HOME,                  "STOP_AT_HOME"       },
  { State::ERROR_STATE,                   "ERROR ERROR ERROR"  },
};

const CommandToBool FS::doesCommandInterrupt= 
{
  { CommandParser::Command::Abort,         true   },
  { CommandParser::Command::Home,          true   },
  { CommandParser::Command::PStatus,       false  },
  { CommandParser::Command::SStatus,       false  },
  { CommandParser::Command::HStatus,       false  },
  { CommandParser::Command::ABSPos,        true   },
  { CommandParser::Command::NoCommand,     false  },
};

const std::unordered_map<CommandParser::Command,
  void (Focuser::*)( CommandParser::CommandPacket),EnumHash> 
  Focuser::commandImpl = 
{
  { CommandParser::Command::Abort,      &Focuser::doAbort },
  { CommandParser::Command::Home,       &Focuser::doHome },
  { CommandParser::Command::PStatus,    &Focuser::doPStatus },
  { CommandParser::Command::SStatus,    &Focuser::doSStatus },
  { CommandParser::Command::HStatus,    &Focuser::doHStatus },
  { CommandParser::Command::ABSPos,     &Focuser::doABSPos },
  { CommandParser::Command::NoCommand,  &Focuser::doError },
};

const std::unordered_map<State,unsigned int (Focuser::*)( void ),EnumHash>
  Focuser::stateImpl =
{
  { State::ACCEPT_COMMANDS,           &Focuser::stateAcceptCommands },
  { State::DO_STEPS,                  &Focuser::stateDoingSteps },
  { State::STEPPER_INACTIVE_AND_WAIT, &Focuser::stateStepInactiveAndWait },
  { State::STEPPER_ACTIVE_AND_WAIT,   &Focuser::stateStepActiveAndWait },
  { State::SET_DIR,                   &Focuser::stateSetDir },
  { State::MOVING,                    &Focuser::stateMoving },
  { State::STOP_AT_HOME,              &Focuser::stateStopAtHome },
  { State::ERROR_STATE,               &Focuser::stateError }
};

Focuser::Focuser(
    std::unique_ptr<NetInterface> netArg,
    std::unique_ptr<HWI> hardwareArg,
    std::unique_ptr<DebugInterface> debugArg
)
{
  doStepsMax = 50; 
  focuserPosition = 0;
  isHomed = false;

  std::swap( net, netArg );
  std::swap( hardware, hardwareArg );
  std::swap( debugLog, debugArg );
  
  DebugInterface& log = *debugLog;
  log << "Bringing up net interface\n";
  
  // Bring up the interface to the controlling computer

  net->setup( log );

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
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_INACTIVE );

  log << "Focuser is up\n";
}

void Focuser::doAbort( CommandParser::CommandPacket cp )
{
  (void) cp;
  // Do nothing - command triggers a state interrupt.
}

void Focuser::doHome( CommandParser::CommandPacket cp )
{
  (void) cp;
  stateStack.push( State::STOP_AT_HOME );
  return;
}

void Focuser::doPStatus( CommandParser::CommandPacket cp )
{
  (void) cp;
  DebugInterface& log = *debugLog;
  log << "Processing pstatus request\n";
  *net << "Position: " << focuserPosition << "\n";
}

void Focuser::doSStatus( CommandParser::CommandPacket cp )
{
  (void) cp;
  DebugInterface& log = *debugLog;

  log << "Processing sstatus request\n";
  *net << "State: " << stateNames.at(stateStack.topState()) << 
                " " << stateStack.topArg() << "\n";
  return;
}

void Focuser::doHStatus( CommandParser::CommandPacket cp )
{
  (void) cp;
  DebugInterface& log = *debugLog;

  log << "Processing hstatus request\n";
  *net << "Homed: " << (isHomed ? "YES" : "NO" ) << "\n";
  return;
}

void Focuser::doABSPos( CommandParser::CommandPacket cp )
{
  stateStack.push( State::MOVING, cp.optionalArg );
  int new_position = cp.optionalArg;

  if ( new_position < focuserPosition )
  {
    int backtrack = new_position - 500;
    backtrack = backtrack < 0 ? 0 : backtrack;
    stateStack.push( State::MOVING, backtrack );
  }
}

void Focuser::doError( CommandParser::CommandPacket cp )
{
  (void) cp;
  stateStack.push( State::ERROR_STATE, __LINE__ );   
}

void Focuser::processCommand( CommandParser::CommandPacket cp )
{
  auto function = commandImpl.at( cp.command );
  (this->*function)( cp );
}

unsigned int Focuser::stateAcceptCommands()
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

unsigned int Focuser::stateSetDir()
{
  Dir desiredDir = stateStack.topArg() ? Dir::FORWARD : Dir::REVERSE;

  stateStack.pop();

  if ( desiredDir != dir )
  {
    dir = desiredDir;
    if ( dir == Dir::FORWARD )
      hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_FORWARD); 
    if ( dir == Dir::REVERSE )       
      hardware->DigitalWrite( HWI::Pin::DIR, HWI::PinState::DIR_BACKWARD );
    // Trigger a 1ms pause so the stepper motor controller sees the
    // state change before we try to do anything.
    return 1000;
  }    

  return 0;
}

unsigned int Focuser::stateStepInactiveAndWait()
{
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_INACTIVE );
  stateStack.pop();
  return 1000;
}

unsigned int Focuser::stateStepActiveAndWait()
{
  hardware->DigitalWrite( HWI::Pin::STEP, HWI::PinState::STEP_ACTIVE );
  stateStack.pop();
  return 1000;
}

unsigned int Focuser::stateDoingSteps()
{
  if ( stateStack.topArg() == 0 )
  {
    // We're done at 0
    stateStack.pop();
    return 0;
  }
  stateStack.topArg()--;

  stateStack.push( State::STEPPER_INACTIVE_AND_WAIT );
  stateStack.push( State::STEPPER_ACTIVE_AND_WAIT );

  focuserPosition += (dir == Dir::FORWARD) ? 1 : -1;
  focuserPosition = focuserPosition >= 0 ? focuserPosition : 0;

  return 0;  
}

unsigned int Focuser::stateMoving()
{
  if ( motorState != MotorState::ON )
  {
    hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );
    return 100000;    // .1s pause to power things up.
  }
          
  WifiDebugOstream log( debugLog.get(), net.get() );
  log << "Moving " << focuserPosition << "\n";
  
  if ( stateStack.topArg() == focuserPosition ) {
    // We're at the target,  exit
    stateStack.pop();
    return 0;    
  }

  // Check for new commands
  DebugInterface& debug= *debugLog;
  auto cp = CommandParser::checkForCommands( debug, *net );

  if ( cp.command != CommandParser::Command::NoCommand )
  {
    if ( doesCommandInterrupt.at( cp.command ))
    {
      stateStack.reset();
    }
    processCommand( cp );
    if ( doesCommandInterrupt.at( cp.command ))
    {
      return 0;
    }
  }

  const int  steps        = stateStack.topArg() - focuserPosition;
  const bool nextDir      = steps > 0;    // TODO, enum, !bool
  const int  absSteps     = steps > 0 ? steps : -steps;
  const int  clippedSteps = absSteps > doStepsMax ? doStepsMax : absSteps;

  stateStack.push( State::DO_STEPS, clippedSteps );
  stateStack.push( State::SET_DIR,  nextDir );
  return 0;        
}

unsigned int Focuser::stateStopAtHome()
{
  WifiDebugOstream log( debugLog.get(), net.get() );

  if ( motorState != MotorState::ON )
  {
    hardware->DigitalWrite( HWI::Pin::MOTOR_ENA, HWI::PinState::MOTOR_ON );
    return 100000;    // .1s pause to power things up.
  }

  if ( hardware->DigitalRead( HWI::Pin::HOME ) == HWI::PinState::HOME_ACTIVE ) 
  {
    log << "Hit home at position " << focuserPosition << "\n";
    log << "Resetting position to 0\n";
    focuserPosition = 0;
    isHomed = true;
    stateStack.pop();
    return 0;        
  }

  if ( (focuserPosition % doStepsMax) == 0 )
  {
    log << "Homing " << focuserPosition << "\n";

    // Check for new commands
    DebugInterface& debug= *debugLog;
    auto cp = CommandParser::checkForCommands( debug, *net );

    if ( cp.command != CommandParser::Command::NoCommand )
    {
      if ( doesCommandInterrupt.at( cp.command ))
      {
        stateStack.reset();
      }
      processCommand( cp );
      if ( doesCommandInterrupt.at( cp.command ))
      {
        return 0;
      }
    }
  }

  stateStack.push( State::DO_STEPS, 1 );
  stateStack.push( State::SET_DIR, 0 );
  return 0;        
}

unsigned int Focuser::stateError()
{
  WifiDebugOstream log( debugLog.get(), net.get() );
  log << "hep hep hep error error error\n";
  return 10*1000*1000; // 10 sec pause 
}

unsigned int Focuser::loop(void)
{
  ptrToMember function = stateImpl.at( stateStack.topState() );
  return (this->*function)();
}

