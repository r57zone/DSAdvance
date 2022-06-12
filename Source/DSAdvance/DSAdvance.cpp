#include <windows.h>
#include <math.h>
#include <mutex>
#include <iostream>
#include "ViGEm\Client.h"
#include "IniReader\IniReader.h"
#include "JoyShockLibrary\JoyShockLibrary.h"
#include "hidapi.h"
#include "DSAdvance.h"

Gamepad CurGamepad;
InputOutState GamepadOutState;

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
				unsigned char outputReport[79];
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

		}
		else if (CurGamepad.ControllerType == SONY_DUALSHOCK4 && CurGamepad.USBConnection) { // JoyShockLibrary rumble working for USB DS4 ??? 
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

		} else {
			if (JslGetControllerType(0) == JS_TYPE_DS || JslGetControllerType(0) == JS_TYPE_DS4)
				JslSetLightColour(0, (std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255) << 16) + (std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255) << 8) + std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255)); // https://github.com/CyberPlaton/_Nautilus_/blob/master/Engine/PSGamepad.cpp
			JslSetRumble(0, OutState.SmallMotor, OutState.LargeMotor); // Not working with DualSense USB connection
		}
	} else // Unknown controllers - Pro controller, Joy-cons
		JslSetRumble(0, OutState.SmallMotor, OutState.LargeMotor);
}

void GamepadSearch() {
	struct hid_device_info *cur_dev;
	cur_dev = hid_enumerate(SONY_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == SONY_DS5 || cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB)
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
			}
			else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB)
			{
				CurGamepad.ControllerType = SONY_DUALSHOCK4;
				CurGamepad.USBConnection = true;
			}
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
			if (CurGamepad.USBConnection) {
				unsigned char buf[64];
				memset(buf, 0, 64);
				hid_read(CurGamepad.HidHandle, buf, 64);
				CurGamepad.BatteryLevel = (buf[53] & 0x0f) / 2; // ?
				//CurGamepad.BatteryMode = buf[30]; //??? in charge mode, need to show animation within a few seconds
			} else { // BT
				unsigned char buf[64];
				memset(buf, 0, 64);
				hid_read(CurGamepad.HidHandle, buf, 64);
				CurGamepad.BatteryLevel = (buf[54] & 0x0f) / 2; // ?
				//CurGamepad.BatteryMode = buf[31];  //???
			}
			if (CurGamepad.BatteryLevel > 4)
				CurGamepad.BatteryLevel = 4;
			//printf("%d ", CurGamepad.BatteryLevel);
			//printf("%d\n", CurGamepad.BatteryMode);
		}
	}	
}

static std::mutex m;

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

double OffsetYPR(float f, float f2)
{
	f -= f2;
	if (f < -180)
		f += 360;
	else if (f > 180)
		f -= 360;
	return f;
}

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

