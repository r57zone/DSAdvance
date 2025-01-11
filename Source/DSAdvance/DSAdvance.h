#pragma once

#define SONY_DUALSHOCK4					27
#define SONY_DUALSENSE					28
#define NINTENDO_JOYCONS				29

#define SONY_VENDOR						0x054C
#define SONY_DS4_USB					0x05C4
#define SONY_DS4_V2_USB					0x09CC
#define SONY_DS4_DONGLE					0x0BA0
#define SONY_DS4_BT						0x081F // ?
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

#define VK_VOLUME_DOWN2					174 // VK_VOLUME_DOWN - already exists
#define VK_VOLUME_UP2					175
#define VK_VOLUME_MUTE2					173

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
#define VK_CUT							516
#define VK_COPY							517
#define VK_PASTE						518


bool ExternalPedalsConnected = false;
HANDLE hSerial;
std::thread *pArduinoReadThread = NULL;
float PedalsValues[2];

std::vector <std::string> KMProfiles;
int ProfileIndex = 0;


struct Gamepad {
	int deviceID[4];
	hid_device *HidHandle;
	hid_device *HidHandle2;
	WORD ControllerType;
	bool USBConnection;
	unsigned char BatteryMode;
	unsigned char BatteryLevel;
	unsigned char BatteryLevel2;
	unsigned char LEDBatteryLevel;
	wchar_t *serial_number;
	float AutoPressStickValue = 0;
	unsigned char DefaultLEDBrightness = 0;
	unsigned char PacketCounter = 0;
	unsigned char RumbleOffCounter = 0;
	bool TestRumbleProController;

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
		float OffsetAxisX = 0.0f;
		float OffsetAxisY = 0.0f;

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

	struct _TouchSticks
	{
		float LeftX = 0;
		float LeftY = 0;
		float RightX = 0;
		float RightY = 0;
	};
	_TouchSticks TouchSticks;
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
	bool ShowBatteryStatusOnLightBar = false;
	int ScreenshotMode = 0;
	int ScreenShotKey = VK_GAMEBAR_SCREENSHOT;
	bool ExternalPedalsArduinoConnected = false;
	int ExternalPedalsCOMPort = 0;
	bool ExternalPedalsDInputSearch = false;
	bool ExternalPedalsDInputConnected = false;
	JOYINFOEX ExternalPedalsJoyInfo;
	JOYCAPS ExternalPedalsJoyCaps;
	int ExternalPedalsJoyIndex = JOYSTICKID1;
	bool LockedChangeBrightness = false;
	bool LockChangeBrightness = true;
	int BrightnessAreaPressed = 0;
	int MicCustomKey = 0;
	struct _Gamepad
	{
		bool BTReset = true;
	};
	_Gamepad Gamepad;

	struct _HotKeys
	{
		std::string ResetKeyName;
		int ResetKey = 0;
	};
	_HotKeys HotKeys;
	bool DeadZoneMode = false;

