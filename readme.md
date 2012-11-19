Joystick Plugin for CryEngine
=============================
Adds Flownode for Joystick and Controller Input.
Port of the FGPS Joystick Node to the Plugin SDK.

Installation / Integration
==========================
Use the installer or extract the files to your Cryengine SDK Folder so that the Code and BinXX/Plugins directory match up.

The plugin manager will automatically load up the plugin when the game/editor is restarted or if you directly load it.

Flownodes
=========
```Plugin_Joystick:Input``` Joystick Input
* In ```Enabled``` Enables/Disables the joystick
* In ```Joystick``` Select the joystick
* Out ```X``` X-axis, usually the left-right movement of a stick.
* Out ```Y``` Y-axis, usually the forward-backward movement of a stick.
* Out ```Z``` Z-axis, often the throttle control. If the joystick does not have this axis, the value is 0.
* Out ```X-Rotate``` X-axis rotation. If the joystick does not have this axis, the value is 0.
* Out ```Y-Rotate``` Y-axis rotation. If the joystick does not have this axis, the value is 0.
* Out ```Z-Rotate``` Z-axis rotation (often called the rudder). If the joystick does not have this axis, the value is 0.
* Out ```Slider``` Two additional axis values (formerly called the u-axis and v-axis) whose semantics depend on the joystick.
* Out ```POV``` Direction controllers, such as point-of-view hats.
  The position is indicated in hundredths of a degree clockwise from north (away from the user).
  The center position is normally reported as -1.
  For indicators that have only five positions, the value for a controller is -1, 0, 9,000, 18,000, or 27,000.
* Out ```ButtonPressed``` Joystick button pressed
* Out ```ButtonReleased``` Joystick button released