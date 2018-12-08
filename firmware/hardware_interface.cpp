#include "hardware_interface.h"

const std::unordered_map<HardwareInterface::Pin,std::string,EnumHash> HardwareInterface::pinNames = {
    { Pin::STEP,       "Step" },
    { Pin::DIR,        "Direction" },
    { Pin::MOTOR_ENA,  "Motor Enable" },
    { Pin::HOME,       "Home" },
};


