Beefocus
========

Beefocus is an Open Source Hardware/ Software Telescope Focuser 
The goal of the project is to create a high quality DIY focuser that's
relatively easy to build and modify.

It's similar in spirit to other open source focuser projects.  For
example...

- https://sourceforge.net/projects/arduinoascomfocuserpro2diy/
- http://www.scopefocus.info/arduino-focuser-2
- https://sourceforge.net/projects/arduinomoonlitefocuserclone/?source=directory
- https://github.com/aruangra/PnP-Focus

Why start a new project?
------------------------

- **Originally, I needed a custom focuser solution for my telescope**.
  I have a Celestron C11 with a Hyperstar.  When I started this I
  couldn't find a good out of the box solution.
- **I want to try using an ESP-8266 internet of things micro-controller 
  instead of a traditional arduino**.  I've found that every wire you 
  get rid of simplifies your telescope setup and observing
- **I want to open-source the hardware design under a permissive license.**   
  I'm intrigued by the idea of open-source hardware.  I'm including the
  Eagle CAD files for the hardware under a Creative Commons 4.0 license.
  The goal is to make builds as inexpensive and easy as possible.
- This project is a good personal learning experience.

Code Documentation
------------------

Documentation can be found at https://glowmouse.github.io/beefocus/ 

Status March 2019
-----------------

- I have a physical prototype. It's been field tested and
  it worked almost flawlessly.  The issues that I want to fix before
  "Version 1.0" are tracked on the [Github Project Issues Page].
- The [Prototype ESP8266 Firmware] is posted.  The only thing I want to
  fix in the Firmware is the way you hook up to the Wifi Network.
  Right now you have to modify the Firmware Source Code to add your
  SSID and Password.
- I've published a [Prototype Board Design].  I made my prototype using 
  a CNC circuit router. I had an initial version of the board professionally
  fabricated, and it had a flaw :(.  I'll make a new version of the board
  when I make setting the SSID and Password more user friendly.  I'm also
  waiting for some parts from Ebay.
- There is an INDI Driver for Beefocus, the driver is in the
  [INDI Github Repository] in the [3rd Party Beefocus] directory.
- I've created a [Beefocus INDI Usage] page.

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/boards/nema_14_b0/build_example_0.jpg "Nema 14 Build Example")

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/boards/nema_14_b0/schematic.png "Nema 14 Build Schematic")

![alt text](https://raw.githubusercontent.com/glowmouse/beefocus/master/boards/nema_14_b0/board_layout.png "Nema 14 Build Board")

[Github Project Issues Page]:https://github.com/glowmouse/beefocus/issues
[Prototype ESP8266 firmware]: https://github.com/glowmouse/beefocus/tree/master/firmware
[INDI Github Repository]: https://github.com/indilib/indi
[3rd Party Beefocus]: https://github.com/indilib/indi/tree/master/3rdparty/indi-beefocus 
[Prototype board design]: https://github.com/glowmouse/beefocus/tree/master/boards/nema_14_b0
[Beefocus INDI Usage]: https://github.com/glowmouse/beefocus/blob/master/indi_docs/indi_usage_docs.md 


