# Create a profile
Open the `DSAdvance\KMProfiles` folder and copy the `FPS.ini` file, rename it and paste the necessary keys to press.

## Stick modes
The stick modes are specified in the parameters: `LS-MODE` and `RS-MODE`.

Mode | Value
------------ | -------------
Pressing WASD | `WASD`
Pressing arrows | `ARROWS`
Pressing NUMPADs arrows | `NUMPAD-ARROWS`
Pressing NUMPADs arrows | `CUSTOM-BUTTONS`
Mouse movement | `MOUSE-LOOK`
Mouse wheel scrolling | `MOUSE-WHEEL`

For the `CUSTOM-BUTTONS` stick mode, add the following parameters:

```ini
# Left Stick
LS-UP = NONE
LS-LEFT = NONE
LS-RIGHT = NONE
LEFT-STICK-DOWN = NONE

# Right Stick
RS-UP = NONE
RS-LEFT = NONE
RS-RIGHT = NONE
RS-DOWN = NONE
```

To enable support for a second player, duplicate the `[FIRST-GAMEPAD]` section along with all its buttons at the end of the file and rename it to `[SECOND-GAMEPAD]`:
```ini
[SECOND-GAMEPAD]
...
```

## Steering Wheel (gamepad tilt movement)  
When switching to driving mode, tilts of the gamepad along the X-axis are used to emulate presses of the D-Pad buttons: left and right.  

If the tilt is within the dead zone, the wheel is considered centered, and no button presses are emulated.  

Parameter `SteeringWheelDeadZone` defines the wheel's dead zone, specified in percent, from 0 to 100.  

Parameter `SteeringWheelReleaseThreshold` determines how far the wheel must return to the center to stop holding the direction, specified in percent, from 0 to 100.

## Motion Wheel Buttons (Multi-Button)
The motion wheel buttons allows assigning up to 9 emulations of other buttons to a single button. This is done by holding the specified `WHEEL-ACTIVATION` button, then tilting the gamepad in one of 4 or 8 directions to trigger the button emulation. If no tilt occurs, the default assigned button `WHEEL-DEFAULT` is emulated.

