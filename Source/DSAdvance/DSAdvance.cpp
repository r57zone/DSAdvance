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

Gamepad CurGamepad;

InputOutState GamepadOutState;
void GamepadSetState(InputOutState OutState)
{
	if (CurGamepad.HidHandle != NULL) {
		if (CurGamepad.ControllerType == SONY_DUALSENSE && CurGamepad.USBConnection) { // https://github.com/broken-bytes/DualSense4Windows/blob/master/src/DualSense.cxx & https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/
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

			hid_write(CurGamepad.HidHandle, outputReport, 48);

		} else if (CurGamepad.ControllerType == SONY_DUALSHOCK4 && CurGamepad.USBConnection) { // JyoShockLibrary rumble working for USB DS4 ??? 
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
	}
}

void GamepadSearch() {
	struct hid_device_info *cur_dev;
	cur_dev = hid_enumerate(SONY_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == SONY_DS5_USB || cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB)
		{
			CurGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(CurGamepad.HidHandle, 1);
			
			if (cur_dev->product_id == SONY_DS5_USB)
				CurGamepad.ControllerType = SONY_DUALSENSE;
			else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB)
				CurGamepad.ControllerType = SONY_DUALSHOCK4;
			
			CurGamepad.USBConnection = true;
			//printf("DS found\n");
			break;
		}
		cur_dev = cur_dev->next;
	}
	if (cur_dev)
		hid_free_enumeration(cur_dev);
}

static std::mutex m;

struct EulerAngles {
	double Yaw;
	double Pitch;
	double Roll;
};

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

