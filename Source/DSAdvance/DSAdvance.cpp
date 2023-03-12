﻿// DSAdvance by r57zone
// https://github.com/r57zone/DSAdvance

#include <windows.h>
#include <math.h>
#include <mutex>
#include <iostream>
#include "ViGEm\Client.h"
#include "IniReader\IniReader.h"
#include "JoyShockLibrary\JoyShockLibrary.h"
#include "hidapi.h"
#include "DSAdvance.h"
#include <thread>
#include <atlstr.h> 

#pragma comment(lib, "winmm.lib")

void ArduinoRead()
{
	DWORD bytesRead;

	while (AppStatus.ExternalPedalsConnected) {
		ReadFile(hSerial, &PedalsValues, sizeof(PedalsValues), &bytesRead, 0);

		if (PedalsValues[0] > 1.0 || PedalsValues[0] < 0 || PedalsValues[1] > 1.0 || PedalsValues[1] < 0)
		{
			PedalsValues[0] = 0;
			PedalsValues[1] = 0;

			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
		}

		if (bytesRead == 0) Sleep(1);
	}
}

void GamepadSetState(InputOutState OutState)
{
	if (CurGamepad.HidHandle != NULL) {
		if (CurGamepad.ControllerType == SONY_DUALSENSE) { // https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/

			unsigned char PlayersDSPacket = 0;

			if (OutState.PlayersCount == 0) PlayersDSPacket = 0;
			else if (OutState.PlayersCount == 1) PlayersDSPacket = 4;
			else if (OutState.PlayersCount == 2) PlayersDSPacket = 2; // Center 2
			else if (OutState.PlayersCount == 5) PlayersDSPacket = 1; // Both 2
			else if (OutState.PlayersCount == 3) PlayersDSPacket = 5;
			else if (OutState.PlayersCount == 4) PlayersDSPacket = 3;

			if (CurGamepad.USBConnection) {
				unsigned char outputReport[48];
				memset(outputReport, 0, 48);

				outputReport[0] = 0x02;
				outputReport[1] = 0xff;
				outputReport[2] = 0x15;
				outputReport[3] = OutState.LargeMotor;
				outputReport[4] = OutState.SmallMotor;
				outputReport[5] = 0xff;
				outputReport[6] = 0xff;
				outputReport[7] = 0xff;
				outputReport[8] = 0x0c;
				outputReport[38] = 0x07;
				outputReport[44] = PlayersDSPacket;
				outputReport[45] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[46] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[47] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				hid_write(CurGamepad.HidHandle, outputReport, 48);

			} else { // BT
				unsigned char outputReport[79]; // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds5_rumble_light_bt)
				memset(outputReport, 0, 79);

				outputReport[0] = 0xa2;
				outputReport[1] = 0x31;
				outputReport[2] = 0x02;
				outputReport[3] = 0x03;
				outputReport[4] = 0x54;
				outputReport[5] = OutState.LargeMotor;
				outputReport[6] = OutState.SmallMotor;
				outputReport[11] = 0x00;
				outputReport[41] = 0x02;
				outputReport[44] = 0x02;
				outputReport[45] = 0x02;
				outputReport[46] = PlayersDSPacket;
				outputReport[46] &= ~(1 << 7);
				outputReport[46] &= ~(1 << 8);
				outputReport[47] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[48] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[49] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);
				uint32_t crc = crc_32(outputReport, 75);
				memcpy(&outputReport[75], &crc, 4);

				hid_write(CurGamepad.HidHandle, &outputReport[1], 78);
			}

		} else if (CurGamepad.ControllerType == SONY_DUALSHOCK4) { // JoyShockLibrary rumble working for USB DS4 ??? 
			if (CurGamepad.USBConnection) {
				unsigned char outputReport[31];
				memset(outputReport, 0, 31);

				outputReport[0] = 0x05;
				outputReport[1] = 0xff;
				outputReport[4] = OutState.SmallMotor;
				outputReport[5] = OutState.LargeMotor;
				outputReport[6] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[7] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[8] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				hid_write(CurGamepad.HidHandle, outputReport, 31);
			} else { // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds4_rumble_light_bt)
				unsigned char outputReport[79];
				memset(outputReport, 0, 79);

				outputReport[0] = 0xa2;
				outputReport[1] = 0x11;
				outputReport[2] = 0xc0;
				outputReport[3] = 0x20;
				outputReport[4] = 0x07;
				outputReport[5] = 0x00;
				outputReport[6] = 0x00;

				outputReport[7] = OutState.SmallMotor;
				outputReport[8] = OutState.LargeMotor;

				outputReport[9] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[10] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[11] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				outputReport[12] = 0xff;
				outputReport[13] = 0x00;

				uint32_t crc = crc_32(outputReport, 75);
				memcpy(&outputReport[75], &crc, 4);

				hid_write(CurGamepad.HidHandle, &outputReport[1], 78);
			}
		} else {
			//if (JslGetControllerType(0) == JS_TYPE_DS || JslGetControllerType(0) == JS_TYPE_DS4)
				//JslSetLightColour(0, (std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255) << 16) + (std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255) << 8) + std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255)); // https://github.com/CyberPlaton/_Nautilus_/blob/master/Engine/PSGamepad.cpp
			JslSetRumble(0, OutState.SmallMotor, OutState.LargeMotor); // Not working with DualSense USB connection
		}
	} else // Unknown controllers - Pro controller, Joy-cons
		JslSetRumble(0, OutState.SmallMotor, OutState.LargeMotor);
}

void GamepadSearch() {
	struct hid_device_info *cur_dev;

	// Sony controllers
	cur_dev = hid_enumerate(SONY_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == SONY_DS5 || cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB || cur_dev->product_id == SONY_DS4_BT)
		{
			CurGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(CurGamepad.HidHandle, 1);
			
			if (cur_dev->product_id == SONY_DS5) {
				CurGamepad.ControllerType = SONY_DUALSENSE;
				CurGamepad.USBConnection = true;

				// BT detection https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp
				unsigned char buf[64];
				memset(buf, 0, 64);
				hid_read_timeout(CurGamepad.HidHandle, buf, 64, 100);
				if (buf[0] == 0x31)
					CurGamepad.USBConnection = false;
			
			} else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB) {
				CurGamepad.ControllerType = SONY_DUALSHOCK4;
				CurGamepad.USBConnection = true;
			
			} else if (cur_dev->product_id == SONY_DS4_BT) {
				CurGamepad.ControllerType = SONY_DUALSHOCK4;
				CurGamepad.USBConnection = false;
			}
			break;
		}
		cur_dev = cur_dev->next;
	}

	// Sony compatible controllers
	cur_dev = hid_enumerate(BROOK_DS4_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == BROOK_DS4_USB)
		{
			CurGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(CurGamepad.HidHandle, 1);
			CurGamepad.USBConnection = true;
			CurGamepad.ControllerType = SONY_DUALSHOCK4;
			break;
		}
		cur_dev = cur_dev->next;
	}

	if (cur_dev)
		hid_free_enumeration(cur_dev);
}

