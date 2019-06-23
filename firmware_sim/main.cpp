
#include <iostream>
#include <memory>
#include <unistd.h>

#include "focuser_state.h"
#include "hardware_interface.h"

std::unique_ptr<FS::Focuser> focuser;

class NetInterfaceSim: public NetInterface {
  public:

  struct category: virtual beefocus_tag {};
  using char_type = char;

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
  std::streamsize write( const char_type* s, std::streamsize n ) override
  {
    for ( std::streamsize i = 0; i < n; ++i ) {
      std::cout << s[i];
    }
  }
  void flush() override
  {
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
  struct category: virtual beefocus_tag {};
  using char_type = char;

  std::streamsize write( const char_type* s, std::streamsize n )
  {
    // Ignore for now.
    return n;
  }
  void disable() override 
  {
    // Can't disable what we're ignoring.
  }
};

void loop() {
  focuser->loop();
}

void setup() {
  std::unique_ptr<NetInterface> wifi( new NetInterfaceSim );
  std::unique_ptr<HWI> hardware( new HWISim );
  std::unique_ptr<DebugInterface> debug( new DebugInterfaceSim );
  FS::BuildParams params( FS::Build::LOW_POWER_HYPERSTAR_FOCUSER );
  focuser = std::unique_ptr<FS::Focuser>(
     new FS::Focuser( 
        std::move(wifi), 
        std::move(hardware),
				std::move(debug),
        params )
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