int main(int argc, char **argv)
{
	SetConsoleTitle("DSAdvance 0.2");
	// Config parameters
	CIniReader IniFile("Config.ini");
	int KEY_ID_EXIT = IniFile.ReadInteger("Main", "ExitBtn", 192); // "~" by default for RU, US and not for UK

	bool InvertLeftStickX = IniFile.ReadBoolean("Gamepad", "InvertLeftStickX", false);
	bool InvertLeftStickY = IniFile.ReadBoolean("Gamepad", "InvertLeftStickY", false);
	bool InvertRightStickX = IniFile.ReadBoolean("Gamepad", "InvertRightStickX", false);
	bool InvertRightStickY = IniFile.ReadBoolean("Gamepad", "InvertRightStickY", false);
	int SleepTimeOut = IniFile.ReadInteger("Gamepad", "SleepTimeOut", 1);

	float DeadZoneLeftStickX = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickX", 0);
	float DeadZoneLeftStickY = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickY", 0);
	float DeadZoneRightStickX = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickX", 0);
	float DeadZoneRightStickY = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickY", 0);

	float TouchLeftStickX = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensX", 4);
	float TouchLeftStickY = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensY", 4);
	float TouchRightStickX = IniFile.ReadFloat("Gamepad", "TouchRightStickSensX", 4);
	float TouchRightStickY = IniFile.ReadFloat("Gamepad", "TouchRightStickSensY", 4);

	float MotionWheelAngle = IniFile.ReadFloat("Motion", "WheelAngle", 75);
	float MotionSensX = IniFile.ReadFloat("Motion", "SensX", 3);
	float MotionSensY = IniFile.ReadFloat("Motion", "SensY", 3);

	GamepadSearch();
	GamepadOutState.LEDBlue = 255;
	GamepadOutState.LEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("Gamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	GamepadSetState(GamepadOutState);

	int SkipPollCount = 0;
	bool DeadZoneMode = false;
	int GamepadMode = 0; int LastAIMProCtrlMode = 2;
	EulerAngles AnglesOffset;

	int controllersCount = JslConnectDevices();
	int deviceID[4];
	JslGetConnectedDeviceHandles(deviceID, controllersCount);

	system("cls");
	if (controllersCount == 0)
		printf("\n Connect DualSense, DualShock 4, Pro controller via USB and restart the app.");
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
	float InitFirstTouchAxisX, InitFirstTouchAxisY, FirstTouchAxisX, FirstTouchAxisY, InitSecondTouchAxisX, InitSecondTouchAxisY, SecondTouchAxisX, SecondTouchAxisY, TouchAxisLX, TouchAxisLY, TouchAxisRX, TouchAxisRY;
	
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

		if (JslGetControllerType(deviceID[0]) == JS_TYPE_DS || JslGetControllerType(deviceID[0]) == JS_TYPE_DS4) {

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
					else if (TouchState.t0X > 1 / 3.0 && TouchState.t0X <= 1 / 3.0 * 2.0) { // [-O-]

						if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.5) {
							GamepadMode = 0;
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 0;
							// Show battery level - JslSetPlayerNumber(deviceID[0], 5);
						}
						else {
							GamepadMode = 4;
							JslSetRumble(0, 255, 255);
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 255; GamepadOutState.LEDGreen = 0;
						}

					}
					else if (TouchState.t0X > (1 / 3.0) * 2.0 && TouchState.t0X <= 1 && GamepadMode != 4) { // [--O]

						if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.5) { // Motion AIM always
							GamepadMode = 3;
							GamepadOutState.LEDBlue = 255; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
						}
						else { // Motion AIM with L2 trigger
							GamepadMode = 2;
							GamepadOutState.LEDBlue = 0; GamepadOutState.LEDRed = 0; GamepadOutState.LEDGreen = 255;
						}
					}
				}
				GamepadSetState(GamepadOutState);
				//printf("current mode = %d\r\n", GamepadMode);
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
			if (InputState.buttons & JSMASK_MINUS && SkipPollCount == 0) { if (GamepadMode == 1) GamepadMode = 0; else { GamepadMode = 1; AnglesOffset = MotionAngles; } SkipPollCount = 15; }
			if (InputState.buttons & JSMASK_PLUS && SkipPollCount == 0) { if (GamepadMode == 0 || GamepadMode == 1) GamepadMode = LastAIMProCtrlMode; else if (GamepadMode == 2) { GamepadMode = 3; LastAIMProCtrlMode = 3; } else { GamepadMode = 2; LastAIMProCtrlMode = 2; } SkipPollCount = 15; }
		} 

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
					InitFirstTouchAxisX = TouchState.t0X;
					InitFirstTouchAxisY = TouchState.t0Y;
					FirstTouch = true;
				}
				FirstTouchAxisX = TouchState.t0X - InitFirstTouchAxisX;
				FirstTouchAxisY = TouchState.t0Y - InitFirstTouchAxisY;

				if (InitFirstTouchAxisX < 0.5 ) {
					report.sThumbLX = ClampFloat(FirstTouchAxisX * TouchLeftStickX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-FirstTouchAxisY * TouchLeftStickY, -1, 1) * 32767;
					if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
				} else {
					report.sThumbRX = ClampFloat(FirstTouchAxisX * TouchRightStickX, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-FirstTouchAxisY * TouchRightStickY, -1, 1) * 32767;
					//MouseMove(FirstTouchAxisX * 0.25, FirstTouchAxisY * 0.25);
					if (InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
				}
			} else {
				FirstTouchAxisX = 0;
				FirstTouchAxisY = 0;
				FirstTouch = false;
			}

			if (TouchState.t1Down) {
				if (SecondTouch == false) {
					InitSecondTouchAxisX = TouchState.t1X;
					InitSecondTouchAxisY = TouchState.t1Y;
					SecondTouch = true;
				}
				SecondTouchAxisX = TouchState.t1X - InitSecondTouchAxisX;
				SecondTouchAxisY = TouchState.t1Y - InitSecondTouchAxisY;

				if (InitSecondTouchAxisX < 0.5) {
					report.sThumbLX = ClampFloat(SecondTouchAxisX * TouchLeftStickX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-SecondTouchAxisY * TouchLeftStickY, -1, 1) * 32767;
				} else {
					report.sThumbRX = ClampFloat(SecondTouchAxisX * TouchRightStickX, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-SecondTouchAxisY * TouchRightStickY, -1, 1) * 32767;
				}
			} else {
				SecondTouchAxisX = 0;
				SecondTouchAxisY = 0;
				SecondTouch = false;
			}
		
		}

		ret = vigem_target_x360_update(client, x360, report);

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