void GetBatteryInfo() {
	if (CurGamepad.HidHandle != NULL) {
		if (CurGamepad.ControllerType == SONY_DUALSENSE) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(CurGamepad.HidHandle, buf, 64);
			if (CurGamepad.USBConnection) {
				CurGamepad.LEDBatteryLevel = (buf[53] & 0x0f) / 2 + 1; // "+1" for the LED to be responsible for 25%. Each unit of battery data corresponds to 10%, 0 = 0 - 9 % , 1 = 10 - 19 % , .. and 10 = 100 %
				//??? in charge mode, need to show animation within a few seconds 
				CurGamepad.BatteryMode = ((buf[52] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT; // 0x0 - discharging, 0x1 - full, 0x2 - charging, 0xa & 0xb - not-charging, 0xf - unknown
				//printf(" Battery status: %d\n", CurGamepad.BatteryMode); // if there is charging, then we do not add 1 led
				CurGamepad.BatteryLevel = (buf[53] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
			} else { // BT
				CurGamepad.LEDBatteryLevel = (buf[54] & 0x0f) / 2 + 1;
				CurGamepad.BatteryMode = ((buf[53] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT;
				CurGamepad.BatteryLevel = (buf[54] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
			}
			if (CurGamepad.LEDBatteryLevel > 4) // min(data * 10 + 5, 100);
				CurGamepad.LEDBatteryLevel = 4;
		} else if (CurGamepad.ControllerType == SONY_DUALSHOCK4) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(CurGamepad.HidHandle, buf, 64);
			if (CurGamepad.USBConnection)
				CurGamepad.BatteryLevel = (buf[30] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS4_USB_BATTERY_MAX;
			else
				CurGamepad.BatteryLevel = (buf[31] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX; // offset 1?
		}
		if (CurGamepad.BatteryLevel > 100) CurGamepad.BatteryLevel = 100; // It looks like something is not right, once it gave out 125%
	}	
}

EulerAngles QuaternionToEulerAngle(double qW, double qX, double qY, double qZ) // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
{
	EulerAngles resAngles;
	// roll (x-axis rotation)
	double sinr = +2.0 * (qW * qX + qY * qZ);
	double cosr = +1.0 - 2.0 * (qX * qX + qY * qY);
	resAngles.Roll = atan2(sinr, cosr);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (qW * qY - qZ * qX);
	if (fabs(sinp) >= 1)
		resAngles.Pitch = copysign(3.14159265358979323846 / 2, sinp); // use 90 degrees if out of range
	else
		resAngles.Pitch = asin(sinp);

	// yaw (z-axis rotation)
	double siny = +2.0 * (qW * qZ + qX * qY);
	double cosy = +1.0 - 2.0 * (qY * qY + qZ * qZ);
	resAngles.Yaw = atan2(siny, cosy);

	return resAngles;
}

double RadToDeg(double Rad)
{
	return Rad / 3.14159265358979323846 * 180.0;
}

double OffsetYPR(float Angle1, float Angle2)
{
	Angle1 -= Angle2;
	if (Angle1 < -180)
		Angle1 += 360;
	else if (Angle1 > 180)
		Angle1 -= 360;
	return Angle1;
}

static std::mutex m;
VOID CALLBACK notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
)
{
	m.lock();
	GamepadOutState.LargeMotor = LargeMotor;
	GamepadOutState.SmallMotor = SmallMotor;
	GamepadSetState(GamepadOutState);
	m.unlock();
}

float DeadZoneAxis(float StickAxis, float DeadZoneValue) // Possibly wrong
{
	if (StickAxis > 0) {
		StickAxis -= DeadZoneValue;
		if (StickAxis < 0)
			StickAxis = 0;
	} else if (StickAxis < 0) {
		StickAxis += DeadZoneValue;
		if (StickAxis > 0)
			StickAxis = 0;
	}
	return StickAxis * 1 / (1 - DeadZoneValue); // 1 - max value of stick
}

float ClampFloat(float Value, float Min, float Max)
{
	if (Value > Max)
		Value = Max;
	else if (Value < Min)
		Value = Min;
	return Value;
}

float accumulatedX = 0, accumulatedY = 0;
void MouseMove(float x, float y) { // Implementation from https://github.com/JibbSmart/JoyShockMapper/blob/master/JoyShockMapper/src/win32/InputHelpers.cpp
	accumulatedX += x;
	accumulatedY += y;

	int applicableX = (int)accumulatedX;
	int applicableY = (int)accumulatedY;

	accumulatedX -= applicableX;
	accumulatedY -= applicableY;

	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.time = 0;
	input.mi.dx = applicableX;
	input.mi.dy = applicableY;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &input, sizeof(input));
}

SHORT ToLeftStick(double Value, float WheelAngle)
{
	int LeftAxisX = trunc((32767 / WheelAngle) * Value);
	if (LeftAxisX < -32767)
		LeftAxisX = -32767;
	else if (LeftAxisX > 32767)
		LeftAxisX = 32767;
	return LeftAxisX;
}

int KMLeftStickMode = 0;
int KMRightStickMode = 0;
float KMJoySensX = 0;
float KMJoySensY = 0;
void KMStickMode(float StickX, float StickY, int Mode) {
	if (Mode == WASDStickMode) {
		KeyPress('W', StickY > 0.2, &ButtonsStates.Up);
		KeyPress('S', StickY < -0.2, &ButtonsStates.Down);
		KeyPress('A', StickX < -0.2, &ButtonsStates.Left);
		KeyPress('D', StickX > 0.2, &ButtonsStates.Right);
	} else if (Mode == ArrowsStickMode) {
		KeyPress(VK_UP, StickY > 0.2, &ButtonsStates.Up);
		KeyPress(VK_DOWN, StickY < -0.2, &ButtonsStates.Down);
		KeyPress(VK_RIGHT, StickX > 0.2, &ButtonsStates.Right);
		KeyPress(VK_LEFT, StickX < -0.2, &ButtonsStates.Left);
	} else if (Mode == MouseLookStickMode)
		MouseMove(StickX * KMJoySensX, -StickY * KMJoySensY);
	else if (Mode == MouseWheelStickMode)
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, StickY * 50, 0);
	else if (Mode == NumpadsStickMode) {
		KeyPress(VK_NUMPAD8, StickY > 0.2, &ButtonsStates.Up);
		KeyPress(VK_NUMPAD2, StickY < -0.2, &ButtonsStates.Down);
		KeyPress(VK_NUMPAD4, StickX < -0.2, &ButtonsStates.Left);
		KeyPress(VK_NUMPAD6, StickX > 0.2, &ButtonsStates.Right);
	}
}

void LoadKMProfile(std::string ProfileFile) {
	CIniReader IniFile("Profiles\\" + ProfileFile);

	ButtonsStates.LeftTrigger.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "LEFT-TRIGGER", "RIGHT-MOUSE-CLICK"));
	ButtonsStates.RightTrigger.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "RIGHT-TRIGGER", "LEFT-MOUSE-CLICK"));

	ButtonsStates.LeftBumper.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "L1-LB", "F"));
	ButtonsStates.RightBumper.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "R1-RB", "G"));

	ButtonsStates.Back.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "BACK", "ESCAPE"));
	ButtonsStates.Start.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "START", "ENTER"));

	ButtonsStates.DPADUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-UP", "2"));
	ButtonsStates.DPADLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-LEFT", "1"));
	ButtonsStates.DPADRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-RIGHT", "3"));
	ButtonsStates.DPADDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-DOWN", "4"));

	ButtonsStates.Y.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "TRIANGLE-Y", "E"));
	ButtonsStates.X.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "SQUARE-X", "R"));
	ButtonsStates.A.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "CROSS-A", "SPACE"));
	ButtonsStates.B.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "CIRCLE-B", "CTRL"));

	ButtonsStates.LeftStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "LEFT-STICK-CLICK", "NONE"));
	ButtonsStates.RightStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "RIGHT-STICK-CLICK", "NONE"));

	KMLeftStickMode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "LEFT-STICK", "WASD"));
	KMRightStickMode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "RIGHT-STICK", "MOUSE-LOOK"));

	KMJoySensX = IniFile.ReadFloat("MOUSE", "SensitivityX", 100) * 0.22;
	KMJoySensY = IniFile.ReadFloat("MOUSE", "SensitivityY", 100) * 0.22;
}

