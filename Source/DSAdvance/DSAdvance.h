#pragma once

#define SONY_DUALSHOCK4					27
#define SONY_DUALSENSE					28

#define SONY_VENDOR						0x054C
#define SONY_DS4_USB					0x05C4
#define SONY_DS4_V2_USB					0x09CC
#define SONY_DS4_DONGLE					0x0BA0
#define SONY_DS4_BT						0x081F
#define SONY_DS5						0x0CE6
#define SONY_DS5_EDGE					0x0DF2

// DS4 compatible controllers
#define BROOK_DS4_VENDOR				0x0C12
#define BROOK_DS4_USB					0x0E20

#define NINTENDO_VENDOR					0x57E
#define NINTENDO_SWITCH_PRO				0x2009
#define NINTENDO_JOYCON_L				0x2006
#define NINTENDO_JOYCON_R				0x2007

#define DS_STATUS_BATTERY_CAPACITY		0xF
#define DS_STATUS_CHARGING				0xF0
#define DS_STATUS_CHARGING_SHIFT		4
#define DS_BATTERY_MAX					8
#define DS4_USB_BATTERY_MAX				11

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

#define EmuGamepadDisabled				2
#define EmuGamepadEnabled				0
#define EmuGamepadOnlyDriving			1
#define EmuKeyboardAndMouse				3
#define EmuGamepadMaxModes				3

#define AimMouseMode					true
#define AimJoystickMode					false

#define GamepadDefaultMode				0
#define MotionDrivingMode				1
#define MotionAimingMode				2
#define MotionAimingModeOnlyPressed		3
#define TouchpadSticksMode				4

#define LeftStickDefaultMode			0
#define LeftStickAutoPressMode			1
#define LeftStickInvertPressMode		2
#define LeftStickMaxModes				2

#define ScreenShotCustomKeyMode			0
#define ScreenShotXboxGameBarMode		1
#define ScreenShotSteamMode				2
#define ScreenShotMultiMode				3
#define ScreenShotMaxModes				3

#define	SkipPollTimeOut					15
#define	PSReleasedTimeOut				30

#define WASDStickMode					0
#define ArrowsStickMode					1
#define MouseLookStickMode				2
#define MouseWheelStickMode				3
#define NumpadsStickMode				4

#define VK_VOLUME_DOWN					174
#define VK_VOLUME_UP					175
#define VK_VOLUME_MUTE					173

#define VK_MOUSE_LEFT_CLICK				501
#define VK_MOUSE_MIDDLE_CLICK			502
#define VK_MOUSE_RIGHT_CLICK			503
#define VK_MOUSE_WHEEL_UP				504
#define VK_MOUSE_WHEEL_DOWN				505

#define VK_HIDE_APPS					506
#define VK_SWITCH_APP					507
#define VK_SWITCH_APP					507
#define VK_DISPLAY_KEYBOARD				508
#define VK_GAMEBAR						509
#define VK_GAMEBAR_SCREENSHOT			510
#define VK_STEAM_SCREENSHOT				511
#define VK_MULTI_SCREENSHOT				512
#define VK_FULLSCREEN					513
#define VK_FULLSCREEN_PLUS				514
#define VK_CHANGE_LANGUAGE				515


bool ExternalPedalsConnected = false;
HANDLE hSerial;
std::thread *pArduinoReadThread = NULL;
float PedalsValues[2];

std::vector <std::string> KMProfiles;
int ProfileIndex = 0;


struct Gamepad {
	int deviceID[4];
	hid_device *HidHandle;
	WORD ControllerType;
	bool USBConnection;
	unsigned char BatteryMode;
	unsigned char BatteryLevel;
	unsigned char LEDBatteryLevel;
	wchar_t *serial_number;

	struct _Sticks
	{
		float DeadZoneLeftX = 0;
		float DeadZoneLeftY = 0;
		float DeadZoneRightX = 0;
		float DeadZoneRightY = 0;

		bool InvertLeftX = false;
		bool InvertLeftY = false;
		bool InvertRightX = false;
		bool InvertRightY = false;
	};
	_Sticks Sticks;

