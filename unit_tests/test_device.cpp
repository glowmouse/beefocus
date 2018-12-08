#include <gtest/gtest.h>

#include "wifi_secrets.h"
#include "hardware_interface.h"

/// @brief Try to keep folks from committing their passwords
TEST( DEVICE, should_not_leak_wifi_secrets )
{
  ASSERT_STREQ( WifiSecrets::ssid, "yourssid" );
  ASSERT_STREQ( WifiSecrets::password, "yourpassword" );
}

TEST( DEVICE, should_have_complete_pinnanes )
{
  for( HWI::Pin pin = HWI::Pin::START_OF_PINS; 
       pin < HWI::Pin::END_OF_PINS;
       ++pin )
  {
    ASSERT_NE(HWI::pinNames.find( pin ), HWI::pinNames.end() );
  }
}