std::string ResetKeyName;
void MainTextUpdate() {
	system("cls");
	if (AppStatus.ControllerCount < 1)
		printf("\n Connect DualSense, DualShock 4, Pro controller or Joycons and reset.");
	printf("\n Press \"CTRL + R\" or \"%s\" to reset controllers.\n", ResetKeyName.c_str());
	
	if (AppStatus.ControllerCount > 0 && AppStatus.ShowBatteryStatus && (CurGamepad.ControllerType == SONY_DUALSENSE || CurGamepad.ControllerType == SONY_DUALSHOCK4)) {
		printf(" Gamepad mode:");
		if (CurGamepad.USBConnection) printf(" wired"); else printf(" wireless (not recomended for gyro aiming)");
		printf(", battery charge: %d\%%.", CurGamepad.BatteryLevel);
		if (CurGamepad.BatteryMode == 0x2)
			printf(" (charging)", CurGamepad.BatteryLevel);
		printf("\n");
	}

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled)
		printf(" Emulation: Xbox gamepad.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)
		printf(" Emulation: Xbox gamepad (only driving) & mouse aiming.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled)
		printf(" Emulation: Only mouse (for mouse aiming).\n");
	else if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse)
		printf_s(" Emulation: Keyboard and mouse (%s).\n Change profiles with \"ALT + Up | Down\" or \"PS + DPAD Up | Down\".\n", KMProfiles[ProfileIndex].c_str());
	printf(" Press \"ALT + Q\" or \"PS + DPAD Left | Right\" to switch emulation.\n");

	if (AppStatus.ExternalPedalsConnected)
		printf(" External pedals connected.\n");

	if (AppStatus.AimMode == AimMouseMode) printf(" AIM mode = mouse"); else printf(" AIM mode = mouse-joystick");
	printf(", press \"ALT + A\" or \"PS + R1\" to switch.\n");

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled) {
		if (AppStatus.LeftStickMode == LeftStickDefaultMode)
			printf(" Left stick mode: Default");
		else if (AppStatus.LeftStickMode == LeftStickAutoPressMode)
			printf(" Left stick mode: Auto pressing by value");
		else if (AppStatus.LeftStickMode == LeftStickInvertPressMode)
				printf(" Left stick mode: Invert pressed");
		printf(", press \"ALT + S\" or \"PS + LS\" to switch.\n");
	}

	if (AppStatus.ChangeModesWithoutPress) printf(" Change modes without pressing the touchpad"); else printf(" Change modes by pressing the touchpad");
	printf(", press \"ALT + W\" or  \"PS + Share\" to switch.\n");
	printf(" Press \"ALT + B\" or \"PS + L1\" to turn the backlight on or off.\n");

	if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode)
		printf(" Screenshot mode: Custom key");
	else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode)
		printf(" Screenshot mode: Xbox Game Bar");
	else if (AppStatus.ScreenshotMode == ScreenShotSteamMode)
		printf(" Screenshot mode: Steam (F12)");
	else if (AppStatus.ScreenshotMode == ScreenShotMultiMode)
		printf(" Screenshot mode: Xbox Game Bar & Steam (F12)");
	printf(", press \"ALT + X\" to switch.\n");

	printf(" Press \"ALT + F9\" to get the sticks dead zones.\n");
	printf(" Press \"ALT + Escape\" to exit.\n");
}

