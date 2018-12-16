
#include <memory>
#include "focuser_state.h"
#include "net_esp8266.h"
#include "hardware_esp8266.h"
#include "debug_esp8266.h"

std::unique_ptr<FOCUSER_STATE> focuser;

void loop() {
  unsigned int pause = focuser->loop();
  if ( pause != 0 )
  {
    delayMicroseconds( pause );
  }
}

void setup() {
  std::unique_ptr<NetInterface> wifi( new WifiInterfaceEthernet );
  std::unique_ptr<HWI> hardware( new HardwareESP8266 );
  std::unique_ptr<DebugInterface> debug( new DebugESP8266 );
  focuser = std::unique_ptr<FOCUSER_STATE>(
     new FOCUSER_STATE( 
        std::move(wifi), 
        std::move(hardware),
				std::move(debug) )
  );
}

