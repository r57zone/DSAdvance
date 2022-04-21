#include <windows.h>
#include <math.h>
#include <mutex>
#include <iostream>
#include <ViGEm/Client.h>
#include "IniReader\IniReader.h"
#include "JoyShockLibrary\JoyShockLibrary.h"
#include "hidapi.h"
#include "DSAdvance.h"

struct Gamepad {
	hid_device *HidHandle;
	WORD ControllerType;
	bool USBConnection;
	wchar_t *serial_number;
};

Gamepad Gamepads[4];
WORD gamepadsCount;

InputOutState GamepadOutState;
void GamepadSetState(InputOutState OutState)
{
	if (Gamepads[0].HidHandle != NULL) {
		if (Gamepads[0].ControllerType == SONY_DUALSENSE && Gamepads[0].USBConnection) {
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
			outputReport[45] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
			outputReport[46] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
			outputReport[47] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

			hid_write(Gamepads[0].HidHandle, outputReport, 48);
		}

		else if (Gamepads[0].ControllerType == SONY_DUALSHOCK4 && Gamepads[0].USBConnection) {
			unsigned char outputReport[31];
			memset(outputReport, 0, 31);

			outputReport[0] = 0x05;
			outputReport[1] = 0xff;
			outputReport[4] = OutState.SmallMotor;
			outputReport[5] = OutState.LargeMotor;
			outputReport[6] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
			outputReport[7] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
			outputReport[8] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

			hid_write(Gamepads[0].HidHandle, outputReport, 31);
		}
	}
}

void GamepadsSearch() {
	struct hid_device_info *cur_dev;
	gamepadsCount = 0;

	cur_dev = hid_enumerate(SONY_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == SONY_DS5_USB || cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB)
		{
			Gamepads[gamepadsCount].HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(Gamepads[gamepadsCount].HidHandle, 1);
			
			if (cur_dev->product_id == SONY_DS5_USB)
				Gamepads[gamepadsCount].ControllerType = SONY_DUALSENSE;
			else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB)
				Gamepads[gamepadsCount].ControllerType = SONY_DUALSHOCK4;
			
			Gamepads[gamepadsCount].USBConnection = true;
			gamepadsCount++;
			//printf("DS found\n");
		}

		cur_dev = cur_dev->next;

		if (gamepadsCount = DS_MAX_COUNT)
			break;
	}

	if (gamepadsCount > 0) // Return normal count
		gamepadsCount--;

	if (cur_dev)
		hid_free_enumeration(cur_dev);
}

void GamepadsFree() {
	gamepadsCount = 0;
	for (int i = 0; i <= gamepadsCount; i++)
		hid_close(Gamepads[i].HidHandle);
}

static std::mutex m;

struct EulerAngles {
	double Yaw;
	double Pitch;
	double Roll;
};

EulerAngles QuaternionToEulerAngle(double qW, double qX, double qY, double qZ)
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
	if (f < -180) {
		f += 360;
	} else if (f > 180) {
		f -= 360; }
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

