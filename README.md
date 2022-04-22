[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/DSAdvance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/DSAdvance/blob/master/README.RU.md)
&#8211; Other languages

# DSAdvance
Advanced Xbox gamepad emulation for Sony DualSense and DualShock 4. Supports aiming and driving by tilting the gamepad, as well as emulation of sticks on the touchpad. Works based on the driver [ViGEm](https://github.com/ViGEm).

# Working modes
Several operating modes are supported, they are switched by pressing the touchpad. Brightness is adjustable without pressing.

![](https://user-images.githubusercontent.com/9499881/164546699-7aa59a26-50ff-4b49-82b9-60c666fd6b9a.png)

In order to exit the stick emulation mode on the touchpad, need to switch to the default mode.


The "PS" button simulates pressing "Win" + "G", and the microphone button "Win" + "Alt" + "PrtScr".

## Setup
1. Install [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or newer.
3. Connect a Sony DualSense or DualShock 4 gamepad via USB (wireless mode is not supported yet).
4. Unzip and launch "DSAdvance".
5. If necessary, change the dead zones of the sticks or other parameters in the configuration file "Config.ini".
6. When used with Steam games, in the controller settings, disable "Playstation personal settings".

## Download
>Version for Windows 10.

**[Download](https://github.com/r57zone/DSAdvance/releases)**

## Thanks
* Sony for the most advanced gamepads and investing in innovation.
* [ViGEm](https://github.com/ViGEm) for the ability to emulate different gamepads.
* [HIDAPI library](https://github.com/signal11/hidapi) with [fixes](https://github.com/libusb/hidapi) for the library to work with a USB devices. The project uses this [fork](https://github.com/r57zone/hidapi).
* [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) for a cool gamepad library that makes it easy to get controller rotation.
* For developer of [DualSense4Windows](https://github.com/broken-bytes/DualSense4Windows) & [Reddit users](https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/) for a detailed description of the USB output packet.
* For developer of [Gen_Dev_TactonBiotic](https://github.com/hizbi-github/Gen_Dev_TactonBiotic) for an example of working with JoyShockLibrary.

## Feedback
`r57zone[at]gmail.com`