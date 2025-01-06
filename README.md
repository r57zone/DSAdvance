[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)
← Choose language | Выберите язык

# DSAdvance
Advanced Xbox gamepad emulation for Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-cons. Supports aiming and driving by tilting the gamepad, stick emulation on the touchpad, keyboard and mouse emulation, and [external pedals](https://github.com/r57zone/XboxExternalPedals). Works based on the driver [ViGEm](https://github.com/nefarius/ViGEmBus).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Features
Several working modes are supported, they are switched by pressing the touchpad for DualSene & DualShock 4 and for Pro controllers & Joy-Cons to the `Capture`, and `Home` buttons.

![](https://user-images.githubusercontent.com/9499881/173076115-3f520a03-41ff-4da9-a7a5-a3de405c779f.png)

To exit the stick emulation mode on the touchpad, need to switch to the default mode.


By clicking on the default profile on DualSense the white LEDs display the current battery status (1 - 0..25%, 2 - 26..50%, 3 - 51..75%, 4 - 76..100%), also on DualSense and DualShock 4 the battery status is shown on the lightbar (green - 100..30%, yellow - 29..10%, red - 9..1%), can be disabled in the config, parameter `ShowBatteryStatusOnLightBar`. For DualSense and DualShock 4 the current charge is displayed in the program itself.


To change the brightness of Sony controllers, double-click on the brightness area. If the brightness change is blocked, the backlight will turn off by double-clicking.


`PS` or `Capture + Home` button opens "Xbox Game Bar" (press `Win + G`), `PS + □` or `CAPTURE + Y` - decrease volume, `PS + ○` or `PS + A` - increase volume, `PS + △` or `CAPTURE + X` - increases and then decreases aiming sensitivity (reset to `PS + R3` or `CAPTURE + R3`), `PS + X` or `CAPTURE + B` - microphone button (screenshot or pressing configured keyboard button).


You can make the stick automatically pressed at a certain stick tilt (value in the configuration file), and also invert the pressing of `ALT + S` or `PS/HOME + LS`.


By default, the microphone button takes a screenshot of `Win + Alt + PrtScr` (for DualShock 4 `PS + X`, and for Nintendo controllers `CAPTURE + B`). By changing the `MicCustomKey` parameter to the [desired button value](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md), it will be pressed.


To emulate keyboard and mouse, for old games, switch the operating mode to `ALT + Q` or `PS/HOME + ←` and `PS/HOME + →`, and select the desired profile or [create the desired profile](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md). Profiles are switched to the `ALT + ↑` and `ALT + ↓` keys if the window is active or on a gamepad using `PS + ↑` and `PS + ↓` or `HOME + ↑` and `HOME + ↓`. The default profile allows to work in Windows.


To connect [external pedals (DInput)](https://github.com/r57zone/XboxExternalPedals#setup-dinput-pedals-mh-et-live-board), change the `DInput` parameter to `1`, in the `ExternalPedals` section.  To connect [external pedals on Arduino](https://github.com/r57zone/XboxExternalPedals#setup-arduino-pedals), change the COM port number by changing the `COMPort` parameter.


To turn off the DualSense or DualShock 4, hold down the PS button for 10-15 seconds until the controller turns off.

## Setup
1. Install [ViGEmBus](https://github.com/nefarius/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or newer.
3. Connect the Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-Cons.
4. Unzip and launch DSAdvance.
5. If necessary, change the dead zones of the sticks, triggers or other parameters in the configuration file `Config.ini`.
6. When used with Steam games, in the controller settings, disable "Playstation personal settings".
7. It is also recommended to install [HidHide](https://github.com/nefarius/HidHide/releases), then in the "HidHide Configuration Client" add "DSAdvance.exe" and turn on the parameter `Enable device hiding` (If turned off). It is necessary so that the game did not see our controller, and saw only emulated Xbox 360 gamepad.
8. (Optional) To launch from the notification area (tray), by double-clicking, you can add a shortcut to `Launcher.exe` to Windows startup `%AppData%\Microsoft\Windows\Start Menu\Programs\Startup`.
9. (Optional) To run third-party utilities via Launcher, specify the title and path to the application in the configuration file.

## Download
>Version for Windows 10, 11.

**[Download](https://github.com/r57zone/DSAdvance/releases)**

## Possible problems
• **The game sees 2 controllers at the same time (DualSense / DualShock 4 / Nintendo Pro controller or JoyCons and Xbox)**<br>
If the game supports a modern gamepad you can turn off the emulation of the Xbox gamepad on the keys `ALT + Q` or hide this gamepad at all using the program [HidHide](https://github.com/ViGEm/HidHide) or try in wireless mode.



• **Permanently changing keyboard and gamepad icons**<br>
You can change the aiming mode to "mouse-joystick", in the program, or use aiming by left trigger.



• **Adaptive triggers or light bar don't work in the game**<br>
Add the game to the "HidHide" exceptions list and change the "DSAdvance" mode to "Only mouse".



• **Driving don't work in games with DualSense support (without HidHide)**<br>
Launch DSAdvance first, and only then the game itself, the game can give priority to the emulated Xbox controller and driving will work. You can also enable "Only driving & aiming" emulation mode so that the controller will only turn on in driving mode.



• **Keyboard emulation don't work in some games**<br>
In some games, for example, Max Payne or Crysis 2, unfortunately, this don'n work yet.



• **Vibration does not work on Nintendo controllers**<br>
Unfortunately, this is not yet implemented.

## Credits
* Sony and Nintendo for the most advanced gamepads and investment in innovation, and for driving innovation in games.
* [ViGEm](https://github.com/ViGEm) for the ability to emulate different gamepads.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also uses some code from this library and [JibbSmart snippet](https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df) for aiming.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.

## Building
1. Download the sources and unzip them.
2. [Download](https://code.visualstudio.com/download) and [install](https://github.com/r57zone/RE4ExtendedControl/assets/9499881/69dafce6-fd57-4768-83eb-c1bb69901f07) Microsoft Visual Studio Code 2017+.
3. Update the project properties with your tools and SDK.
4. Choose the `Release` build type (if `Debug` is installed) and `x86`, then compile the project.

## Feedback
`r57zone[at]gmail.com`