float DeadZoneAxis(float StickAxis, float DeadZoneValue)
{
	if (StickAxis > 0)
	{
		StickAxis -= DeadZoneValue;
		if (StickAxis < 0)
			StickAxis = 0;
	}
	else if (StickAxis < 0) {
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

void DefMainText(int ControllerCount, int EmuMode, int AimMode, bool ChangeModesWithoutPress) {
	system("cls");
	if (ControllerCount < 1)
		printf("\n Connect DualSense, DualShock 4, Pro controller or Joycons and reset.");
	printf("\n Press \"CTRL\" + \"R\" to reset controllers.\n");

	if (EmuMode == XboxGamepadEnabled)
		printf(" Emulation: Xbox gamepad");
	else if (EmuMode == XboxGamepadOnlyDriving)
		printf(" Emulation: Xbox gamepad (only driving & mouse aiming)");
	else
		printf(" Emulation: -");
	printf(", press \"ALT\" + \"Q\" to switch.\n");

	if (AimMode == 1) printf(" AIM mode = mouse"); else printf(" AIM mode = mouse-joystick");
	printf(", press \"ALT\" + \"A\" to switch.\n");

	if (ChangeModesWithoutPress) printf(" Change modes without pressing the touchpad"); else printf(" Change modes by pressing the touchpad");
	printf(", press \"ALT\" + \"W\" to switch.\n");

	printf(" Press \"ALT\" + \"Escape\" to exit.\n");
}

int main(int argc, char **argv)
{
	SetConsoleTitle("DSAdvance 0.6.1");
	// Config parameters
	CIniReader IniFile("Config.ini");

	bool InvertLeftStickX = IniFile.ReadBoolean("Gamepad", "InvertLeftStickX", false);
	bool InvertLeftStickY = IniFile.ReadBoolean("Gamepad", "InvertLeftStickY", false);
	bool InvertRightStickX = IniFile.ReadBoolean("Gamepad", "InvertRightStickX", false);
	bool InvertRightStickY = IniFile.ReadBoolean("Gamepad", "InvertRightStickY", false);
	int SleepTimeOut = IniFile.ReadInteger("Gamepad", "SleepTimeOut", 1);
	bool ResetOnDefaultMode = IniFile.ReadBoolean("Gamepad", "ResetOnDefaultMode", false);

	float DeadZoneLeftStickX = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickX", 0);
	float DeadZoneLeftStickY = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickY", 0);
	float DeadZoneRightStickX = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickX", 0);
	float DeadZoneRightStickY = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickY", 0);

	float TouchLeftStickX = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensX", 4);
	float TouchLeftStickY = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensY", 4);
	float TouchRightStickX = IniFile.ReadFloat("Gamepad", "TouchRightStickSensX", 4);
	float TouchRightStickY = IniFile.ReadFloat("Gamepad", "TouchRightStickSensY", 4);

	unsigned char DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("Gamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	bool LockedChangeBrightness = IniFile.ReadBoolean("Gamepad", "LockChangeBrightness", false);
	bool LockChangeBrightness = true;
	int BrightnessAreaPressed = 0;
	bool ChangeModesWithoutPress = IniFile.ReadBoolean("Gamepad", "ChangeModesWithoutPress", false);

	bool AimMode = IniFile.ReadBoolean("Motion", "AimMode", false);
	float MotionWheelAngle = IniFile.ReadFloat("Motion", "WheelAngle", 75);
	float MotionSensX = IniFile.ReadFloat("Motion", "MouseSensX", 3);
	float MotionSensY = IniFile.ReadFloat("Motion", "MouseSensY", 3);
	float JoySensX = IniFile.ReadFloat("Motion", "JoySensX", 3) * 10.0f;
	float JoySensY = IniFile.ReadFloat("Motion", "JoySensY", 3) * 10.0f;

	GamepadSearch();
	GamepadOutState.PlayersCount = 0;
	GamepadOutState.LEDBlue = 255;
	GamepadOutState.LEDBrightness = DefaultLEDBrightness;
	GamepadSetState(GamepadOutState);

	int SkipPollCount = 0;
	int BackOutStateCounter = 0;
	bool DeadZoneMode = false;
	int GamepadMode = 0; int LastAIMProCtrlMode = 2;
	EulerAngles MotionAngles, AnglesOffset;
	int XboxGamepadEmuMode = XboxGamepadEnabled;
	bool XboxGamepadReset = false;

	bool BTReset = true; // Problems with BlueTooth, on first connection. Reconnecting fixes the problem.
	int controllersCount = JslConnectDevices();
	int deviceID[4];
	JslGetConnectedDeviceHandles(deviceID, controllersCount);

	DefMainText(controllersCount, XboxGamepadEmuMode, AimMode, ChangeModesWithoutPress);

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
		// Dead zones
		if (((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) && (GetAsyncKeyState(VK_F9) & 0x8000) != 0  && SkipPollCount == 0)
		{
			DeadZoneMode = !DeadZoneMode;
			if (DeadZoneMode == false) DefMainText(controllersCount, XboxGamepadEmuMode, AimMode, ChangeModesWithoutPress);
			SkipPollCount = SkipPollTimeOut;
		}

		if (DeadZoneMode) {
			printf(" Left Stick X=%6.2f, ", abs(InputState.stickLX));
			printf("Y=%6.2f\t", abs(InputState.stickLY));
			printf("Right Stick X=%6.2f ", abs(InputState.stickRX));
			printf("Y=%6.2f\n", abs(InputState.stickRY));
		}

		if ( ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 && (GetAsyncKeyState('R') & 0x8000) != 0 && SkipPollCount == 0 ) || BTReset)
		{
			controllersCount = JslConnectDevices();
			JslGetConnectedDeviceHandles(deviceID, controllersCount);
			if (CurGamepad.HidHandle != NULL)
				hid_close(CurGamepad.HidHandle);
			GamepadSearch();
			GamepadSetState(GamepadOutState);
			SkipPollCount = SkipPollTimeOut;
			BTReset = false;
			DefMainText(controllersCount, XboxGamepadEmuMode, AimMode, ChangeModesWithoutPress);
		}

		if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('Q') & 0x8000) != 0 && SkipPollCount == 0) // Disable Xbox controller emulation for games that support DualSense, DualShock, Nintendo controllers or enable only driving & mouse aiming
		{
			XboxGamepadEmuMode++;
			if (XboxGamepadEmuMode > 2) XboxGamepadEmuMode = 0;
			if (XboxGamepadEmuMode == XboxGamepadDisabled) {
				vigem_target_x360_unregister_notification(x360);
				vigem_target_remove(client, x360);
			} else if (XboxGamepadEmuMode == XboxGamepadEnabled || XboxGamepadEmuMode == XboxGamepadOnlyDriving) {
				ret = vigem_target_add(client, x360);
				ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);
			}

			DefMainText(controllersCount, XboxGamepadEmuMode, AimMode, ChangeModesWithoutPress);
			SkipPollCount = SkipPollTimeOut;
		}

		if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('A') & 0x8000) != 0 && SkipPollCount == 0)
		{
			AimMode = !AimMode;
			DefMainText(controllersCount, XboxGamepadEmuMode, AimMode, ChangeModesWithoutPress);
			SkipPollCount = SkipPollTimeOut;
		}

		if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState('W') & 0x8000) != 0 && SkipPollCount == 0)
		{
			ChangeModesWithoutPress = !ChangeModesWithoutPress;
			DefMainText(controllersCount, XboxGamepadEmuMode, AimMode, ChangeModesWithoutPress);
			SkipPollCount = SkipPollTimeOut;
		}

		XUSB_REPORT_INIT(&report);

		InputState = JslGetSimpleState(deviceID[0]);
		MotionState = JslGetMotionState(deviceID[0]);
		MotionAngles = QuaternionToEulerAngle(MotionState.quatW, MotionState.quatZ, MotionState.quatX, MotionState.quatY); // ?? correct?

		if (JslGetControllerType(deviceID[0]) == JS_TYPE_DS || JslGetControllerType(deviceID[0]) == JS_TYPE_DS4) {

			TouchState = JslGetTouchState(deviceID[0]);

			if (LockChangeBrightness == false && TouchState.t0Down && TouchState.t0Y <= 0.1 && TouchState.t0X >= 0.325 && TouchState.t0X <= 0.675) { // Brightness change
				GamepadOutState.LEDBrightness = 255 - std::clamp((int)((TouchState.t0X - 0.335) * 255 * 3.2), 0, 255);
				//printf("%5.2f %d\n", (TouchState.t0X - 0.335) * 255 * 3.2, GamepadOutState.LEDBrightness);
				GamepadSetState(GamepadOutState);
			}

			if ((InputState.buttons & JSMASK_TOUCHPAD_CLICK) || (TouchState.t0Down && ChangeModesWithoutPress)) {
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
					if (TouchState.t0X > 0 && TouchState.t0X <= 1 / 3.0 && GamepadMode != TouchpadSticksMode) { // [O--] - Driving mode
						GamepadMode = MotionDrivingMode;
						AnglesOffset = MotionAngles;
						GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
					
					} else if (TouchState.t0X > 1 / 3.0 && TouchState.t0X <= 1 / 3.0 * 2.0) { // [-O-] // Default & touch sticks modes
						
						if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.7) { // Default mode
							GamepadMode = GamepadDefaultMode;
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 0;
							// Show battery level
							GetBatteryInfo(); BackOutStateCounter = 40; GamepadOutState.PlayersCount = CurGamepad.BatteryLevel; GamepadSetState(GamepadOutState); // JslSetPlayerNumber(deviceID[0], 5);
							if (ResetOnDefaultMode) BTReset = true;
						} else {  // Touch sticks mode
							GamepadMode = TouchpadSticksMode;
							JslSetRumble(0, 255, 255);
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
						}

					} else if (TouchState.t0X > (1 / 3.0) * 2.0 && TouchState.t0X <= 1 && GamepadMode != TouchpadSticksMode) { // [--O] Aiming mode
						AnglesOffset = MotionAngles;
						if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.5) { // Motion AIM always
							GamepadMode = MotionAimingMode;
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
						}
						else { // Motion AIM with L2 trigger
							GamepadMode = MotionAimingModeOnlyPressed;
							GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
						}
					}
					BrightnessAreaPressed = 0; // Reset lock brightness if clicked in another area
					if (LockChangeBrightness == false) LockChangeBrightness = true;
				}
				GamepadSetState(GamepadOutState);
				//printf("current mode = %d\r\n", GamepadMode);
				if (XboxGamepadEmuMode == XboxGamepadOnlyDriving && GamepadMode != MotionDrivingMode) XboxGamepadReset = true; // Reset last state
			}

		}

		//printf("%5.2f\t%5.2f\r\n", InputState.stickLX, DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX));
		report.sThumbLX = InvertLeftStickX == false ? DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX) * 32767 : DeadZoneAxis(-InputState.stickLX, DeadZoneLeftStickX) * 32767;
		report.sThumbLY = InvertLeftStickX == false ? DeadZoneAxis(InputState.stickLY, DeadZoneLeftStickY) * 32767 : DeadZoneAxis(-InputState.stickLY, DeadZoneLeftStickY) * 32767;
		report.sThumbRX = InvertRightStickX == false ? DeadZoneAxis(InputState.stickRX, DeadZoneRightStickX) * 32767 : DeadZoneAxis(-InputState.stickRX, DeadZoneRightStickX) * 32767;
		report.sThumbRY = InvertRightStickY == false ? DeadZoneAxis(InputState.stickRY, DeadZoneRightStickY) * 32767 : DeadZoneAxis(-InputState.stickRY, DeadZoneRightStickY) * 32767;

		report.bLeftTrigger = InputState.lTrigger * 255;
		report.bRightTrigger = InputState.rTrigger * 255;
		if (JslGetControllerType(deviceID[0]) == JS_TYPE_DS || JslGetControllerType(deviceID[0]) == JS_TYPE_DS4) { 
			report.wButtons |= InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
			report.wButtons |= InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
		} else if (JslGetControllerType(deviceID[0]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_RIGHT) {
			report.wButtons |= InputState.buttons & JSMASK_CAPTURE ? XINPUT_GAMEPAD_BACK : 0;
			report.wButtons |= InputState.buttons & JSMASK_HOME ? XINPUT_GAMEPAD_START : 0;
		}
		report.wButtons |= InputState.buttons & JSMASK_L ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0;
		report.wButtons |= InputState.buttons & JSMASK_R ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0;
		report.wButtons |= InputState.buttons & JSMASK_LCLICK ? XINPUT_GAMEPAD_LEFT_THUMB : 0;
		report.wButtons |= InputState.buttons & JSMASK_RCLICK ? XINPUT_GAMEPAD_RIGHT_THUMB : 0;
		report.wButtons |= InputState.buttons & JSMASK_UP ? XINPUT_GAMEPAD_DPAD_UP : 0;
		report.wButtons |= InputState.buttons & JSMASK_DOWN ? XINPUT_GAMEPAD_DPAD_DOWN : 0;
		report.wButtons |= InputState.buttons & JSMASK_LEFT ? XINPUT_GAMEPAD_DPAD_LEFT : 0;
		report.wButtons |= InputState.buttons & JSMASK_RIGHT ? XINPUT_GAMEPAD_DPAD_RIGHT : 0;
		report.wButtons |= InputState.buttons & JSMASK_N ? XINPUT_GAMEPAD_Y : 0;
		report.wButtons |= InputState.buttons & JSMASK_W ? XINPUT_GAMEPAD_X : 0;
		report.wButtons |= InputState.buttons & JSMASK_S ? XINPUT_GAMEPAD_A : 0;
		report.wButtons |= InputState.buttons & JSMASK_E ? XINPUT_GAMEPAD_B : 0;

		// Nintendo controllers + - change working mode
		if (JslGetControllerType(deviceID[0]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(deviceID[0]) == JS_TYPE_JOYCON_RIGHT) {
			if (InputState.buttons & JSMASK_MINUS && SkipPollCount == 0) { if (GamepadMode == 1) GamepadMode = 0; else { GamepadMode = 1; AnglesOffset = MotionAngles; } SkipPollCount = SkipPollTimeOut; }
			if (InputState.buttons & JSMASK_PLUS && SkipPollCount == 0) { if (GamepadMode == 0 || GamepadMode == 1) GamepadMode = LastAIMProCtrlMode; else if (GamepadMode == 2) { GamepadMode = 3; LastAIMProCtrlMode = 3; } else { GamepadMode = 2; LastAIMProCtrlMode = 2; } SkipPollCount = SkipPollTimeOut; }
		} 

		if (InputState.buttons & JSMASK_PS && SkipPollCount == 0) {
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event('G', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event('G', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			SkipPollCount = SkipPollTimeOut;
		}
			
		if (InputState.buttons & JSMASK_MIC && SkipPollCount == 0) {
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			SkipPollCount = SkipPollTimeOut;
		}

		if (GamepadMode == MotionDrivingMode) // Motion racing  [O--]
			report.sThumbLX = ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Roll), RadToDeg(AnglesOffset.Roll)) * -1, MotionWheelAngle);
		else if (GamepadMode == MotionAimingMode || GamepadMode == MotionAimingModeOnlyPressed) { // Motion aiming  [--X}]
			float DeltaX = OffsetYPR(MotionAngles.Yaw, AnglesOffset.Yaw) * -1;
			float DeltaY = OffsetYPR(MotionAngles.Pitch, AnglesOffset.Pitch)  * -1;
			if (GamepadMode == MotionAimingMode || (GamepadMode == MotionAimingModeOnlyPressed && InputState.lTrigger > 0) )
				if (AimMode)
					MouseMove(RadToDeg(DeltaX) * MotionSensX, RadToDeg(DeltaY) * MotionSensY);
				else {
					report.sThumbRX = std::clamp((int)(ClampFloat((DeltaX) * JoySensX, -1, 1) * 32767 + report.sThumbRX), -32767, 32767);
					report.sThumbRY = std::clamp((int)(ClampFloat(-(DeltaY) * JoySensY, -1, 1) * 32767 + report.sThumbRY), -32767, 32767);
				}

			AnglesOffset = MotionAngles; // Not the best way but it works
		} else if (GamepadMode == TouchpadSticksMode) { // [-_-] Touchpad sticks

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

		if (XboxGamepadEmuMode == XboxGamepadEnabled || (XboxGamepadEmuMode == XboxGamepadOnlyDriving && GamepadMode == MotionDrivingMode) || XboxGamepadReset) {
			if (XboxGamepadReset) { XboxGamepadReset = false; XUSB_REPORT_INIT(&report); }
			ret = vigem_target_x360_update(client, x360, report);
		}

		// Battery level display
		if (BackOutStateCounter > 0) { if (BackOutStateCounter == 1) { GamepadOutState.PlayersCount = 0; GamepadSetState(GamepadOutState); } BackOutStateCounter--; } 
		
		if (SkipPollCount > 0) SkipPollCount--;
		Sleep(SleepTimeOut);
	}

	JslDisconnectAndDisposeAll();
	if (CurGamepad.HidHandle != NULL)
		hid_close(CurGamepad.HidHandle);

	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
}