int main(int argc, char **argv)
{
	SetConsoleTitle("DSAdvance 0.8.6");
	// Config parameters
	CIniReader IniFile("Config.ini");

	bool InvertLeftStickX = IniFile.ReadBoolean("Gamepad", "InvertLeftStickX", false);
	bool InvertLeftStickY = IniFile.ReadBoolean("Gamepad", "InvertLeftStickY", false);
	bool InvertRightStickX = IniFile.ReadBoolean("Gamepad", "InvertRightStickX", false);
	bool InvertRightStickY = IniFile.ReadBoolean("Gamepad", "InvertRightStickY", false);
	ResetKeyName = IniFile.ReadString("Gamepad", "ResetKey", "NONE");
	int ResetKey = KeyNameToKeyCode(ResetKeyName);
	bool ShowBatteryStatusOnLightBar = IniFile.ReadBoolean("Gamepad", "ShowBatteryStatusOnLightBar", true);
	bool AutoReconnect = IniFile.ReadBoolean("Gamepad", "AutoReconnect", false);
	int SleepTimeOut = IniFile.ReadInteger("Gamepad", "SleepTimeOut", 1);

	float DeadZoneLeftStickX = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickX", 0);
	float DeadZoneLeftStickY = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickY", 0);
	float DeadZoneRightStickX = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickX", 0);
	float DeadZoneRightStickY = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickY", 0);

	float TouchLeftStickX = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensX", 4);
	float TouchLeftStickY = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensY", 4);
	float TouchRightStickX = IniFile.ReadFloat("Gamepad", "TouchRightStickSensX", 4);
	float TouchRightStickY = IniFile.ReadFloat("Gamepad", "TouchRightStickSensY", 4);

	float AutoPressModeValue = IniFile.ReadFloat("Gamepad", "AutoPressModeStick", 99) * 0.01f;

	unsigned char DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("Gamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	bool LockedChangeBrightness = IniFile.ReadBoolean("Gamepad", "LockChangeBrightness", false);
	bool LockChangeBrightness = true;
	int BrightnessAreaPressed = 0;
	AppStatus.ChangeModesWithoutPress = IniFile.ReadBoolean("Gamepad", "ChangeModesWithoutPress", false);

	AppStatus.AimMode = IniFile.ReadBoolean("Motion", "AimMode", AimMouseMode);
	float MotionWheelAngle = IniFile.ReadFloat("Motion", "WheelAngle", 150) / 2.0f;
	bool MotionWheelPitch = IniFile.ReadBoolean("Motion", "WheelPitch", false);
	bool MotionWheelRoll = IniFile.ReadBoolean("Motion", "WheelRoll", true);
	int WheelInvertPitch = IniFile.ReadBoolean("Motion", "WheelInvertPitch", false) ? -1 : 1;
	float MotionSensX = IniFile.ReadFloat("Motion", "MouseSensX", 3) / 10.0f;;
	float MotionSensY = IniFile.ReadFloat("Motion", "MouseSensY", 3) / 10.0f;;
	float JoySensX = IniFile.ReadFloat("Motion", "JoySensX", 3);
	float JoySensY = IniFile.ReadFloat("Motion", "JoySensY", 3);
	float CustomMulSens = 1;

	int ScreenShotKey = VK_GAMEBAR_SCREENSHOT;
	int MicCustomKey = KeyNameToKeyCode(IniFile.ReadString("Gamepad", "MicCustomKey", "NONE"));
	if (MicCustomKey == 0) AppStatus.ScreenshotMode = ScreenShotXboxGameBarMode; // If not set, then hide this mode
	else ScreenShotKey = MicCustomKey; 
	

	int ExternalPedalsCOMPort = IniFile.ReadInteger("ExternalPedals", "COMPort", 0);
	if (ExternalPedalsCOMPort != 0) {
		char sPortName[32];
		sprintf_s(sPortName, "\\\\.\\COM%d", ExternalPedalsCOMPort);

		hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (hSerial != INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND) {

			DCB dcbSerialParams = { 0 };
			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

			if (GetCommState(hSerial, &dcbSerialParams))
			{
				dcbSerialParams.BaudRate = CBR_115200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;

				if (SetCommState(hSerial, &dcbSerialParams))
				{
					AppStatus.ExternalPedalsConnected = true;
					PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
					pArduinoReadThread = new std::thread(ArduinoRead);
				}
			}
		}
	}

	TCHAR ChangeEmuModeWav[MAX_PATH] = { 0 };
	GetSystemWindowsDirectory(ChangeEmuModeWav, sizeof(ChangeEmuModeWav));
	_tcscat_s(ChangeEmuModeWav, sizeof(ChangeEmuModeWav), _T("\\Media\\Windows Pop-up Blocked.wav"));

	// Search profiles
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile("Profiles\\*.ini", &ffd);
	KMProfiles.push_back("Desktop.ini");
	KMProfiles.push_back("FPS.ini");
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(ffd.cFileName, "Desktop.ini") && strcmp(ffd.cFileName, "FPS.ini")) // Already added to the top of the list
				KMProfiles.push_back(ffd.cFileName);
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}

	GamepadSearch();
	GamepadOutState.PlayersCount = 0;
	GamepadOutState.LEDBlue = 255;
	GamepadOutState.LEDBrightness = DefaultLEDBrightness;
	GamepadSetState(GamepadOutState);

	int SkipPollCount = 0;
	int PSOnlyCheckCount = 0;
	int PSReleasedCount = 0;
	bool PSOnlyPressed = false;
	int BackOutStateCounter = 0;
	unsigned char LastLEDBrightness = 0; // For battery show
	int ResetCounter = 0; // Auto reconnect controllers & fix JoyShockLibrary bug with increase in CPU usage when the controller is turned off
	bool DeadZoneMode = false;
	int GamepadActionMode = 0; int LastAIMProCtrlMode = 2;
	EulerAngles MotionAngles, AnglesOffset;
	AppStatus.GamepadEmulationMode = EmuGamepadEnabled;

	bool XboxGamepadReset = false;

	bool BTReset = true; bool LastConnectionType = true; // Problems with BlueTooth, on first connection. Reset fixes this problem.
	AppStatus.ControllerCount = JslConnectDevices();
	int deviceID[4];
	JslGetConnectedDeviceHandles(deviceID, AppStatus.ControllerCount);

	MainTextUpdate();

	JOY_SHOCK_STATE InputState;
	MOTION_STATE MotionState;
	TOUCH_STATE TouchState;

	const auto client = vigem_alloc();
	auto ret = vigem_connect(client);

	const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);

	XUSB_REPORT report;

	TouchpadTouch FirstTouch, SecondTouch;

	while (! ( GetAsyncKeyState(VK_LMENU) & 0x8000 && GetAsyncKeyState(VK_ESCAPE) & 0x8000 ) )
	{
		// Reset
		if ((((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 && (GetAsyncKeyState('R') & 0x8000) != 0) || (GetAsyncKeyState(ResetKey) & 0x8000) != 0 && SkipPollCount == 0) || BTReset || ResetCounter == ResetControllersTimeOut)
		{
			AppStatus.ControllerCount = JslConnectDevices();
			JslGetConnectedDeviceHandles(deviceID, AppStatus.ControllerCount);
			GamepadSearch();
			GamepadSetState(GamepadOutState);
			BTReset = false;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		XUSB_REPORT_INIT(&report);
		InputState = JslGetSimpleState(deviceID[0]);
		MotionState = JslGetMotionState(deviceID[0]);
		MotionAngles = QuaternionToEulerAngle(MotionState.quatW, MotionState.quatZ, MotionState.quatX, MotionState.quatY);

		// Stick dead zones
		if (((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) && (GetAsyncKeyState(VK_F9) & 0x8000) != 0 && SkipPollCount == 0)
		{
			DeadZoneMode = !DeadZoneMode;
			if (DeadZoneMode == false) MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}
		if (DeadZoneMode) {
			printf(" Left stick X=%6.2f, ", abs(InputState.stickLX));
			printf("Y=%6.2f\t", abs(InputState.stickLY));
			printf("Right stick X=%6.2f ", abs(InputState.stickRX));
			printf("Y=%6.2f\n", abs(InputState.stickRY));
		}

		// Switch emulation mode
		if ((((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('Q') & 0x8000) != 0) || ((InputState.buttons & JSMASK_LEFT || InputState.buttons & JSMASK_RIGHT) && InputState.buttons & JSMASK_PS)) && SkipPollCount == 0) // Disable Xbox controller emulation for games that support DualSense, DualShock, Nintendo controllers or enable only driving & mouse aiming
		{
			int LastGamepadEmuMode = AppStatus.GamepadEmulationMode;
			if (InputState.buttons & JSMASK_LEFT) {
				if (AppStatus.GamepadEmulationMode == 0) AppStatus.GamepadEmulationMode = EmuGamepadMaxModes; else AppStatus.GamepadEmulationMode--;
			} else
				AppStatus.GamepadEmulationMode++;
			if (AppStatus.GamepadEmulationMode > EmuGamepadMaxModes) AppStatus.GamepadEmulationMode = EmuGamepadEnabled;
			if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled || AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) {
				if (AppStatus.XboxGamepadAttached) {
					vigem_target_x360_unregister_notification(x360);
					vigem_target_remove(client, x360);
					AppStatus.XboxGamepadAttached = false;
				}
			} else if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) {
				if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) AppStatus.AimMode = AimMouseMode;
				if (AppStatus.XboxGamepadAttached == false) {
					ret = vigem_target_add(client, x360);
					ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);
					AppStatus.XboxGamepadAttached = true;
				}
			}
			
			SkipPollCount = 30; // 15 is seems not enough to enable or disable Xbox virtual gamepad

			if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse)
				LoadKMProfile(KMProfiles[ProfileIndex]);

			if ( ( (LastGamepadEmuMode == EmuGamepadEnabled && AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) || (LastGamepadEmuMode == EmuGamepadOnlyDriving && AppStatus.GamepadEmulationMode == EmuGamepadEnabled) ) ||
				 ( (LastGamepadEmuMode == EmuGamepadDisabled && AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) || (LastGamepadEmuMode == EmuKeyboardAndMouse && AppStatus.GamepadEmulationMode == EmuGamepadDisabled) ) )
				PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);

			MainTextUpdate();
		}

		// Switch aiming mode: mouse / joymouse
		if ( ( ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('A') & 0x8000) != 0) || (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_R) ) && SkipPollCount == 0)
		{
			AppStatus.AimMode = !AppStatus.AimMode;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch modes by pressing or touching
		if ( ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('W') & 0x8000) != 0) || (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_SHARE) && SkipPollCount == 0)
		{
			AppStatus.ChangeModesWithoutPress = !AppStatus.ChangeModesWithoutPress;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch left stick mode
		if ((((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('S') & 0x8000) != 0) || (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_LCLICK)) && SkipPollCount == 0)
		{
			AppStatus.LeftStickMode++; if (AppStatus.LeftStickMode > LeftStickMaxModes) AppStatus.LeftStickMode = LeftStickDefaultMode;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch screenshot mode
		if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('X') & 0x8000) != 0 && SkipPollCount == 0)
		{
			AppStatus.ScreenshotMode++; if (AppStatus.ScreenshotMode > ScreenShotMaxModes) AppStatus.ScreenshotMode = MicCustomKey == 0 ? ScreenShotXboxGameBarMode : ScreenShotCustomKeyMode;
			if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode) ScreenShotKey = MicCustomKey;
			else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode) ScreenShotKey = VK_GAMEBAR_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotSteamMode) ScreenShotKey = VK_STEAM_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotMultiMode) ScreenShotKey = VK_MULTI_SCREENSHOT;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Enable or disable lightbar
		if ((((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('B') & 0x8000) != 0) || (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_L)) && SkipPollCount == 0)
		{
			if (GamepadOutState.LEDBrightness == 255) GamepadOutState.LEDBrightness = DefaultLEDBrightness;
			else {
				if (LockedChangeBrightness == false && GamepadOutState.LEDBrightness > 4) // 5 is the minimum brightness
					DefaultLEDBrightness = GamepadOutState.LEDBrightness; // Save the new selected value as default
				GamepadOutState.LEDBrightness = 255;
			} 
			GamepadSetState(GamepadOutState);
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch keyboard and mouse profile
		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse && SkipPollCount == 0)
			if ( (InputState.buttons & JSMASK_PS && (InputState.buttons & JSMASK_UP || InputState.buttons & JSMASK_DOWN)) ||
				( ( (GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && ( (GetAsyncKeyState(VK_UP) & 0x8000) != 0 || (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0) ) && GetConsoleWindow() == GetForegroundWindow()) )
			{
				SkipPollCount = SkipPollTimeOut;
				if ((GetAsyncKeyState(VK_UP) & 0x8000) != 0 || InputState.buttons & JSMASK_UP) if (ProfileIndex > 0) ProfileIndex--; else ProfileIndex = KMProfiles.size() - 1;
				if ((GetAsyncKeyState(VK_DOWN) & 0x8000) != 0 || InputState.buttons & JSMASK_DOWN) if (ProfileIndex < KMProfiles.size() - 1) ProfileIndex++; else ProfileIndex = 0;
				MainTextUpdate();
				LoadKMProfile(KMProfiles[ProfileIndex]);
				PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
			}

		// Switch modes by touchpad
		if (JslGetControllerType(deviceID[0]) == JS_TYPE_DS || JslGetControllerType(deviceID[0]) == JS_TYPE_DS4) {

			TouchState = JslGetTouchState(deviceID[0]);

			if (LockChangeBrightness == false && TouchState.t0Down && TouchState.t0Y <= 0.1 && TouchState.t0X >= 0.325 && TouchState.t0X <= 0.675) { // Brightness change
				GamepadOutState.LEDBrightness = 255 - std::clamp((int)((TouchState.t0X - 0.335) * 255 * 3.2), 0, 255);
				//printf("%5.2f %d\n", (TouchState.t0X - 0.335) * 255 * 3.2, GamepadOutState.LEDBrightness);
				GamepadSetState(GamepadOutState);
			}

			if ((InputState.buttons & JSMASK_TOUCHPAD_CLICK) || (TouchState.t0Down && AppStatus.ChangeModesWithoutPress)) {
				if (TouchState.t0Y <= 0.1 && SkipPollCount == 0) { // Brightness area
					BrightnessAreaPressed++;
					if (BrightnessAreaPressed > 1) {
						if (LockedChangeBrightness) {
							if (GamepadOutState.LEDBrightness == 255) GamepadOutState.LEDBrightness = DefaultLEDBrightness; else GamepadOutState.LEDBrightness = 255;
						} else
							LockChangeBrightness = !LockChangeBrightness;

						BrightnessAreaPressed = 0;
					}
					SkipPollCount = SkipPollTimeOut;
				
				} else if (TouchState.t0Y > 0.1) { // Main area
					if (TouchState.t0X > 0 && TouchState.t0X <= 1 / 3.0 && GamepadActionMode != TouchpadSticksMode) { // [O--] - Driving mode
						GamepadActionMode = MotionDrivingMode;
						AnglesOffset = MotionAngles;
						GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
					
					} else if (TouchState.t0X > 1 / 3.0 && TouchState.t0X <= 1 / 3.0 * 2.0) { // [-O-] // Default & touch sticks modes
						
						if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.7) { // Default mode
							GamepadActionMode = GamepadDefaultMode;
							// Show battery level
							GetBatteryInfo(); if (BackOutStateCounter == 0) BackOutStateCounter = 40; // It is executed many times, so it is done this way, it is necessary to save the old brightness value for return
							if (ShowBatteryStatusOnLightBar) {
								if (BackOutStateCounter == 40) LastLEDBrightness = GamepadOutState.LEDBrightness; // Save on first click (tick)
								if (CurGamepad.BatteryLevel >= 30) { GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255; } // Battery fine 30%-100%
								else if (CurGamepad.BatteryLevel >= 10) { GamepadOutState.LEDBlue = 0; GamepadOutState.LEDGreen = 255; GamepadOutState.LEDRed = 255; } // Battery attention 10..29%
								else { GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0; } // battery alarm 10%
								GamepadOutState.LEDBrightness = DefaultLEDBrightness;
							}
							GamepadOutState.PlayersCount = CurGamepad.LEDBatteryLevel; // JslSetPlayerNumber(deviceID[0], 5);
							AppStatus.ShowBatteryStatus = true;
							MainTextUpdate();
							//printf(" %d %d\n", LastLEDBrightness, GamepadOutState.LEDBrightness);
						} else {  // Touch sticks mode
							GamepadActionMode = TouchpadSticksMode;
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
						}

					} else if (TouchState.t0X > (1 / 3.0) * 2.0 && TouchState.t0X <= 1 && GamepadActionMode != TouchpadSticksMode) { // [--O] Aiming mode
						AnglesOffset = MotionAngles;
						if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.5) { // Motion AIM always
							GamepadActionMode = MotionAimingMode;
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
						} else { // Motion AIM with L2 trigger
							GamepadActionMode = MotionAimingModeOnlyPressed;
							GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
						}
					}
					BrightnessAreaPressed = 0; // Reset lock brightness if clicked in another area
					if (LockChangeBrightness == false) LockChangeBrightness = true;
				}
				GamepadSetState(GamepadOutState);
				//printf("current mode = %d\r\n", GamepadActionMode);
				if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && GamepadActionMode != MotionDrivingMode) XboxGamepadReset = true; // Reset last state
			}

		}

		//printf("%5.2f\t%5.2f\r\n", InputState.stickLX, DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX));
		report.sThumbLX = InvertLeftStickX == false ? DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX) * 32767 : DeadZoneAxis(-InputState.stickLX, DeadZoneLeftStickX) * 32767;
		report.sThumbLY = InvertLeftStickX == false ? DeadZoneAxis(InputState.stickLY, DeadZoneLeftStickY) * 32767 : DeadZoneAxis(-InputState.stickLY, DeadZoneLeftStickY) * 32767;
		report.sThumbRX = InvertRightStickX == false ? DeadZoneAxis(InputState.stickRX, DeadZoneRightStickX) * 32767 : DeadZoneAxis(-InputState.stickRX, DeadZoneRightStickX) * 32767;
		report.sThumbRY = InvertRightStickY == false ? DeadZoneAxis(InputState.stickRY, DeadZoneRightStickY) * 32767 : DeadZoneAxis(-InputState.stickRY, DeadZoneRightStickY) * 32767;

		// Auto stick pressing when value is exceeded
		if (AppStatus.LeftStickMode == LeftStickAutoPressMode && (abs(InputState.stickLX) > AutoPressModeValue || abs(InputState.stickLY) > AutoPressModeValue))
			report.wButtons |= JSMASK_LCLICK;

		report.bLeftTrigger = InputState.lTrigger * 255;
		report.bRightTrigger = InputState.rTrigger * 255;

		if (AppStatus.ExternalPedalsConnected) {
			if (InputState.lTrigger == 0)
				report.bLeftTrigger = PedalsValues[0] * 255;
			if (InputState.rTrigger == 0)
				report.bRightTrigger = PedalsValues[1] * 255;
		}

		if (JslGetControllerType(deviceID[0]) == JS_TYPE_DS || JslGetControllerType(deviceID[0]) == JS_TYPE_DS4) { 
			if (!(InputState.buttons & JSMASK_PS)) {
				report.wButtons |= InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
				report.wButtons |= InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
			}
		} else if (JslGetControllerType(deviceID[0]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_RIGHT) {
			report.wButtons |= InputState.buttons & JSMASK_MINUS ? XINPUT_GAMEPAD_BACK : 0;
			report.wButtons |= InputState.buttons & JSMASK_PLUS ? XINPUT_GAMEPAD_START : 0;
		}

		if (!(InputState.buttons & JSMASK_PS)) { // During special functions, nothing is pressed in the game
			report.wButtons |= InputState.buttons & JSMASK_L ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0;
			report.wButtons |= InputState.buttons & JSMASK_R ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0;
			if (AppStatus.LeftStickMode != LeftStickInvertPressMode) // Invert stick mode
				report.wButtons |= InputState.buttons & JSMASK_LCLICK ? XINPUT_GAMEPAD_LEFT_THUMB : 0;
			else
				report.wButtons |= InputState.buttons & JSMASK_LCLICK ? 0 : XINPUT_GAMEPAD_LEFT_THUMB;
			report.wButtons |= InputState.buttons & JSMASK_RCLICK ? XINPUT_GAMEPAD_RIGHT_THUMB : 0;
			report.wButtons |= InputState.buttons & JSMASK_UP ? XINPUT_GAMEPAD_DPAD_UP : 0;
			report.wButtons |= InputState.buttons & JSMASK_DOWN ? XINPUT_GAMEPAD_DPAD_DOWN : 0;
			report.wButtons |= InputState.buttons & JSMASK_LEFT ? XINPUT_GAMEPAD_DPAD_LEFT : 0;
			report.wButtons |= InputState.buttons & JSMASK_RIGHT ? XINPUT_GAMEPAD_DPAD_RIGHT : 0;
			report.wButtons |= InputState.buttons & JSMASK_N ? XINPUT_GAMEPAD_Y : 0;
			report.wButtons |= InputState.buttons & JSMASK_W ? XINPUT_GAMEPAD_X : 0;
			report.wButtons |= InputState.buttons & JSMASK_S ? XINPUT_GAMEPAD_A : 0;
			report.wButtons |= InputState.buttons & JSMASK_E ? XINPUT_GAMEPAD_B : 0;
		}

		// Nintendo controllers buttons: Capture & Home - changing working mode
		if (JslGetControllerType(deviceID[0]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_RIGHT) {
			if (InputState.buttons & JSMASK_CAPTURE && SkipPollCount == 0) { if (GamepadActionMode == 1) GamepadActionMode = 0; else { GamepadActionMode = 1; AnglesOffset = MotionAngles; } SkipPollCount = SkipPollTimeOut; }
			if (InputState.buttons & JSMASK_HOME && SkipPollCount == 0) { if (GamepadActionMode == 0 || GamepadActionMode == 1) GamepadActionMode = LastAIMProCtrlMode; else if (GamepadActionMode == 2) { GamepadActionMode = 3; LastAIMProCtrlMode = 3; } else { GamepadActionMode = 2; LastAIMProCtrlMode = 2; } SkipPollCount = SkipPollTimeOut; }
		} 

		// GameBar & multi keys
		// PS without any keys
		if (InputState.buttons == JSMASK_PS && PSReleasedCount == 0) { PSOnlyCheckCount = 20; PSOnlyPressed = true; }
		if (PSOnlyCheckCount > 0) {
			if (PSOnlyCheckCount == 1 && PSOnlyPressed)
				PSReleasedCount = PSReleasedTimeOut; // Timeout to release the PS button and don't execute commands
			PSOnlyCheckCount--;
			if (InputState.buttons != JSMASK_PS && InputState.buttons != 0) { PSOnlyPressed = false; PSOnlyCheckCount = 0; }
		}
		if (InputState.buttons & JSMASK_PS && InputState.buttons != JSMASK_PS) PSReleasedCount = PSReleasedTimeOut; // printf("PS + any button\n"); }
		if (PSReleasedCount > 0) PSReleasedCount--;

		KeyPress(VK_GAMEBAR, PSOnlyCheckCount == 1 && PSOnlyPressed, &ButtonsStates.PS);
		KeyPress(VK_VOLUME_DOWN, InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_W, &ButtonsStates.VolumeDown);
		KeyPress(VK_VOLUME_UP, InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_E, &ButtonsStates.VolumeUP);
		KeyPress(ScreenShotKey, InputState.buttons & JSMASK_MIC || (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_S), &ButtonsStates.Mic); // + DualShock 4

		// Custom sens
		if (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_N && SkipPollCount == 0) {
			CustomMulSens += 0.2;
			if (CustomMulSens > 2.4)
				CustomMulSens = 0.2;
 			SkipPollCount = SkipPollTimeOut;
		}
		if (InputState.buttons & JSMASK_PS && InputState.buttons & JSMASK_RCLICK) CustomMulSens = 1; //printf("%5.2f\n", CustomMulSens);

		// Gamepad modes
		if (GamepadActionMode == MotionDrivingMode) { // Motion racing  [O--]
			report.sThumbLX = MotionWheelRoll ? ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Roll), RadToDeg(AnglesOffset.Roll)) * -1, MotionWheelAngle) : report.sThumbLX = ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Yaw), RadToDeg(AnglesOffset.Yaw)) * -1, MotionWheelAngle);
			if (MotionWheelPitch) report.sThumbLY = ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Pitch), RadToDeg(AnglesOffset.Pitch)) * WheelInvertPitch, MotionWheelAngle);
		} else if (GamepadActionMode == MotionAimingMode || GamepadActionMode == MotionAimingModeOnlyPressed) { // Motion aiming  [--X}]
			float DeltaX = OffsetYPR(RadToDeg(MotionAngles.Yaw), RadToDeg(AnglesOffset.Yaw)) * -1;
			float DeltaY = OffsetYPR(RadToDeg(MotionAngles.Pitch), RadToDeg(AnglesOffset.Pitch))  * -1;
			if (GamepadActionMode == MotionAimingMode || (GamepadActionMode == MotionAimingModeOnlyPressed && InputState.lTrigger > 0) )
				if (AppStatus.AimMode == AimMouseMode)
					MouseMove(RadToDeg(DeltaX) * MotionSensX * CustomMulSens, RadToDeg(DeltaY) * MotionSensY  * CustomMulSens);
				else {
					report.sThumbRX = std::clamp((int)(ClampFloat((DeltaX) * JoySensX * CustomMulSens, -1, 1) * 32767 + report.sThumbRX), -32767, 32767);
					report.sThumbRY = std::clamp((int)(ClampFloat(-(DeltaY) * JoySensY * CustomMulSens, -1, 1) * 32767 + report.sThumbRY), -32767, 32767);
				}

			AnglesOffset = MotionAngles; // Not the best way but it works
		} else if (GamepadActionMode == TouchpadSticksMode) { // [-_-] Touchpad sticks

			if (TouchState.t0Down) {
				if (FirstTouch.Touched == false) {
					FirstTouch.InitAxisX = TouchState.t0X;
					FirstTouch.InitAxisY = TouchState.t0Y;
					FirstTouch.Touched = true;
				}
				FirstTouch.AxisX = TouchState.t0X - FirstTouch.InitAxisX;
				FirstTouch.AxisY = TouchState.t0Y - FirstTouch.InitAxisY;

				if (FirstTouch.InitAxisX < 0.5 ) {
					report.sThumbLX = ClampFloat(FirstTouch.AxisX * TouchLeftStickX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-FirstTouch.AxisY * TouchLeftStickY, -1, 1) * 32767;
					if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
				} else {
					report.sThumbRX = ClampFloat((TouchState.t0X - FirstTouch.LastAxisX) * TouchRightStickX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t0Y - FirstTouch.LastAxisY) * TouchRightStickY * 200, -1, 1) * 32767;
					FirstTouch.LastAxisX = TouchState.t0X; FirstTouch.LastAxisY = TouchState.t0Y;
					if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
				}
			} else {
				FirstTouch.AxisX = 0;
				FirstTouch.AxisY = 0;
				FirstTouch.Touched = false;
			}

			if (TouchState.t1Down) {
				if (SecondTouch.Touched == false) {
					SecondTouch.InitAxisX = TouchState.t1X;
					SecondTouch.InitAxisY = TouchState.t1Y;
					SecondTouch.Touched = true;
				}
				SecondTouch.AxisX = TouchState.t1X - SecondTouch.InitAxisX;
				SecondTouch.AxisY = TouchState.t1Y - SecondTouch.InitAxisY;

				if (SecondTouch.InitAxisX < 0.5) {
					report.sThumbLX = ClampFloat(SecondTouch.AxisX * TouchLeftStickX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-SecondTouch.AxisY * TouchLeftStickY, -1, 1) * 32767;
				} else {
					report.sThumbRX = ClampFloat((TouchState.t1X - SecondTouch.LastAxisX) * TouchRightStickX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t1Y - SecondTouch.LastAxisY) * TouchRightStickY * 200, -1, 1) * 32767;
					SecondTouch.LastAxisX = TouchState.t1X; SecondTouch.LastAxisY = TouchState.t1Y;
				}
			} else {
				SecondTouch.AxisX = 0;
				SecondTouch.AxisY = 0;
				SecondTouch.Touched = false;
			}
		
		}

		// Keyboard and mouse mode
		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse && !(InputState.buttons & JSMASK_PS)) {
			KeyPress(ButtonsStates.LeftTrigger.KeyCode, InputState.lTrigger > 0.2, &ButtonsStates.LeftTrigger);
			KeyPress(ButtonsStates.RightTrigger.KeyCode, InputState.rTrigger > 0.2, &ButtonsStates.RightTrigger);

			KeyPress(ButtonsStates.LeftBumper.KeyCode, InputState.buttons & JSMASK_L, &ButtonsStates.LeftBumper);
			KeyPress(ButtonsStates.RightBumper.KeyCode, InputState.buttons & JSMASK_R, &ButtonsStates.RightBumper);

			KeyPress(ButtonsStates.Back.KeyCode, InputState.buttons & JSMASK_SHARE || InputState.buttons & JSMASK_CAPTURE, &ButtonsStates.Back);
			KeyPress(ButtonsStates.Start.KeyCode, InputState.buttons & JSMASK_OPTIONS || InputState.buttons & JSMASK_HOME, &ButtonsStates.Start);

			KeyPress(ButtonsStates.DPADUp.KeyCode, InputState.buttons & JSMASK_UP, &ButtonsStates.DPADUp);
			KeyPress(ButtonsStates.DPADDown.KeyCode, InputState.buttons & JSMASK_DOWN, &ButtonsStates.DPADDown);
			KeyPress(ButtonsStates.DPADLeft.KeyCode, InputState.buttons & JSMASK_LEFT, &ButtonsStates.DPADLeft);
			KeyPress(ButtonsStates.DPADRight.KeyCode, InputState.buttons & JSMASK_RIGHT, &ButtonsStates.DPADRight);

			KeyPress(ButtonsStates.Y.KeyCode, InputState.buttons & JSMASK_N, &ButtonsStates.Y);
			KeyPress(ButtonsStates.A.KeyCode, InputState.buttons & JSMASK_S, &ButtonsStates.A);
			KeyPress(ButtonsStates.X.KeyCode, InputState.buttons & JSMASK_W, &ButtonsStates.X);
			KeyPress(ButtonsStates.B.KeyCode, InputState.buttons & JSMASK_E, &ButtonsStates.B);

			KeyPress(ButtonsStates.LeftStick.KeyCode, InputState.buttons & JSMASK_LCLICK, &ButtonsStates.LeftStick);
			KeyPress(ButtonsStates.RightStick.KeyCode, InputState.buttons & JSMASK_RCLICK, &ButtonsStates.RightStick);

			KMStickMode(DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX), DeadZoneAxis(InputState.stickLY, DeadZoneLeftStickY), KMLeftStickMode);
			KMStickMode(DeadZoneAxis(InputState.stickRX, DeadZoneRightStickX), DeadZoneAxis(InputState.stickRY, DeadZoneRightStickY), KMRightStickMode);
		}

		if (LastConnectionType != CurGamepad.USBConnection) BTReset = true; LastConnectionType = CurGamepad.USBConnection; // Reset if the connection has been changed. Fixes BT bug.

		if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && GamepadActionMode == MotionDrivingMode) || XboxGamepadReset) {
			if (XboxGamepadReset) { XboxGamepadReset = false; XUSB_REPORT_INIT(&report); }
			if (AppStatus.XboxGamepadAttached)
				ret = vigem_target_x360_update(client, x360, report);
		}

		// Battery level display
		if (BackOutStateCounter > 0) { if (BackOutStateCounter == 1) { GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 0; GamepadOutState.PlayersCount = 0; if (ShowBatteryStatusOnLightBar) GamepadOutState.LEDBrightness = LastLEDBrightness; GamepadSetState(GamepadOutState); AppStatus.ShowBatteryStatus = false; MainTextUpdate(); } BackOutStateCounter--; }
		
		if (AutoReconnect) { if (ResetCounter >= ResetControllersTimeOut) ResetCounter = 0; else ResetCounter++; } // Auto reconnect controllers & fix JoyShockLibrary bug with increase in CPU usage when the controller is turned off
		//printf("%d \n", ResetCounter);

		if (SkipPollCount > 0) SkipPollCount--;
		//ButtonsSkipPoll();
		Sleep(SleepTimeOut);
	}

	JslDisconnectAndDisposeAll();
	if (CurGamepad.HidHandle != NULL)
		hid_close(CurGamepad.HidHandle);

	if (AppStatus.ExternalPedalsConnected) {
		AppStatus.ExternalPedalsConnected = false;
		pArduinoReadThread->join();
		delete pArduinoReadThread;
		pArduinoReadThread = nullptr;
		CloseHandle(hSerial);
	}

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)) {
		vigem_target_x360_unregister_notification(x360);
		vigem_target_remove(client, x360);
		vigem_target_free(x360);
	}

	vigem_disconnect(client);
	vigem_free(client);
}
