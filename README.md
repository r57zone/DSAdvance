[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)
← Choose language | Выберите язык

# DSAdvance
Advanced Xbox gamepad emulation for Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-cons. Supports aiming and driving by tilting the gamepad, stick emulation on the touchpad, keyboard and mouse emulation, and [external pedals](https://github.com/r57zone/XboxExternalPedals). Works based on the driver [ViGEm](https://github.com/ViGEm).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Working modes
Several working modes are supported, they are switched by pressing the touchpad for DualSene & DualShock 4 and for Pro controllers & Joy-Cons to the `Capture`, and `Home` buttons.

![](https://user-images.githubusercontent.com/9499881/173076115-3f520a03-41ff-4da9-a7a5-a3de405c779f.png)

To exit the stick emulation mode on the touchpad, need to switch to the default mode.


By clicking on the default profile on DualSense the white LEDs display the current battery status (1 - 0..25%, 2 - 26..50%, 3 - 51..75%, 4 - 76..100%), also on DualSense and DualShock 4 the battery status is shown on the lightbar (green - 100..30%, yellow - 29..10%, red - 9..1%), can be disabled in the config, parameter `ShowBatteryStatusOnLightBar`. For DualSense and DualShock 4 the current charge is displayed in the program itself.


The `PS` button opens the "Xbox Game Bar", `PS + □` - decrease the volume, `PS + ○` - increase the volume, `PS + △` - increases and then decreases aiming sensitivity (reset to `PS + R3`), `PS + X` - microphone button (screenshot / pressing configured keyboard button).


By default, the microphone button takes a screenshot of `Win + Alt + PrtScr` (for DualShock 4 press `PS + X`). By changing the `MicCustomKey` parameter to the [desired button value](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md), it will be pressed.


To emulate the keyboard and mouse for older games, switch the mode to `ALT + Q` or `PS + ←` and `PS + →` and select the desired profile or [create the desired profile](https://github.com/r57zone/DSAdvance/blob/master/BINDINGS.md). Profiles are switched to the `ALT + ↑` and `ALT + ↓` keys if the window is active or on a gamepad using `PS + ↑` and `PS + ↓`. The default profile allows to work in Windows.


To connect external pedals, change the COM port number by changing the `COMPort` parameter in the `ExternalPedals` section.


To turn off the DualSense or DualShock 4, hold down the PS button for 10-15 seconds until the controller turns off.

## Setup
1. Install [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or newer.
3. Connect the Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-Cons.
4. Unzip and launch DSAdvance.
5. If necessary, change the dead zones of the sticks or other parameters in the configuration file `Config.ini`.
6. When used with Steam games, in the controller settings, disable "Playstation personal settings".
7. It is also recommended to install [HidHide](https://github.com/ViGEm/HidHide/releases/), then in the "HidHide Configuration Client" add "DSAdvance.exe" and turn on the parameter `Enable device hiding` (If turned off). It is necessary so that the game did not see our controller, and saw only emulated Xbox 360 gamepad.

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
In some games, for example, Max Payne or Crysis 2, unfortunately, this does not work yet.

## Download
>Version for Windows 10, 11.

**[Download](https://github.com/r57zone/DSAdvance/releases)**

## Credits
* Sony and Nintendo for the most advanced gamepads and investment in innovation, and for driving innovation in games.
* [ViGEm](https://github.com/ViGEm) for the ability to emulate different gamepads.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also some code from this library is used.
* For [Reddit users](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) for a detailed description of the DualSense USB output packet.
* DS4Windows[[1]](https://github.com/Jays2Kings/DS4Windows)[[2]](https://github.com/Ryochan7/DS4Windows) for the battery level.
* ChatGPT for improved aiming.

## Feedback
`r57zone[at]gmail.com`