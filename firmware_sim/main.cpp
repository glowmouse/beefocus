
#include <iostream>
#include <memory>
#include <unistd.h>

#include "focuser_state.h"
#include "hardware_interface.h"

std::unique_ptr<FOCUSER_STATE> focuser;

class NetInterfaceSim: public NetInterface {
  public:

  void reset() override {}
  void setup( DebugInterface& debugLog ) override
  {
    debugLog << "Simulator Net Interface Init\n";
  }
  bool getString( WifiDebugOstream& log, std::string& input ) override
  {
    fd_set readfds;
    FD_ZERO(&readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_SET(STDIN_FILENO, &readfds );
    if ( select(1, &readfds, nullptr, nullptr, &timeout ))
    {
      std::cin >> input;
      return true;
    }
    input = "";
    return false;
  }
  NetInterface& operator<<( char c )
  {
    std::cout << c;
    return *this;
  }
};

class HWISim: public HWI
{
  public: 

  void PinMode( Pin pin, PinIOMode mode ) override
  {
    std::cout << "PM (" << HWI::pinNames.at(pin) << ") = " << HWI::pinIOModeNames.at(mode) << "\n";
  }
  void DigitalWrite( Pin pin, PinState state ) override
  {
    const std::string name = HWI::pinNames.at(pin);
    std::cout << "DW (" << HWI::pinNames.at(pin) 
              << ") = " << HWI::pinStateNames.at( state ) 
              << "\n";
  }
  PinState DigitalRead( Pin pin ) override
  {
    std::cout << "DR " << HWI::pinNames.at(pin) << " returning HOME_INACTIVE";
    return HWI::PinState::HOME_INACTIVE;
  }
};

class DebugInterfaceSim: public DebugInterface
{
  void rawWrite( const char* bytes, std::size_t numBytes ) override
  {
    // Ignore for now.
    //for ( int i=0; i<numBytes; ++i )
    //{
    //  std::cout << bytes[i];
    //}
  }
};

void loop() {
  focuser->loop();
}

void setup() {
  std::unique_ptr<NetInterface> wifi( new NetInterfaceSim );
  std::unique_ptr<HWI> hardware( new HWISim );
  std::unique_ptr<DebugInterface> debug( new DebugInterfaceSim );
  focuser = std::unique_ptr<FOCUSER_STATE>(
     new FOCUSER_STATE( 
        std::move(wifi), 
        std::move(hardware),
				std::move(debug) )
  );
}

int main(int argc, char* argv[])
{
  setup();
  for ( ;; ) 
  {
    loop();
  }
}