// Implementation from https://github.com/JibbSmart/JoyShockMapper/blob/master/JoyShockMapper/src/win32/InputHelpers.cpp
float accumulatedX = 0, accumulatedY = 0;
void MouseMove(float x, float y) {
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

int main(int argc, char **argv)
{
	SetConsoleTitle("DSAdvance 0.1");
	// Config parameters
	CIniReader IniFile("Config.ini");
	int KEY_ID_EXIT = IniFile.ReadInteger("Main", "ExitBtn", 192); // "~" by default for RU, US and not for UK
	bool InvertX = IniFile.ReadBoolean("Main", "InvertX", false);
	bool InvertY = IniFile.ReadBoolean("Main", "InvertY", false);
	int SleepTimeOut = IniFile.ReadInteger("Main", "SleepTimeOut", 1);

	float DeadZoneLeftStickX = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickX", 0);
	float DeadZoneLeftStickY = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickY", 0);
	float DeadZoneRightStickX = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickX", 0);
	float DeadZoneRightStickY = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickY", 0);

	float TouchLeftStickX = IniFile.ReadFloat("Gamepad", "TouchLeftStickX", 4);
	float TouchLeftStickY = IniFile.ReadFloat("Gamepad", "TouchLeftStickY", 4);
	float TouchRightStickX = IniFile.ReadFloat("Gamepad", "TouchRightStickX", 4);
	float TouchRightStickY = IniFile.ReadFloat("Gamepad", "TouchRightStickY", 4);

	float MotionWheelAngle = IniFile.ReadFloat("Motion", "WheelAngle", 75);
	float MotionSensX = IniFile.ReadFloat("Motion", "SensX", 3);
	float MotionSensY = IniFile.ReadFloat("Motion", "SensY", 3);

	GamepadsSearch();
	GamepadOutState.LEDBlue = 255;
	GamepadOutState.LEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("Gamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	GamepadSetState(GamepadOutState);

	int SkipPollCount = 0;
	bool DeadZoneMode = false;
	int GamepadMode = 0;
	EulerAngles AnglesOffset;

	int ñontrollersCount = JslConnectDevices();
	int deviceID[4]; // max playes 12
	JslGetConnectedDeviceHandles(deviceID, ñontrollersCount);

	system("cls");
	if (ñontrollersCount == 0)
		printf("\n Connect DualSense or DualShock 4 via USB and restart the app.");
	printf("\n Press \"ALT\" + \"Escape\" or \"exit key\" to exit.\n");
	
	JOY_SHOCK_STATE InputState;
	MOTION_STATE MotionState;
	TOUCH_STATE TouchState;

	const auto client = vigem_alloc();
	auto ret = vigem_connect(client);

	const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);

	XUSB_REPORT report;

	bool FirstTouch, SecondTouch;
	float FirstTouchAxisRX, FirstTouchAxisRY, TouchAxisRX, TouchAxisRY, FirstTouchAxisLX, FirstTouchAxisLY, TouchAxisLX, TouchAxisLY;

	while (!((GetAsyncKeyState(KEY_ID_EXIT) & 0x8000) || ((GetAsyncKeyState(VK_LMENU) & 0x8000) && (GetAsyncKeyState(VK_ESCAPE) & 0x8000)))) // "~" by default
	{
		// Dead zones
		if ((GetAsyncKeyState(VK_F9) & 0x8000) != 0 && ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) && SkipPollCount == 0)
		{
			DeadZoneMode = !DeadZoneMode;
			if (DeadZoneMode == false) {
				system("cls");
				printf("\n Press \"ALT\" + \"Escape\" or \"exit key\" to exit.\n");
			}
			SkipPollCount = 15;
		}

		if (DeadZoneMode) {
			printf(" Left Stick X=%6.2f, ", abs(InputState.stickLX));
			printf("Y=%6.2f\t", abs(InputState.stickLY));
			printf("Right Stick X=%6.2f ", abs(InputState.stickRX));
			printf("Y=%6.2f\n", abs(InputState.stickRY));
		}

		XUSB_REPORT_INIT(&report);

		InputState = JslGetSimpleState(deviceID[0]);
		MotionState = JslGetMotionState(deviceID[0]);

		EulerAngles MotionAngles;
		MotionAngles = QuaternionToEulerAngle(MotionState.quatW, MotionState.quatZ, MotionState.quatX, MotionState.quatY); // ?? correct?

		TouchState = JslGetTouchState(deviceID[0]);

		if (TouchState.t0Down && TouchState.t0Y <= 0.1 && TouchState.t0X >= 0.325 && TouchState.t0X <= 0.675) {
			GamepadOutState.LEDBrightness = std::clamp((int)((TouchState.t0X - 0.325) * 255 * 2.9), 0, 255);
			//printf("%5.2f\n", (TouchState.t0X - 0.325) * 255 * 2.9);
			GamepadSetState(GamepadOutState);
		}

		if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) {
			if (TouchState.t0Y > 0.1) {
				if (TouchState.t0X > 0 && TouchState.t0X <= 1 / 3.0 && GamepadMode != 4) { // [O--]
					GamepadMode = 1;
					AnglesOffset = MotionAngles;
					GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
				}
				else if (TouchState.t0X > 1 / 3.0 && TouchState.t0X <= (1 / 3.0) * 2.0) { // [-O-]

					if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.5) {
						GamepadMode = 0;
						GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 0;
					} else { 
						GamepadMode = 4;
						GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
					}

				} else if (TouchState.t0X > (1 / 3.0) * 2.0 && TouchState.t0X <= 1 && GamepadMode != 4) { // [--O]

					if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.5) { // Motion AIM always
						GamepadMode = 3;
						GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255; 
					} else { // Motion AIM with L2 trigger
						GamepadMode = 2;
						GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
					}
				}
			}
			GamepadSetState(GamepadOutState);
			//printf("current mode = %d\r\n", GamepadMode);
		}

		//printf("%5.2f\t%5.2f\r\n", InputState.stickLX, DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX));
		report.sThumbLX = DeadZoneAxis(InputState.stickLX, DeadZoneLeftStickX) * 32767;
		report.sThumbLY = DeadZoneAxis(InputState.stickLY, DeadZoneLeftStickY) * 32767;
		report.sThumbRX = DeadZoneAxis(InputState.stickRX, DeadZoneRightStickX) * 32767;
		report.sThumbRY = DeadZoneAxis(InputState.stickRY, DeadZoneRightStickY) * 32767;

		report.bLeftTrigger = InputState.lTrigger * 255;
		report.bRightTrigger = InputState.rTrigger * 255;
		report.wButtons |= InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
		report.wButtons |= InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
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

		if (InputState.buttons & JSMASK_PS && SkipPollCount == 0) {
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event('G', 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event('G', 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			SkipPollCount = 15;
		}
			
		if (InputState.buttons & JSMASK_MIC && SkipPollCount == 0) {
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_LWIN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			SkipPollCount = 15;
		}

		if (GamepadMode == 1) // Motion racing  [O--]
			report.sThumbLX = ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Roll), RadToDeg(AnglesOffset.Roll)) * -1, MotionWheelAngle);
		else if (GamepadMode == 2 || GamepadMode == 3) { // Motion aiming  [--õ]
			float NewX = OffsetYPR(RadToDeg(MotionAngles.Yaw), RadToDeg(AnglesOffset.Yaw)) * -1;
			float NewY = OffsetYPR(RadToDeg(MotionAngles.Pitch), RadToDeg(AnglesOffset.Pitch))  * -1;
			if (GamepadMode == 3 || (GamepadMode == 2 && InputState.lTrigger > 0) ) 
				MouseMove(NewX * MotionSensX, NewY * MotionSensY);
			AnglesOffset = MotionAngles;
		}
		else if (GamepadMode == 4) { // [-_-] Touchpad sticks

			if (TouchState.t0Down) {
				if (FirstTouch == false) {
					FirstTouchAxisRX = TouchState.t0X;
					FirstTouchAxisRY = TouchState.t0Y;
					FirstTouch = true;
				}
				TouchAxisRX = TouchState.t0X - FirstTouchAxisRX;
				TouchAxisRY = TouchState.t0Y - FirstTouchAxisRY;
			} else {
				TouchAxisRX = 0;
				TouchAxisRY = 0;
				FirstTouch = false;
			}

			if (TouchState.t1Down) {
				if (SecondTouch == false) {
					FirstTouchAxisLX = TouchState.t1X;
					FirstTouchAxisLY = TouchState.t1Y;
					SecondTouch = true;
				}
				TouchAxisLX = TouchState.t1X - FirstTouchAxisLX;
				TouchAxisLY = TouchState.t1Y - FirstTouchAxisLY;
				//TouchAxisLX = ClampFloat(TouchAxisLX * TouchLeftStickX, -1, 1);
				//TouchAxisLY = ClampFloat(-TouchAxisLY * TouchLeftStickY, -1, 1);
			} else {
				TouchAxisLX = 0;
				TouchAxisLY = 0;
				SecondTouch = false;
			}

			// Output to sticks
			if (FirstTouchAxisRX > 0.5 ) {
				if (TouchAxisRX != 0) report.sThumbRX = ClampFloat(TouchAxisRX * TouchRightStickX, -1, 1) * 32767;
				if (TouchAxisRY != 0) report.sThumbRY = ClampFloat(-TouchAxisRY * TouchRightStickY, -1, 1) * 32767;
				if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
			} else {
				if (TouchAxisRX != 0) report.sThumbLX = ClampFloat(TouchAxisRX * TouchLeftStickX, -1, 1) * 32767;
				if (TouchAxisRY != 0) report.sThumbLY = ClampFloat(-TouchAxisRY * TouchLeftStickY, -1, 1) * 32767;
				if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
			}

			if (FirstTouchAxisLX < 0.5) {
				if (TouchAxisLX != 0) report.sThumbLX = ClampFloat(TouchAxisLX * TouchLeftStickX, -1, 1) * 32767;
				if (TouchAxisLY != 0) report.sThumbLY = ClampFloat(-TouchAxisLY * TouchLeftStickY, -1, 1) * 32767;
			} else {
				if (TouchAxisLX != 0) report.sThumbRX = ClampFloat(TouchAxisLX * TouchRightStickX, -1, 1) * 32767;
				if (TouchAxisLY != 0) report.sThumbRY = ClampFloat(-TouchAxisLY * TouchRightStickY, -1, 1) * 32767;
			}
		}

		ret = vigem_target_x360_update(client, x360, report);

		if (SkipPollCount > 0) SkipPollCount--;
		Sleep(SleepTimeOut);
	}

	JslDisconnectAndDisposeAll();
	GamepadsFree();

	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
}
