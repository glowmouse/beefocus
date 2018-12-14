#include "hardware_interface.h"

const std::unordered_map<HWI::Pin,std::string,EnumHash> HWI::pinNames = {
    { Pin::STEP,       "Step" },
    { Pin::DIR,        "Direction" },
    { Pin::MOTOR_ENA,  "Motor Enable" },
    { Pin::HOME,       "Home" },
};

const std::unordered_map<HWI::PinState,std::string,EnumHash> HWI::pinStateNames = {
    { PinState::STEP_HIGH,       "Step High" },
    { PinState::STEP_LOW,        "Step Low" },
    { PinState::DIR_FORWARD,     "Dir Firward" },
    { PinState::DIR_BACKWARD,    "Dir Backward" },
    { PinState::MOTOR_ON,        "Motor On" },
    { PinState::MOTOR_OFF,       "Motor Off" },
    { PinState::HOME_ACTIVE,     "Home Active" },
    { PinState::HOME_INACTIVE,   "Home Inactive" },
};

const std::unordered_map<HWI::PinIOMode,std::string,EnumHash> HWI::pinIOModeNames = {
    { PinIOMode::M_INPUT,        "Input" },
    { PinIOMode::M_OUTPUT,       "Output" }
};

