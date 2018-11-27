TODO before 1.0 can be announced...
===================================

Project
-------
* DONE 	Issue database instead of TODO.md file.

Hardware
--------
* DONE  Confirm the nema_14_b0 board will put the stepper motor into sleep mode to save power
* MOVED	Document steps to have the nema_14_b0 board externally made
* MOVED	Have the board externally made and document a build

Continuous Integration
----------------------
* TODO  Create a Hello World Unit test ( SSID and Password Check )
* TODO  Get Continous Integration flow working in Github
* TODO  Switch to pull only model on git hub master

Firmware 
--------
* DONE  Move SSID and Password to a file that makes people pause before adding it to git. 
* TODO  Add CMakefiles to build a device agnostic version of the firmware
* TODO  Test:  Basic framework
* TODO  Unit Test:  SSID and Password should not change
* TODO  Cleanup:  CommandParser::Deltas should only allow one state change
* TODO  Cleanup:  Pin specifics should be in the hardware interface, not in the command parser.  The current pins are Wemos D1 specificm, and specific to the Nema 14 build.  
* TODO  Cleanup:  Remove status information from CommandParser::check_for_commands.  It's at the wrong level.
* TODO  Cleanup:  Enum instead of bool in accept_only_abort argument on check_for_commands 
* DONE  Sleep mode in the firmware.

Device Drivers - INDI
---------------------
* TODO  Move current prototype driver to 3rd Party

Device Drivers - ASCOMM
-----------------------

Documentation
-------------

