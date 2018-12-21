#include <gtest/gtest.h>

#include "hardware_interface.h"
#include "wifi_secrets.h"

/// @brief Keep developers from committing their passwords
TEST( DEVICE, should_not_leak_wifi_secrets )
{
  ASSERT_STREQ( WifiSecrets::ssid, "yourssid" );
  ASSERT_STREQ( WifiSecrets::password, "yourpassword" );
}

/// @brief Every Pin in the Pins enum should have a debug name
TEST( DEVICE, should_have_complete_pin_names )
{
  for( HWI::Pin pin = HWI::Pin::START_OF_PINS; 
       pin < HWI::Pin::END_OF_PINS;
       ++pin )
  {
    ASSERT_NE(HWI::pinNames.find( pin ), HWI::pinNames.end() );
  }
}

/// @brief Every Pin State the PinsState enum should have a debug name
TEST( DEVICE, should_have_complete_pin_state_names )
{
  for( HWI::PinState pinState = HWI::PinState::START_OF_PIN_STATES; 
       pinState < HWI::PinState::END_OF_PIN_STATES;
       ++pinState )
  {
    ASSERT_NE(HWI::pinStateNames.find( pinState ), HWI::pinStateNames.end() );
  }
}

/// @brief Every IO Mode should have a debug name
TEST( DEVICE, should_have_complete_pin_io_modes )
{
  for( HWI::PinIOMode i= HWI::PinIOMode::START_OF_PIN_IO_MODES; 
       i < HWI::PinIOMode::END_OF_IO_MODES;
       ++i )
  {
    ASSERT_NE(HWI::pinIOModeNames.find( i ), HWI::pinIOModeNames.end() );
  }
}


