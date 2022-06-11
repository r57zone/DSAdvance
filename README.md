[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)
&#8211; Other languages / Другие языки

# DSAdvance
Advanced Xbox gamepad emulation for Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-cons. Supports aiming and driving by tilting the gamepad, as well as emulation of sticks on the touchpad. Works based on the driver [ViGEm](https://github.com/ViGEm).

[![](https://user-images.githubusercontent.com/9499881/164945071-5b9f86dd-c396-45a5-817b-fc7068450f02.gif)](https://youtu.be/gkyqO_HuPnk)
[![](https://user-images.githubusercontent.com/9499881/164945073-cfa1bfb7-cb82-4714-b2ad-7ecd84a5bcfc.gif)](https://youtu.be/gkyqO_HuPnk)

# Working modes
Several working modes are supported, they are switched by pressing the touchpad for DualSene & DualShock 4 and for Pro controllers & Joy-Cons to the `+`, and `-` buttons. Brightness is adjustable without pressing.

![](https://user-images.githubusercontent.com/9499881/173076115-3f520a03-41ff-4da9-a7a5-a3de405c779f.png)

In order to exit the stick emulation mode on the touchpad, need to switch to the default mode.


By clicking on the default profile on DualSense the white LEDs display the current battery status (1 - 25%, 2 - 50%, 3 - 75%, 4 - 100%).


To change the brightness, double-tap the brightness area. If the brightness change is blocked, then the backlight will be turned off by double clicking.


The `PS` button simulates pressing `Win+G`, and the microphone button `Win+Alt+PrtScr`.

## Setup
1. Install [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or newer.
3. Connect the Sony DualSense, DualShock 4, Nintendo Pro controller or Joy-Cons (recommended by wire, aiming works better).
4. Unzip and launch DSAdvance.
5. If necessary, change the dead zones of the sticks or other parameters in the configuration file `Config.ini`.
6. When used with Steam games, in the controller settings, disable "Playstation personal settings".

## Possible problems
• **Permanently changing keyboard and gamepad icons**<br>
If this bothers you, then change the aiming mode to "mouse-joystick", in the program, or use aiming by left trigger.



• **Driving don't work in DualSense games**<br>
Launch DSAdvance first, and only then the game itself, the game can give priority to the emulated Xbox controller and driving will work. You can also enable "Only driving & aiming" emulation mode so that the controller will only turn on in driving mode.

## Download
>Version for Windows 10.

**[Download](https://github.com/r57zone/DSAdvance/releases)**

## Thanks
* Sony for the most advanced gamepads and investing in innovation, as well as Nintendo for promoting similar innovations in games.
* [ViGEm](https://github.com/ViGEm) for the ability to emulate different gamepads.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation. Also some code from this library is used.
* For [Reddit users](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) for a detailed description of the USB output packet.

## Feedback
`r57zone[at]gmail.com`