[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)
← Choose language | Выберите язык

# DSAdvance
Advanced Xbox gamepad emulation for Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-cons. Supports aiming and driving by tilting the gamepad, stick emulation on the touchpad, keyboard and mouse emulation, and [external pedals](https://github.com/r57zone/GamepadExternalPedals) with extra buttons, and other Digispark-based joysticks. Works based on the driver [ViGEm](https://github.com/nefarius/ViGEmBus).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Features
✔️ Tilt driving and aiming support (gyroscope, no additional settings required);<br>
✔️ Easy switching between driving and aiming modes (maximum innovative motion gameplay);<br>
✔️ Windows control, volume adjustment, and screenshot creation using a gamepad;<br>
✔️ Remapping Xbox controller buttons, different profiles;<br>
✔️ Keyboard and mouse emulation for old games, as well as profiles;<br>
✔️ Support for adaptive triggers for Sony DualSense in USB connection mode;<br>
✔️ Various emulation modes for games with adaptive triggers;<br>
✔️ Battery level display on the light bar and player indicators (for Sony gamepads);<br>
✔️ Turning off the light bar for full immersion in the dark;<br>
✔️ Support for two gamepads, the first with full functionality, and the second with simplified features (must be enabled in the config);<br>
✔️ Support for external modified racing pedals with any 16 buttons;<br>
✔️ Support for emulating any Xbox controller button presses using a Digispark joystick (up to 16 buttons);<br>

Multiple operation modes are supported. You can switch between them by pressing the touchpad on DualSense and DualShock 4, the `Capture` and `Home` buttons on Pro Controllers and Joy-Cons (pressing `Home` again toggles aim mode — always/L2), or by pressing the touchpad on gamepads compatible with DualShock 4 but lacking a touch sensor (if the `ChangeModesWithoutAreas` parameter is set to `1`). You can also change operation modes using `ALT` + `1/2`. You can also switch operation modes using `ALT` + `1/2`. Pressing `ALT + 1` again disables driving mode, and pressing `ALT + 2` again toggles the aim mode (always/L2).

![](https://github.com/user-attachments/assets/b13153be-0713-4d90-81dd-28798bc17971)

The default gamepad color and the area colors can be changed in the configuration file.

To enable aiming with the `L1` button, change the `AimingWithL2` parameter to `0` in the config.

By default, the computer control area is located at the bottom center. To enable touch sticks, set the `TouchSticksOn` parameter to `1` — the desktop control area will be replaced with the touch stick area. To exit stick emulation mode, press the default mode button.

When pressing the default profile button on the DualSense, white LEDs indicate the current battery charge status (1 - 0..25%, 2 - 26..50%, 3 - 51..75%, 4 - 76..100%). On DualSense and DualShock 4, battery status is also shown on the light bar (green - 100..30%, yellow - 29..10%, red - 9..1%). This can be disabled in the config via the `ShowBatteryStatusOnLightBar` parameter. Battery status is also displayed in the program via `ALT + I`.

Double-tap the brightness area, then swipe left or right to adjust the backlight brightness. Double-tap again to lock it and prevent accidental changes.

There are 3 emulation modes:
* Simple Xbox gamepad emulation with extended functionality;
* Xbox gamepad emulation only for driving mode and mouse aiming (for certain games with adaptive triggers);
* Mouse only;
* Keyboard and mouse emulation for Windows control and some older games;

Mode switching is done via `ALT + Q` or `PS/Home + ←/→` and `Capture/Home`. Keyboard and mouse emulation supports different profiles; select the required profile or [create a new one](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md). There are also Xbox profiles for changing the button layout. Profiles can be switched using `ALT + ↑/↓` when the window is active, or on the gamepad using `PS/Home + ↑/↓`. The default profile allows Windows operation.

Adaptive triggers for Sony DualSense in USB mode are supported, you can switch between different modes with `ALT + 3/4`.

To connect [external pedals (DInput)](https://github.com/r57zone/GamepadExternalPedals#setup-dinput-pedals-mh-et-live-board) and Digispark joystick based devices, change the parameter change the `DInput` parameter to `1` in the `ExternalPedals` section. To connect [external pedals on Arduino](https://github.com/r57zone/GamepadExternalPedals#setup-arduino-pedals), change the COM port number by modifying the `COMPort` parameter.

[![](https://github.com/r57zone/GamepadExternalPedals/assets/9499881/f4b55990-d795-4455-918f-a08a59122171)](https://youtu.be/aK1SV_eXJ_4)
[![](https://user-images.githubusercontent.com/9499881/195859587-65cdaca4-5abd-4594-b079-e388721ae25d.gif)](https://youtu.be/liI_7U_R0as)

There are 2 modes:
1. "Always pedals" - the pedal axes are always bound to the controller triggers.
2. "Dependent (driving/aiming)" - in driving mode, the pedal axes are bound to the triggers, in aiming mode, you can bind button presses to the axes. The degree of force is determined by the `PedalValuePress` parameter.
You can switch modes using the keys `ALT + E`. You can set the default mode by changing the `DefaultMode` parameter.

You can also set pedals or other devices to have up to 16 buttons, which can be assigned to any Xbox gamepad buttons by changing the `Button1..16` parameter.

To turn off DualSense or DualShock 4, hold the PS button, to turn off Nintendo controllers, hold the Capture or Home button for 10-15 seconds until the controllers turn off.

### Hotkeys
Action | Sony Buttons | Nintendo Buttons | Windows  
------------ | ------------- | ------------- | -------------  
Reset/Search Controllers | - | - | `CTRL + R` or `Numpad 0` (default, can be changed)
Swap first and second controllers | - | - | `ALT + V`
Open Xbox Game Bar (`Win + G` press) | `PS` | `Capture + Home` | -  
Decrease and increase Windows volume | `PS + □` and `PS + ○` | `Capture + Y` and `Capture + A` | -  
Screenshot (`Win + ALT + PrtScn` press) | Microphone button or `PS + X` | `Capture + B` | -  
Record video (press `Win + ALT + R`) | Hold the microphone button or `PS + X` | Hold `Capture + B` | -
Aiming mode: mouse emulation or right stick offset | `PS + R1` | `Capture + R1` | `ALT + A`  
Operation mode (driving or aiming) | Touchpad areas or touchpad press (for controllers without touch panel, enable in config) | `Capture/Home` (press `Capture` again to reset, press `Home` again to toggle aim mode — always/L2) | `ALT + 1`, `ALT + 2` (press `ALT + 1` again to reset, press `ALT + 2` again to toggle aim mode — always/L2)
Change aiming sensitivity: increase and then decrease | `PS + △` | `Capture + X` | -  
Reset aiming sensitivity | `PS + RS` | `Capture + RS` | -  
Change rumble strength or disable it | `PS + Options` | `Capture + Plus` | `ALT + </>`  
Enable/disable touchpad press for mode switching (Sony) | `PS + Share` | - | `ALT + W`  
Auto stick press when tilted at a certain angle (value set in the configuration file), either once or continuously | `PS + LS` | `HOME + LS` | `ALT + S`  
Screenshot modes: Xbox Game Bar, Steam, Xbox Game Bar + Steam, custom-configured button (`MicCustomKey` parameter should be set to [the desired key](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md)) | - | - | `ALT + X`  
Turn off light bar (Sony) | `PS + L1` or double-tap the brightness area of the touchpad and swipe left or right. If brightness adjustment is locked (`LockChangeBrightness`), the light bar will turn off with a double tap. | - | `ALT + B`  

## Setup
1. Study the documentation to be aware of all features and specifics.
2. Install [ViGEmBus](https://github.com/nefarius/ViGEmBus/releases).
3. Install Microsoft Visual C++ Redistributable 2017 or newer.
4. Connect the Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-Cons.
5. Unzip and launch DSAdvance.
6. If necessary, change the dead zones of the sticks, triggers or other parameters in the configuration file `Config.ini`.
7. (Optional) To enable operation of two gamepads, change the `Enabled` parameter to `1` in the `SecondaryGamepad` section. You can also adjust dead zones for sticks, triggers, and colors.
8. When used with Steam games, in the controller settings, disable `Playstation personal settings`.
9. It is also recommended to install [HidHide](https://github.com/nefarius/HidHide/releases), then in the `HidHide Configuration Client` add `DSAdvance.exe` and turn on the parameter `Enable device hiding` (If turned off). It is necessary so that the game did not see our controller, and saw only emulated Xbox 360 gamepad.
10. (Optional) To launch from the notification area (tray), by double-clicking, you can add a shortcut to `Launcher.exe` to Windows startup `%AppData%\Microsoft\Windows\Start Menu\Programs\Startup`.
11. (Optional) To run third-party utilities via Launcher, specify the title and path to the application in the configuration file.

## Download
>Version for Windows 10, 11.

**[Download](https://github.com/r57zone/DSAdvance/releases)**

## Possible problems
• **The game sees 2 controllers at the same time (DualSense / DualShock 4 / Nintendo Pro controller or JoyCons and Xbox)**<br>
If the game supports a modern gamepad you can turn off the emulation of the Xbox gamepad on the keys `ALT + Q` or hide this gamepad at all using the program [HidHide](https://github.com/ViGEm/HidHide) or try in wireless mode.



• **Permanently changing keyboard and gamepad icons**<br>
You can change the aiming mode to `Mouse-Joystick`, in the program, or use aiming by left trigger.



• **Adaptive triggers or light bar don't work in the game**<br>
Add the game to the "HidHide" exceptions list and change the DSAdvance mode to `Only mouse` or `Xbox gamepad (only driving) & mouse aiming`.



• **Driving don't work in games with DualSense support (without HidHide)**<br>
Launch DSAdvance first, and only then the game itself, the game can give priority to the emulated Xbox controller and driving will work. You can also enable `Only driving & aiming` emulation mode so that the controller will only turn on in driving mode.

## Credits
* Sony and Nintendo for the most advanced gamepads and investment in innovation, and for driving innovation in games.
* [ViGEm](https://github.com/nefarius/ViGEmBus) for the ability to emulate different gamepads.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp) for Joy-Cons rumble.

## Building
1. Download the sources and unzip them.
2. [Download](https://code.visualstudio.com/download) and [install](https://github.com/r57zone/RE4ExtendedControl/assets/9499881/69dafce6-fd57-4768-83eb-c1bb69901f07) Microsoft Visual Studio Code 2017+.
3. Update the project properties with your tools and SDK.
4. Choose the `Release` build type (if `Debug` is installed) and `x86`, then compile the project.

## Feedback
`r57zone[at]gmail.com`