	bool XboxGamepadReset = false;
	bool LastConnectionType = true; // Problems with BlueTooth, on first connection. Reset fixes this problem.
	int SleepTimeOut = 0;
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

				}  else if (KeyCode == VK_CUT) {
					keybd_event(VK_LCONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event('X', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
				} else if (KeyCode == VK_COPY) {
					keybd_event(VK_LCONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event('C', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
				} else if (KeyCode == VK_PASTE) {
					keybd_event(VK_LCONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
					keybd_event('V', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
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
			
			} else if (KeyCode == VK_CUT) {
				keybd_event('X', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LCONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			} else if (KeyCode == VK_COPY) {
				keybd_event('C', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LCONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			} else if (KeyCode == VK_PASTE) {
				keybd_event('V', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LCONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			}

		ButtonState->UnpressedOnce = false;
		ButtonState->PressedOnce = false;
	}
}

int KeyNameToKeyCode(std::string KeyName) {
	std::transform(KeyName.begin(), KeyName.end(), KeyName.begin(), ::toupper);

	std::unordered_map<std::string, int> KeyMap = {
		{"NONE", 0},
		{"MOUSE-LEFT-CLICK", VK_MOUSE_LEFT_CLICK},
		{"MOUSE-RIGHT-CLICK", VK_MOUSE_RIGHT_CLICK},
		{"MOUSE-MIDDLE-CLICK", VK_MOUSE_MIDDLE_CLICK},
		//{"MOUSE-SIDE1-CLICK", VK_XBUTTON1},
		//{"MOUSE-SIDE2-CLICK", VK_XBUTTON2},
		{"MOUSE-WHEEL-UP", VK_MOUSE_WHEEL_UP},
		{"MOUSE-WHEEL-DOWN", VK_MOUSE_WHEEL_DOWN},

		{"ESCAPE", VK_ESCAPE},
		{"F1", VK_F1},
		{"F2", VK_F2},
		{"F3", VK_F3},
		{"F4", VK_F4},
		{"F5", VK_F5},
		{"F6", VK_F6},
		{"F7", VK_F7},
		{"F8", VK_F8},
		{"F9", VK_F9},
		{"F10", VK_F10},
		{"F11", VK_F11},
		{"F12", VK_F12},

		{"~", 192},
		{"1", '1'},
		{"2", '2'},
		{"3", '3'},
		{"4", '4'},
		{"5", '5'},
		{"6", '6'},
		{"7", '7'},
		{"8", '8'},
		{"9", '9'},
		{"0", '0'},
		{"-", 189},
		{"=", 187},

		{"TAB", VK_TAB},
		{"CAPS-LOCK", VK_CAPITAL},
		{"SHIFT", VK_SHIFT},
		{"LSHIFT", VK_LSHIFT},
		{"RSHIFT", VK_RSHIFT},
		{"CTRL", VK_CONTROL},
		{"LCTRL", VK_LCONTROL},
		{"RCTRL", VK_RCONTROL},

		{"WIN", VK_LWIN},
		{"ALT", VK_MENU},
		{"LALT", VK_LMENU},
		{"RALT", VK_RMENU},
		{"SPACE", VK_SPACE},
		{"ENTER", VK_RETURN},
		{"BACKSPACE", VK_BACK},

		{"Q", 'Q'},
		{"W", 'W'},
		{"E", 'E'},
		{"R", 'R'},
		{"T", 'T'},
		{"Y", 'Y'},
		{"U", 'U'},
		{"I", 'I'},
		{"O", 'O'},
		{"P", 'P'},
		{"[", 219},
		{"]", 221},
		{"A", 'A'},
		{"S", 'S'},
		{"D", 'D'},
		{"F", 'F'},
		{"G", 'G'},
		{"H", 'H'},
		{"J", 'J'},
		{"K", 'K'},
		{"L", 'L'},
		{":", 186},
		{"APOSTROPHE", 222},
		{"\\", 220},
		{"Z", 'Z'},
		{"X", 'X'},
		{"C", 'C'},
		{"V", 'V'},
		{"B", 'B'},
		{"N", 'N'},
		{"M", 'M'},
		{"<", 188},
		{">", 190},
		{"?", 191},

		{"PRINTSCREEN", VK_SNAPSHOT},
		{"SCROLL-LOCK", VK_SCROLL},
		{"PAUSE", VK_PAUSE},
		{"INSERT", VK_INSERT},
		{"HOME", VK_HOME},
		{"PAGE-UP", VK_NEXT},
		{"DELETE", VK_DELETE},
		{"END", VK_END},
		{"PAGE-DOWN", VK_PRIOR},

		{"UP", VK_UP},
		{"DOWN", VK_DOWN},
		{"LEFT", VK_LEFT},
		{"RIGHT", VK_RIGHT},

		{"NUM-LOCK", VK_NUMLOCK},
		{"NUMPAD0", VK_NUMPAD0},
		{"NUMPAD1", VK_NUMPAD1},
		{"NUMPAD2", VK_NUMPAD2},
		{"NUMPAD3", VK_NUMPAD3},
		{"NUMPAD4", VK_NUMPAD4},
		{"NUMPAD5", VK_NUMPAD5},
		{"NUMPAD6", VK_NUMPAD6},
		{"NUMPAD7", VK_NUMPAD7},
		{"NUMPAD8", VK_NUMPAD8},
		{"NUMPAD9", VK_NUMPAD9},

		{"NUMPAD-DIVIDE", VK_DIVIDE},
		{"NUMPAD-MULTIPLY", VK_MULTIPLY},
		{"NUMPAD-MINUS", VK_SUBTRACT},
		{"NUMPAD-PLUS", VK_ADD},
		{"NUMPAD-DEL", VK_DECIMAL},

		// Additional
		{"VOLUME-UP", VK_VOLUME_UP2},
		{"VOLUME-DOWN", VK_VOLUME_DOWN2},
		{"VOLUME-MUTE", VK_VOLUME_MUTE2},
		{"HIDE-APPS", VK_HIDE_APPS},
		{"SWITCH-APP", VK_SWITCH_APP},
		{"DISPLAY-KEYBOARD", VK_DISPLAY_KEYBOARD},
		{"GAMEBAR", VK_GAMEBAR},
		{"GAMEBAR-SCREENSHOT", VK_GAMEBAR_SCREENSHOT},
		{"FULLSCREEN", VK_FULLSCREEN},
		{"FULLSCREEN-PLUS", VK_FULLSCREEN_PLUS},
		{"CHANGE-LANGUAGE", VK_CHANGE_LANGUAGE},
		{"CUT", VK_CUT},
		{"COPY", VK_COPY },
		{"PASTE", VK_PASTE},

		// Special
		{"WASD", WASDStickMode},
		{"ARROWS", ArrowsStickMode},
		{"NUMPAD-ARROWS", NumpadsStickMode},
		{"MOUSE-LOOK", MouseLookStickMode},
		{"MOUSE-WHEEL", MouseWheelStickMode},

		// Media
		{"MEDIA-NEXT-TRACK", VK_MEDIA_NEXT_TRACK},
		{"MEDIA-PREV-TRACK", VK_MEDIA_PREV_TRACK},
		{"MEDIA-STOP", VK_MEDIA_STOP},
		{"MEDIA-PLAY-PAUSE", VK_MEDIA_PLAY_PAUSE},

		// Browser
		{"BROWSER-BACK", VK_BROWSER_BACK},
		{"BROWSER-FORWARD", VK_BROWSER_FORWARD},
		{"BROWSER-REFRESH", VK_BROWSER_REFRESH},
		{"BROWSER-STOP", VK_BROWSER_STOP},
		{"BROWSER-SEARCH", VK_BROWSER_SEARCH},
		{"BROWSER-FAVORITES", VK_BROWSER_FAVORITES},
		{"BROWSER-HOME", VK_BROWSER_HOME}
	};

	if (KeyMap.find(KeyName) != KeyMap.end())
		return KeyMap[KeyName];
	else
		return 0;
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