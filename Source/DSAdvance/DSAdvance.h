#pragma once

#define SONY_DUALSHOCK4					27
#define SONY_DUALSENSE					28
#define DS_MAX_COUNT					4

#define SONY_VENDOR 0x054C
#define SONY_DS4_USB 0x05C4
#define SONY_DS4_V2_USB 0x09CC
#define SONY_DS4_BT 0x081F
#define SONY_DS5_USB 0x0CE6

#define XINPUT_GAMEPAD_DPAD_UP          0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
#define XINPUT_GAMEPAD_GUIDE            0x0400
#define XINPUT_GAMEPAD_START            0x0010
#define XINPUT_GAMEPAD_BACK             0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
#define XINPUT_GAMEPAD_A                0x1000
#define XINPUT_GAMEPAD_B                0x2000
#define XINPUT_GAMEPAD_X                0x4000
#define XINPUT_GAMEPAD_Y				0x8000

struct InputOutState {
	unsigned char LEDRed;
	unsigned char LEDGreen;
	unsigned char LEDBlue;
	unsigned char LEDBrightness;
	unsigned char LargeMotor;
	unsigned char SmallMotor;
};