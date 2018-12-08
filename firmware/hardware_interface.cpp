#include "hardware_interface.h"

const std::unordered_map<HWI::Pin,std::string,EnumHash> HWI::pinNames = {
    { Pin::STEP,       "Step" },
    { Pin::DIR,        "Direction" },
    { Pin::MOTOR_ENA,  "Motor Enable" },
    { Pin::HOME,       "Home" },
};