	struct _Triggers
	{
		float DeadZoneLeft = 0;
		float DeadZoneRight = 0;
	};
	_Triggers Triggers;

	struct _KMEmu
	{
		int LeftStickMode = 0;
		int RightStickMode = 0;
		float JoySensX = 0;
		float JoySensY = 0;
		float StickValuePressKey = 0;
		float TriggerValuePressKey = 0;
	};
	_KMEmu KMEmu;

	struct _Motion
	{
		float DeltaXSmoothed = 0;
		float DeltaYSmoothed = 0;
		float SensX = 0;
		float SensY = 0;
		float JoySensX = 0;
		float JoySensY = 0;
		float WheelAngle = 0;
		bool WheelPitch = false;
		bool WheelRoll = false;
		int WheelInvertPitch = 0;
		float CustomMulSens = 1.0f;
	};
	_Motion Motion;
};
Gamepad CurGamepad;

struct _AppStatus {
	int ControllerCount;
	int GamepadEmulationMode = EmuGamepadEnabled;
	bool XboxGamepadAttached = true;
	bool AimMode = false;
	int LeftStickMode = 0;
	bool ChangeModesWithoutPress = false;
	bool ShowBatteryStatus = false;
	int ScreenshotMode = 0;
	bool ExternalPedalsConnected = false;
	struct _Gamepad
	{
		bool BTReset = true;
	};
	_Gamepad Gamepad;

	struct _HotKeys
	{
		std::string ResetKeyName;
	};
	_HotKeys HotKeys;
	bool DeadZoneMode = false;

	bool XboxGamepadReset = false;
	bool LastConnectionType = true; // Problems with BlueTooth, on first connection. Reset fixes this problem.
}; _AppStatus AppStatus;

//struct _Settings {}; _Settings Settings;

struct InputOutState {
	unsigned char LEDRed;
	unsigned char LEDGreen;
	unsigned char LEDBlue;
	unsigned char LEDBrightness;
	unsigned char LargeMotor;
	unsigned char SmallMotor;
	unsigned char PlayersCount;
};
InputOutState GamepadOutState;

struct EulerAngles {
	double Yaw;
	double Pitch;
	double Roll;
};

struct TouchpadTouch {
	bool Touched;
	float InitAxisX, InitAxisY;
	float AxisX, AxisY;
	float LastAxisX = 0, LastAxisY = 0;
};

struct Button {
	bool PressedOnce = false;
	bool UnpressedOnce;
	int KeyCode = 0;
};

struct _ButtonsState{
	Button LeftBumper;
	Button RightBumper;
	Button LeftTrigger;
	Button RightTrigger;
	Button Back;
	Button Start;
	Button Y;
	Button X;
	Button A;
	Button B;
	Button DPADUp;
	Button DPADDown;
	Button DPADLeft;

	bool DPADAdvancedMode;
	Button DPADRight;
	Button DPADUpLeft;
	Button DPADUpRight;
	Button DPADDownLeft;
	Button DPADDownRight;

	Button LeftStick;
	Button RightStick;
	Button PS;
	Button Mic;

	// Multi keys
	Button VolumeUP;
	Button VolumeDown;

	// Keyboard keys
	Button Up;
	Button Down;
	Button Left;
	Button Right;
};
_ButtonsState ButtonsStates;

