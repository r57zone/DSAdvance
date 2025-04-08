[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)
← Choose language | Выберите язык

# DSAdvance
Advanced Xbox gamepad emulation for Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-cons. Supports aiming and driving by tilting the gamepad, stick emulation on the touchpad, keyboard and mouse emulation, and [external pedals](https://github.com/r57zone/GamepadExternalPedals) with extra buttons, and other Digispark-based joysticks.. Works based on the driver [ViGEm](https://github.com/nefarius/ViGEmBus).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Features
✔️ Tilt driving and aiming support (gyroscope, no additional settings required);<br>
✔️ Easy switching between driving and aiming modes (maximum innovative motion gameplay);<br>
✔️ Windows control, volume adjustment, and screenshot creation using a gamepad;<br>
✔️ Battery level display on the light bar and player indicators (for Sony gamepads);<br>
✔️ Various emulation modes for games with adaptive triggers;<br>
✔️ Turning off the light bar for full immersion in the dark;<br>
✔️ Support for external modified racing pedals with any 16 buttons;<br>
✔️ Support for emulating any Xbox controller button presses using a Digispark joystick (up to 16 buttons);<br>

Multiple operating modes are supported, switching is done by tapping the touchpad on DualSense and DualShock 4, or using the `Capture` and `Home` buttons on the Pro Controller and Joy-Cons (pressing `Home` again switches the aiming mode - Always/L2).

![](https://github.com/user-attachments/assets/b13153be-0713-4d90-81dd-28798bc17971)

To enable aiming with the `L1` button, change the `AimingWithL2` parameter to `0` in the config.

To exit stick emulation mode, press the default mode button.

When pressing the default profile button on the DualSense, white LEDs indicate the current battery charge status (1 - 0..25%, 2 - 26..50%, 3 - 51..75%, 4 - 76..100%). On DualSense and DualShock 4, battery status is also shown on the light bar (green - 100..30%, yellow - 29..10%, red - 9..1%). This can be disabled in the config via the `ShowBatteryStatusOnLightBar` parameter. Battery status is also displayed in the program via `ALT + I`.

There are 3 emulation modes:
* Simple Xbox gamepad emulation with extended functionality;
* Xbox gamepad emulation only for driving mode and mouse aiming (for certain games with adaptive triggers);
* Mouse only;
* Keyboard and mouse emulation for Windows control and some older games;

Mode switching is done via `ALT + Q` or `PS/Home + ←/→` and `PS/HOME`. Keyboard and mouse emulation supports different profiles; select the required profile or [create a new one](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md). Profiles can be switched using `ALT + ↑/↓` when the window is active, or on the gamepad using `PS/Home + ↑/↓`. The default profile allows Windows operation.

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
Open Xbox Game Bar (`Win + G` press) | `PS` | `Capture + Home` | -  
Decrease and increase Windows volume | `PS + □` and `PS + ○` | `Capture + Y` and `Capture + A` | -  
Screenshot (`Win + ALT + PrtScn` press) | Microphone button or `PS + X` | `Capture + B` | -  
Aiming mode: mouse emulation or right stick offset | `PS + R1` | `Capture + R1` | `ALT + A`  
Change aiming sensitivity: increase and then decrease | `PS + △` | `Capture + X` | -  
Reset aiming sensitivity | `PS + RS` | `Capture + RS` | -  
Change rumble strength or disable it | `PS + Options` | `Capture + Plus` | `ALT + </>`  
Enable/disable touchpad press for mode switching (Sony) | `PS + Share` | - | `ALT + W`  
Auto-stick press at a certain tilt angle (value set in the config file), as well as inversion | `PS + LS` | `HOME + LS` | `ALT + S`  
Screenshot modes: Xbox Game Bar, Steam, Xbox Game Bar + Steam, custom-configured button (`MicCustomKey` parameter should be set to [the desired key](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md)) | - | - | `ALT + X`  
Turn off light bar (Sony) | `PS + L1` or double-tap the brightness area of the touchpad and swipe left or right. If brightness adjustment is locked (`LockChangeBrightness`), the light bar will turn off with a double tap. | - | `ALT + B`  

## Setup
1. Install [ViGEmBus](https://github.com/nefarius/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or newer.
3. Connect the Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-Cons.
4. Unzip and launch DSAdvance.
5. If necessary, change the dead zones of the sticks, triggers or other parameters in the configuration file `Config.ini`.
6. When used with Steam games, in the controller settings, disable `Playstation personal settings`.
7. It is also recommended to install [HidHide](https://github.com/nefarius/HidHide/releases), then in the `HidHide Configuration Client` add `DSAdvance.exe` and turn on the parameter `Enable device hiding` (If turned off). It is necessary so that the game did not see our controller, and saw only emulated Xbox 360 gamepad.
8. (Optional) To launch from the notification area (tray), by double-clicking, you can add a shortcut to `Launcher.exe` to Windows startup `%AppData%\Microsoft\Windows\Start Menu\Programs\Startup`.
9. (Optional) To run third-party utilities via Launcher, specify the title and path to the application in the configuration file.

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



• **Keyboard emulation don't work in some games**<br>
In some games, for example, Max Payne or Crysis 2, unfortunately, this don'n work yet.



• **Rumble don't work on Nintendo Pro controller**<br>
Not supported yet, solutions are being explored.

## Credits
* Sony and Nintendo for the most advanced gamepads and investment in innovation, and for driving innovation in games.
* [ViGEm](https://github.com/nefarius/ViGEmBus) for the ability to emulate different gamepads.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* [JoyCon-Driver](https://github.com/fossephate/JoyCon-Driver/blob/857e4e76e26f05d72400ae5d9f2a22cae88f3548/joycon-driver/include/Joycon.hpp) for Joy-Cons rumble.

## Building
1. Download the sources and unzip them.
2. [Download](https://code.visualstudio.com/download) and [install](https://github.com/r57zone/RE4ExtendedControl/assets/9499881/69dafce6-fd57-4768-83eb-c1bb69901f07) Microsoft Visual Studio Code 2017+.
3. Update the project properties with your tools and SDK.
4. Choose the `Release` build type (if `Debug` is installed) and `x86`, then compile the project.

## Feedback
`r57zone[at]gmail.com`