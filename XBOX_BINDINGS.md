# Creating a Profile
Open the folder `DSAdvance\XboxProfiles` and copy the `Default.ini` file, rename it, and insert the necessary buttons for pressing.

## Motion Wheel Buttons (Multi-Button)
The motion wheel buttons allows assigning up to 9 emulations of other buttons to a single button. This is done by holding the specified `WHEEL-ACTIVATION` button, then tilting the gamepad in one of 4 or 8 directions to trigger the button emulation. If no tilt occurs, the default assigned button `WHEEL-DEFAULT` is emulated.

To set up a 4-button motion wheel buttons, do the following. In the `WHEEL-ACTIVATION` parameter, specify one of the "Sony" or "Nintendo" button values (see the "Sony and Nintendo Button Mapping" section), and in the other parameters, specify the "Xbox" values (see the "Xbox Button Names" section).
```ini
[MOTION]
WHEEL-ACTIVATION=TRIANGLE
WHEEL-DEFAULT=Y
WHEEL-UP=UP
WHEEL-LEFT=LEFT
WHEEL-RIGHT=RIGHT
WHEEL-DOWN=DOWN
...
```
To set up an 8-button motion wheel buttons, do the following.
```ini
[MOTION]
WHEEL-ACTIVATION=TRIANGLE
WHEEL-DEFAULT=Y
WHEEL-UP=UP
WHEEL-UP-LEFT=LB
WHEEL-LEFT=LEFT
WHEEL-UP-RIGHT=RB
WHEEL-RIGHT=RIGHT
WHEEL-DOWN=DOWN
WHEEL-DOWN-LEFT=LT
WHEEL-DOWN-RIGHT=RT
...
```

## Xbox Button Names and Stick Movements for the Xbox Section
Buttons | Value
------------ | -------------
None | `NONE`
Left Bumper / L1 | `LB`
Right Bumper / R1 | `RB`
Left Trigger / L2 | `LT`
Right Trigger / R2 | `RT`
Left Stick Press | `LS`
Right Stick Press | `RS`
DPAD Up | `UP`
DPAD Down | `DOWN`
DPAD Left | `LEFT`
DPAD Right | `RIGHT`
Top function button, Sony - ▲ (Triangle), Nintendo - Y | `Y`
Left function button, Sony - ▢ (Square), Nintendo - X | `X`
Right function button, Sony - O (Circle), Nintendo - A | `B`
Bottom function button, Sony - X (Cross), Nintendo - B | `A`
New Xbox - View, Sony - Share, Nintendo - Minus | `BACK`
New Xbox - Menu, Sony - Options, Nintendo - Plus | `START`
Left stick up movement | `LS_UP`
Left stick down movement | `LS_DOWN`
Left stick left movement | `LS_LEFT`
Left stick right movement | `LS_RIGHT`
Right stick up movement | `RS_UP`
Right stick down movement | `RS_DOWN`
Right stick left movement | `RS_LEFT`
Right stick right movement | `RS_RIGHT`

## Sony and Nintendo Button Mapping for Wheel Activation, Aiming, Joy-Con Mode Switching, etc.
Gamepad | Button | Value
------------ | ------------- | -------------
Any | L1 | `L1`
Any | R1 | `R1`
Any | L2 | `L2`
Any | R2 | `R2`
Any | DPAD Up | `UP`
Any | DPAD Down | `DOWN`
Any | DPAD Left | `LEFT`
Any | DPAD Right | `RIGHT`
Any | Left Stick Press | `L3`
Any | Right Stick Press | `R3`
Sony | X (Cross) | `CROSS`
Sony | O (Circle) | `CIRCLE`
Sony | ▢ (Square) | `SQUARE`
Sony | ▲ (Triangle) | `TRIANGLE`
Sony | Share / Back / Left System Button | `SHARE`
Sony | Options / Menu / Right System Button | `OPTIONS`
Sony DualSense Edge | Rear Left Button | `L4`
Sony DualSense Edge | Rear Right Button | `R4`
Nintendo | Left Trigger | `ZL`
Nintendo | Right Trigger | `ZR`
Nintendo | B | `B`
Nintendo | A | `A`
Nintendo | Y | `Y`
Nintendo | X | `X`
Nintendo | Minus | `MINUS`
Nintendo | Plus | `PLUS`
Nintendo Joy-Con | Bottom extra button on the edge | `SL`
Nintendo Joy-Con | Top extra button on the edge | `SR`

### Additional Buttons
Sony DualSense Edge controllers and Joy-Cons have extra buttons `L4`, `R4` and `SL`, `SR`. They can be remapped in the respective `DUALSENSE-EDGE` and `JOYCONS` sections using the values listed above (see "Xbox Button Names" section).