//void ButtonsSkipPoll() {
//	if (ButtonsStates.LeftBumper.SkipPollCount > 0) ButtonsStates.LeftBumper.SkipPollCount--;
//	if (ButtonsStates.RightBumper.SkipPollCount > 0) ButtonsStates.RightBumper.SkipPollCount--;
//	if (ButtonsStates.LeftTrigger.SkipPollCount > 0) ButtonsStates.LeftTrigger.SkipPollCount--;
//	if (ButtonsStates.RightTrigger.SkipPollCount > 0) ButtonsStates.RightTrigger.SkipPollCount--;
//	if (ButtonsStates.Back.SkipPollCount > 0) ButtonsStates.Back.SkipPollCount--;
//	if (ButtonsStates.Start.SkipPollCount > 0) ButtonsStates.Start.SkipPollCount--;
//	if (ButtonsStates.DPADUp.SkipPollCount > 0) ButtonsStates.DPADUp.SkipPollCount--;
//	if (ButtonsStates.DPADDown.SkipPollCount > 0) ButtonsStates.DPADDown.SkipPollCount--;
//	if (ButtonsStates.DPADLeft.SkipPollCount > 0) ButtonsStates.DPADLeft.SkipPollCount--;
//	if (ButtonsStates.DPADRight.SkipPollCount > 0) ButtonsStates.DPADRight.SkipPollCount--;
//	if (ButtonsStates.Y.SkipPollCount > 0) ButtonsStates.Y.SkipPollCount--;
//	if (ButtonsStates.X.SkipPollCount > 0) ButtonsStates.X.SkipPollCount--;
//	if (ButtonsStates.A.SkipPollCount > 0) ButtonsStates.A.SkipPollCount--;
//	if (ButtonsStates.B.SkipPollCount > 0) ButtonsStates.B.SkipPollCount--;
//	if (ButtonsStates.LeftStick.SkipPollCount > 0) ButtonsStates.LeftStick.SkipPollCount--;
//	if (ButtonsStates.RightStick.SkipPollCount > 0) ButtonsStates.RightStick.SkipPollCount--;
//	if (ButtonsStates.PS.SkipPollCount > 0) ButtonsStates.PS.SkipPollCount--;
//	if (ButtonsStates.Mic.SkipPollCount > 0) ButtonsStates.Mic.SkipPollCount--;
//}

void MousePress(int MouseBtn, bool ButtonPressed, Button* ButtonState) {
	if (ButtonPressed) {
		ButtonState->UnpressedOnce = true;
		if (ButtonState->PressedOnce == false) {
			if (MouseBtn == VK_MOUSE_LEFT_CLICK)
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			else if (MouseBtn == VK_MOUSE_RIGHT_CLICK)
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			else if (MouseBtn == VK_MOUSE_MIDDLE_CLICK)
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			else if (MouseBtn == VK_MOUSE_WHEEL_UP)
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -120, 0);
			else if (MouseBtn == VK_MOUSE_WHEEL_DOWN)
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 120, 0);
			ButtonState->PressedOnce = true;
			//printf("pressed\n");
		}
	}
	else if (ButtonPressed == false && ButtonState->UnpressedOnce) {
		//printf("unpressed\n");
		if (MouseBtn == VK_MOUSE_LEFT_CLICK)
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		else if (MouseBtn == VK_MOUSE_RIGHT_CLICK)
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
		else if (MouseBtn == VK_MOUSE_MIDDLE_CLICK)
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
		ButtonState->UnpressedOnce = false;
		ButtonState->PressedOnce = false;
	}
}

