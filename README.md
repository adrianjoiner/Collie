# Collie
Turn a vintage VIC20 / Commodore 64 keyboard into a functional external USB keyboard for other devices using a Teensy2.0

# Goal
* Must be able to plug the keyboard into a computers USB slot and just start using it as an external keyboard
* What's on the keycaps should faithfully represent what is sent to USB
* All the displayable characters you might normally expect should easily be accesible despite the limited number of keys available (66)
* Control key combinations should work, is Ctrl + C to copy, Shift + cursor key to highlight text
* Windows / Mac Command key should be respected

# Quickstart
Just want to start? Here goes.
* Connect the C64 keyboard wiring harnes to a Teensy 2.0 as per the fritzing layout
* Connect the Teensy to your development computer via USB
* Install the [Arduino IDE](https://www.arduino.cc/en/Main/Software) 
* Install the [Teensyduino plugin] (https://www.pjrc.com/teensy/td_download.html)
* Copy the sketch.ino file from here to a new Arduino sketch
* In Arduino IDE drop down, select board type - Teensy 2.0 - and Mouse + Keyboard
* Build and upload you sketch.
* Open TextEdit, Notepad or something similar, why? Because you don't want to start tying randomly into your sketch :-)
* Use your C64 - it should now behave as a standard keyboard.