To set up a 4-button motion wheel buttons, do the following. In the `WHEEL-ACTIVATION` parameter, specify one of the ["Sony" or "Nintendo" button values](https://github.com/r57zone/DSAdvance/blob/master/XBOX_BINDINGS.md#sony-and-nintendo-button-mapping-for-wheel-activation-aiming-joy-con-mode-switching-etc) (see the "Sony and Nintendo Button Mapping" section), in the other parameters, specify the values from the "Keyboard and Mouse" section.
```ini
[MOTION]
WHEEL-ACTIVATION=TRIANGLE
WHEEL-DEFAULT=1
WHEEL-UP=2
WHEEL-LEFT=1
WHEEL-RIGHT=3
WHEEL-DOWN=4
...
```
To set up an 8-button motion wheel buttons, do the following.
```ini
[MOTION]
WHEEL-ACTIVATION=TRIANGLE
WHEEL-DEFAULT=0
WHEEL-UP=3
WHEEL-UP-LEFT=2
WHEEL-LEFT=1
WHEEL-UP-RIGHT=4
WHEEL-RIGHT=5
WHEEL-DOWN=7
WHEEL-DOWN-LEFT=8
WHEEL-DOWN-RIGHT=6
...
```

## Keyboard and mouse
Key name | Value
------------ | -------------
MOUSE LEFT CLICK | `MOUSE-LEFT`
MOUSE RIGHT CLICK | `MOUSE-RIGHT`
MOUSE MIDDLE CLICK | `MOUSE-MIDDLE`
MOUSE WHEEL UP | `MOUSE-WHEEL-UP`
MOUSE WHEEL DOWN | `MOUSE-WHEEL-DOWN`
ESCAPE | `ESCAPE`
F1 | `F1`
F2 | `F2`
F3 | `F3`
F4 | `F4`
F5 | `F5`
F6 | `F6`
F7 | `F7`
F8 | `F8`
F9 | `F9`
F10 | `F10`
F11 | `F11`
F12 | `F12`
TAB | `TAB`
CAPS-LOCK | `CAPS-LOCK`
SHIFT | `SHIFT`
LEFT SHIFT | `LSHIFT`
RIGHT SHIFT | `RSHIFT`
CTRL | `CTRL`
LEFT CTRL | `LCTRL`
RIGHT CTRL | `RCTRL`
WIN | `WIN`
ALT | `ALT`
LEFT ALT | `LALT`
RIGHT ALT | `RALT`
SPACE | `SPACE`
ENTER | `ENTER`
BACKSPACE | `BACKSPACE`
~ | `~`
1 | `1`
2 | `2`
3 | `3`
4 | `4`
5 | `5`
6 | `6`
7 | `7`
8 | `8`
9 | `9`
0 | `0`
\- | `-`
= + | `=`
a A | `A`
b B | `B`
c C | `C`
d D | `D`
e E | `E`
f F | `F`
g G | `G`
h H | `H`
i I | `I`
j J | `J`
k K | `K`
l L | `L`
m M | `M`
n N | `N`
o O | `O`
p P | `P`
q Q | `Q`
r R | `R`
s S | `S`
t T | `T`
u U | `U`
v V | `V`
w W | `W`
x X | `X`
y Y | `Y`
z Z | `Z`
[ | `[`
] | `]`
; : | `:`
‘ « | `APOSTROPHE`
\ | `\`
< | `<`
\> | `>`
? | `?`
PRINTSCREEN | `PRINTSCREEN`
SCROLL-LOCK | `SCROLL-LOCK`
PAUSE | `PAUSE`
INSERT | `INSERT`
DELETE | `DELETE`
HOME | `HOME`
END | `END`
PAGE-UP | `PAGE-UP`
PAGE-DOWN | `PAGE-DOWN`
UP | `UP`
DOWN | `DOWN`
LEFT | `LEFT`
RIGHT | `RIGHT`
NUM-LOCK | `NUM-LOCK`
NUMPAD 0 | `NUMPAD0`
NUMPAD 1 | `NUMPAD1`
NUMPAD 2 | `NUMPAD2`
NUMPAD 3 | `NUMPAD3`
NUMPAD 4 | `NUMPAD4`
NUMPAD 5 | `NUMPAD5`
NUMPAD 6 | `NUMPAD6`
NUMPAD 7 | `NUMPAD7`
NUMPAD 8 | `NUMPAD8`
NUMPAD 9 | `NUMPAD9`
NUMPAD / | `NUMPAD-DIVIDE`
NUMPAD \* | `NUMPAD-MULTIPLY`
NUMPAD - | `NUMPAD-MINUS`
NUMPAD + | `NUMPAD-PLUS`
NUMPAD ENTER | `NUMPAD-ENTER`
NUMPAD DEL | `NUMPAD-DEL`

## Additional action buttons
Value | Description
------------ | -------------
`VOLUME-UP` | Volume up.
`VOLUME-DOWN` | Volume down.
`VOLUME-MUTE` | Enable / disable sound.
`HIDE-APPS` | Minimize all applications.
`SWITCH-APP` | Show all windows.
`CLOSE-APP` | Close current window.
`DISPLAY-KEYBOARD` | Show / hide the on-screen keyboard.
`GAMEBAR` | Show / hide the Xbox Game Bar.
`GAMEBAR-SCREENSHOT` | Screenshot taken using Xbox Game Bar.
`GAMEBAR-RECORD`     | Video recording using Xbox Game Bar.
`FULLSCREEN` | Switch to full screen mode `ALT + ENTER`.
`FULLSCREEN-PLUS` | Switch to full screen mode `ALT + ENTER` + `F` for YouTube and Twitch services.
`CHANGE-LANGUAGE` | Switch to another language.
`CUT` | Cut.
`COPY` | Copy.
`PASTE` | Paste.
`MEDIA-NEXT-TRACK` | Play the next track.
`MEDIA-PREV-TRACK` | Play the previous track.
`MEDIA-STOP` | Stop media playback.
`MEDIA-PLAY-PAUSE` | Play or pause media.
`BROWSER-BACK` | Go back in the browser.
`BROWSER-FORWARD` | Go forward in the browser.
`BROWSER-REFRESH` | Refresh the current page.
`BROWSER-STOP` | Stop loading the page.
`BROWSER-SEARCH` | Open search in the browser.
`BROWSER-FAVORITES` | Open the favorites list.
`BROWSER-HOME` | Go to the home page.