void KeyPress(int KeyCode, bool ButtonPressed, Button* ButtonState) {
	if (KeyCode == 0) exit;
	else if (KeyCode == VK_MOUSE_LEFT_CLICK || KeyCode == VK_MOUSE_MIDDLE_CLICK || KeyCode == VK_MOUSE_RIGHT_CLICK || 
		KeyCode == VK_MOUSE_WHEEL_UP || KeyCode == VK_MOUSE_WHEEL_DOWN) // Move to mouse press
		MousePress(KeyCode, ButtonPressed, ButtonState);
	else if (ButtonPressed) {
		ButtonState->UnpressedOnce = true;
		if (ButtonState->PressedOnce == false) {

			if (KeyCode < 500)
				keybd_event(KeyCode, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			else
				if (KeyCode == VK_HIDE_APPS) {
					keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event('D', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

				} else if (KeyCode == VK_SWITCH_APP) {
					keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event(VK_TAB, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

				} else if (KeyCode == VK_DISPLAY_KEYBOARD) {
					keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event(VK_CONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event('O', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
				
				} else if (KeyCode == VK_GAMEBAR) {
					keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event('G', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
				
				} else if (KeyCode == VK_GAMEBAR_SCREENSHOT || KeyCode == VK_MULTI_SCREENSHOT) {
					keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

				} else if (KeyCode == VK_STEAM_SCREENSHOT) {
					keybd_event(VK_F12, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

				} else if (KeyCode == VK_FULLSCREEN || KeyCode == VK_FULLSCREEN_PLUS) {
					keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event(VK_RETURN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

				} else if (KeyCode == VK_CHANGE_LANGUAGE) {
					keybd_event(VK_LMENU, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event(VK_SHIFT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

				}
			
			ButtonState->PressedOnce = true;
			//printf("pressed\n");
		}
	} else if (ButtonPressed == false && ButtonState->UnpressedOnce) {
		//printf("unpressed\n");
		if (KeyCode < 500)
			keybd_event(KeyCode, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		else
			if (KeyCode == VK_HIDE_APPS) {
				keybd_event('D', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

			} else if (KeyCode == VK_SWITCH_APP) {
				keybd_event(VK_TAB, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			
			} else if (KeyCode == VK_DISPLAY_KEYBOARD) {
				keybd_event('O', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_CONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			
			} else if (KeyCode == VK_GAMEBAR) {
				keybd_event('G', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			
			} else if (KeyCode == VK_GAMEBAR_SCREENSHOT || (KeyCode == VK_MULTI_SCREENSHOT)) {
				keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				if (KeyCode == VK_MULTI_SCREENSHOT) { keybd_event(VK_F12, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);  keybd_event(VK_F12, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);  } // Steam

			} else if (KeyCode == VK_STEAM_SCREENSHOT) {
				keybd_event(VK_F12, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

			} else if (KeyCode == VK_FULLSCREEN || KeyCode == VK_FULLSCREEN_PLUS) {
				keybd_event(VK_RETURN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				if (KeyCode == VK_FULLSCREEN_PLUS) { keybd_event('F', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);  keybd_event('F', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0); } // YouTube / Twitch fullscreen on F
			
			} else if (KeyCode == VK_CHANGE_LANGUAGE) {
				keybd_event(VK_SHIFT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LMENU, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			}

		ButtonState->UnpressedOnce = false;
		ButtonState->PressedOnce = false;
	}
}

int KeyNameToKeyCode(std::string KeyName) {
	std::transform(KeyName.begin(), KeyName.end(), KeyName.begin(), ::toupper);

	if (KeyName == "NONE") return 0;

	else if (KeyName == "MOUSE-LEFT-CLICK") return VK_MOUSE_LEFT_CLICK;
	else if (KeyName == "MOUSE-RIGHT-CLICK") return VK_MOUSE_RIGHT_CLICK;
	else if (KeyName == "MOUSE-MIDDLE-CLICK") return VK_MOUSE_MIDDLE_CLICK;
	//else if (KeyName == "MOUSE-SIDE1-CLICK") return VK_XBUTTON1;
	//else if (KeyName == "MOUSE-SIDE2-CLICK") return VK_XBUTTON2;
	else if (KeyName == "MOUSE-WHEEL-UP") return VK_MOUSE_WHEEL_UP;
	else if (KeyName == "MOUSE-WHEEL-DOWN") return VK_MOUSE_WHEEL_DOWN;

	else if (KeyName == "ESCAPE") return VK_ESCAPE;
	else if (KeyName == "F1") return VK_F1;
	else if (KeyName == "F2") return VK_F2;
	else if (KeyName == "F3") return VK_F3;
	else if (KeyName == "F4") return VK_F4;
	else if (KeyName == "F5") return VK_F5;
	else if (KeyName == "F6") return VK_F6;
	else if (KeyName == "F7") return VK_F7;
	else if (KeyName == "F8") return VK_F8;
	else if (KeyName == "F9") return VK_F9;
	else if (KeyName == "F10") return VK_F10;
	else if (KeyName == "F11") return VK_F11;
	else if (KeyName == "F12") return VK_F12;

	else if (KeyName == "~") return 192; // VK_OEM_3
	else if (KeyName == "1") return '1';
	else if (KeyName == "2") return '2';
	else if (KeyName == "3") return '3';
	else if (KeyName == "4") return '4';
	else if (KeyName == "5") return '5';
	else if (KeyName == "6") return '6';
	else if (KeyName == "7") return '7';
	else if (KeyName == "8") return '8';
	else if (KeyName == "9") return '9';
	else if (KeyName == "0") return '0';
	else if (KeyName == "-") return 189;
	else if (KeyName == "=") return 187;

	else if (KeyName == "TAB") return VK_TAB;
	else if (KeyName == "CAPS-LOCK") return VK_CAPITAL;
	else if (KeyName == "SHIFT") return VK_SHIFT;
	else if (KeyName == "CTRL") return VK_CONTROL;
	else if (KeyName == "WIN") return VK_LWIN;
	else if (KeyName == "ALT") return VK_MENU;
	else if (KeyName == "SPACE") return VK_SPACE;
	else if (KeyName == "ENTER") return VK_RETURN;
	else if (KeyName == "BACKSPACE") return VK_BACK;

	else if (KeyName == "Q") return 'Q';
	else if (KeyName == "W") return 'W';
	else if (KeyName == "E") return 'E';
	else if (KeyName == "R") return 'R';
	else if (KeyName == "T") return 'T';
	else if (KeyName == "Y") return 'Y';
	else if (KeyName == "U") return 'U';
	else if (KeyName == "I") return 'I';
	else if (KeyName == "O") return 'O';
	else if (KeyName == "P") return 'P';
	else if (KeyName == "[") return 219; // VK_OEM_4
	else if (KeyName == "]") return 221; // VK_OEM_6
	else if (KeyName == "A") return 'A';
	else if (KeyName == "S") return 'S';
	else if (KeyName == "D") return 'D';
	else if (KeyName == "F") return 'F';
	else if (KeyName == "G") return 'G';
	else if (KeyName == "H") return 'H';
	else if (KeyName == "J") return 'J';
	else if (KeyName == "K") return 'K';
	else if (KeyName == "L") return 'L';
	else if (KeyName == ":") return 186;
	else if (KeyName == "APOSTROPHE") return 222; // VK_OEM_7
	else if (KeyName == "\\") return 220; // VK_OEM_6
	else if (KeyName == "Z") return 'Z';
	else if (KeyName == "X") return 'X';
	else if (KeyName == "C") return 'C';
	else if (KeyName == "V") return 'V';
	else if (KeyName == "B") return 'B';
	else if (KeyName == "N") return 'N';
	else if (KeyName == "M") return 'M';
	else if (KeyName == "<") return 188;
	else if (KeyName == ">") return 190;
	else if (KeyName == "?") return 191; // VK_OEM_2

	else if (KeyName == "PRINTSCREEN") return VK_SNAPSHOT;
	else if (KeyName == "SCROLL-LOCK") return VK_SCROLL;
	else if (KeyName == "PAUSE") return VK_PAUSE;
	else if (KeyName == "INSERT") return VK_INSERT;
	else if (KeyName == "HOME") return VK_HOME;
	else if (KeyName == "PAGE-UP") return VK_NEXT;
	else if (KeyName == "DELETE") return VK_DELETE;
	else if (KeyName == "END") return VK_END;
	else if (KeyName == "PAGE-DOWN") return VK_PRIOR;

	else if (KeyName == "UP") return VK_UP;
	else if (KeyName == "DOWN") return VK_DOWN;
	else if (KeyName == "LEFT") return VK_LEFT;
	else if (KeyName == "RIGHT") return VK_RIGHT;

	else if (KeyName == "NUM-LOCK") return VK_NUMLOCK;
	else if (KeyName == "NUMPAD0") return VK_NUMPAD0;
	else if (KeyName == "NUMPAD1") return VK_NUMPAD1;
	else if (KeyName == "NUMPAD2") return VK_NUMPAD2;
	else if (KeyName == "NUMPAD3") return VK_NUMPAD3;
	else if (KeyName == "NUMPAD4") return VK_NUMPAD4;
	else if (KeyName == "NUMPAD5") return VK_NUMPAD5;
	else if (KeyName == "NUMPAD6") return VK_NUMPAD6;
	else if (KeyName == "NUMPAD7") return VK_NUMPAD7;
	else if (KeyName == "NUMPAD8") return VK_NUMPAD8;
	else if (KeyName == "NUMPAD9") return VK_NUMPAD9;

	else if (KeyName == "NUMPAD-DIVIDE") return VK_DIVIDE;
	else if (KeyName == "NUMPAD-MULTIPLY") return VK_MULTIPLY;
	else if (KeyName == "NUMPAD-MINUS") return VK_SUBTRACT;
	else if (KeyName == "NUMPAD-PLUS") return VK_ADD;
	else if (KeyName == "NUMPAD-DEL") return VK_DECIMAL;

	// Additional
	else if (KeyName == "VOLUME-UP") return VK_VOLUME_UP;
	else if (KeyName == "VOLUME-DOWN") return VK_VOLUME_DOWN;
	else if (KeyName == "VOLUME-MUTE") return VK_VOLUME_MUTE;
	else if (KeyName == "HIDE-APPS") return VK_HIDE_APPS;
	else if (KeyName == "SWITCH-APP") return VK_SWITCH_APP;
	else if (KeyName == "DISPLAY-KEYBOARD") return VK_DISPLAY_KEYBOARD;
	else if (KeyName == "GAMEBAR") return VK_GAMEBAR;
	else if (KeyName == "GAMEBAR-SCREENSHOT") return VK_GAMEBAR_SCREENSHOT;
	else if (KeyName == "FULLSCREEN") return VK_FULLSCREEN;
	else if (KeyName == "FULLSCREEN-PLUS") return VK_FULLSCREEN_PLUS;
	else if (KeyName == "CHANGE-LANGUAGE") return VK_CHANGE_LANGUAGE;

	// Special
	else if (KeyName == "WASD") return WASDStickMode;
	else if (KeyName == "ARROWS") return ArrowsStickMode;
	else if (KeyName == "NUMPAD-ARROWS") return NumpadsStickMode;
	else if (KeyName == "MOUSE-LOOK") return MouseLookStickMode;
	else if (KeyName == "MOUSE-WHEEL") return MouseWheelStickMode;

	// Media
	//else if (KeyName == "MEDIA-NEXT-TRACK") return VK_MEDIA_NEXT_TRACK;
	//else if (KeyName == "MEDIA-PREV-TRACK") return VK_MEDIA_PREV_TRACK;
	//else if (KeyName == "MEDIA-STOP") return VK_MEDIA_STOP;
	//else if (KeyName == "MEDIA-PLAY-PAUSE") return VK_MEDIA_PLAY_PAUSE;

	// Browser
	//else if (KeyName == "BROWSER-BACK") return VK_BROWSER_BACK;
	//else if (KeyName == "BROWSER-FORWARD") return VK_BROWSER_FORWARD;
	//else if (KeyName == "BROWSER-REFRESH") return VK_BROWSER_REFRESH;
	//else if (KeyName == "BROWSER-STOP") return VK_BROWSER_STOP;
	//else if (KeyName == "BROWSER-SEARCH") return VK_BROWSER_SEARCH;
	//else if (KeyName == "BROWSER-FAVORITES") return VK_BROWSER_FAVORITES;
	//else if (KeyName == "BROWSER-HOME") return VK_BROWSER_HOME;

	else return 0;
}

// https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp
uint32_t crc_table[256] = {
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
		0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
		0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
		0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
		0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
		0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
		0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
		0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
		0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
		0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
		0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
		0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
		0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
		0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
		0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
		0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
		0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
		0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
		0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
		0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
		0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
		0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
		0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
		0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
		0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
		0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
		0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
		0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
		0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
		0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
		0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
		0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
		0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

// https://docs.microsoft.com/en-us/openspecs/office_protocols/ms-abs/06966aa2-70da-4bf9-8448-3355f277cd77 & https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp
uint32_t crc_32(unsigned char* buf, int length) {
	uint32_t result = 0xFFFFFFFF;
	int index = 0;
	while (index < length) {
		result = crc_table[(result & 0xFF) ^ buf[index]] ^ (result >> 8);
		index++;
	}
	return result ^ 0xFFFFFFFF;
}