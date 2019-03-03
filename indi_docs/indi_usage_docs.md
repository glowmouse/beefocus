Beefocused
==========

Beefocused is an Open Source Hardware/ Software Telescope Focuser
The goal of the project is to create a high quality DIY focuser that's
relatively easy to build and modify.

Installation
------------

- Beefocused can only be built from source right now (and only on Indi -
  https://indilib.org/ ) - ASCOM drivers haven't been written yet.  
  If you're interested in building one send an email to 
  andrew.brownbill@gmail.com

Features
--------

- Beefocused talks to the INDI computer using WiFi.  This is nice because
  it means there's one less wire on the telescope to worry about.  It does 
  mean that the telescope needs a WiFi hotspot,  but that's becoming more 
  common now that people are running telescopes from compute sticks and 
  Raspberry Pis that are mounted directly onto the scope.
- The default built uses a Nema 14 Stepper motor for positioning - these motors
  are widely used by 3D printers.  The Motor's step rate is 1.8 degrees 
  (200 steps / rotation), so it's reasonably accurate.
- The electronic parts for a Beefocused will cost you about $25 on eBay.
  That includes the Nema 14 Stepper Motor. 
- Beefocus focusers operate as either Absolute or Relative focusers,
  depending on the build.  The focuser supports either automatic homing at
  start up or manual syncing.

Using the Simulated Focuser
---------------------------

The Beefocused Indi driver has a "Simulation" mode that's very close to what
you'll see if you use the actual hardware.  Simulation mode is selected by
going to the BeeFocused connection tab and selecting a Simulated connection

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim.png "Selecting a Simulated Connection")

You'll need to go to the Main focuser menu and "connect" to the Simulated
Focuser, the way you'd connect to an actual focuser.

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_main.png "Simulated Focuser Connection Menu")

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_main_online.png "Simulated Focuser Online")

You can start moving the simulated focuser by pressing the Absolute Position
Set Button.  The Focuser Status message box will change from Ready to Moving,
and the Absolute Position of the focuser will begin changing.

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_main_moving.png "Simulated Focuser Moving")

A move can be interrupted by pressing the Abort button.  If you try to move
the focuser past the Maximum Position the driver will generate an Error
message

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_main_move_oob.png "Out of bounds error message")

If the focuser's hardware supports a home switch the focuser will 
automatically move to it's home, or zero position, the first time INDI 
connects.  The sync switch can also be used to set the Focuser's current
absolute position.  In this example the Sync switch is used to reset the
absolute position to 2000.
 
![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_abs_dst_sync_before.png "Sync Before")
![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_abs_dst_sync_after.png "Sync After")

The focuser can move relative to it's current position.  In this mode the
user changes the Relative Position field number to the number of positions they
want the focuser to move (2000 in this example) and presses set.  Pressing
Set again will cause the focuser to move again.

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_main_rel_move.png "Relative Move +2000")

The Focus In and Focus Out buttons control the direction of relative movements.
In the last example the Focus Out switch was active. In the example the 
Focus In switch was active and then the Relative Position Set button was 
pressed twice.  The Focuser moved by -2000 each time.

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/indi_docs/con_sim_main_rel_move_in.png "Relative Move -2000")
 

Installing Firmware 
-------------------

TODO


