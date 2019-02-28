Beefocused
==========

Beefocused is an Open Source Hardware/ Software Telescope Focuser
The goal of the project is to create a high quality DIY focuser that's
relatively easy to build and modify.

Installation
------------

- Beefocused can only be built from source right now.  If you're interested in
  building one send an email to andrew.brownbill@gmail.com

Features
--------

- Beefocused talks to the computer that runs INDI using Wifi.  The advantage
  of this is that there's one less wire to worry about.  It does mean that
  your telescope needs a Wifi hotspot,  but that's becoming more common now
  that people are running telescopes from compute sticks and Raspberry Pis
  mounting on the telescope itself.
- The default built uses a Nema 14 Stepper motor for positioning - these motors
  are widely used by 3D printers.  The Motor's step rate is 1.8 degrees 
  (200 steps / rotation), so it's reasonably accurate.
- The electronic parts for a Beefocused will cost you about $25 on ebay.
  That includes the Nema 14 Stepper Motor. 
- Beefocus focusers operate as either Absolute or Relative focusers,
  dependong on the build.  The focuser supports either automatic homing at
  start up or manual synching.

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

Installing Firmware 
-------------------



