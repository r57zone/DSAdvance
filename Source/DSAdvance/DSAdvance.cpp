// DSAdvance by r57zone
// Advanced Xbox controller emulation for DualSense, DualShock 4, Pro Controller, Joy-Cons
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
#include <dbt.h>
#include <chrono>
#include <mmsystem.h>
#include <locale.h>

#pragma comment(lib, "winmm.lib")

void GamepadSearch() {
	//PrimaryGamepad.ControllerType = EMPTY_CONTROLLER;
	//SecondaryGamepad.ControllerType = EMPTY_CONTROLLER;

	struct hid_device_info *cur_dev;

	// Sony controllers
	cur_dev = hid_enumerate(SONY_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == SONY_DS5 ||
			cur_dev->product_id == SONY_DS5_EDGE ||
			cur_dev->product_id == SONY_DS4_USB ||
			cur_dev->product_id == SONY_DS4_V2_USB ||
			cur_dev->product_id == SONY_DS4_BT ||
			cur_dev->product_id == SONY_DS4_DONGLE)
		{
			PrimaryGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(PrimaryGamepad.HidHandle, 1);

			if (cur_dev->product_id == SONY_DS5 || cur_dev->product_id == SONY_DS5_EDGE) {
				PrimaryGamepad.ControllerType = SONY_DUALSENSE;
				PrimaryGamepad.USBConnection = true;

				// BT detection https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp
				unsigned char buf[64];
				memset(buf, 0, 64);
				hid_read_timeout(PrimaryGamepad.HidHandle, buf, 64, 100);
				if (buf[0] == 0x31)
					PrimaryGamepad.USBConnection = false;

			} else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB || cur_dev->product_id == SONY_DS4_DONGLE) {
				PrimaryGamepad.ControllerType = SONY_DUALSHOCK4;
				PrimaryGamepad.USBConnection = true;

				// JoyShock Library apparently sent something, so it worked without a package (needed for BT detection to work, does not affect USB)
				unsigned char checkBT[2] = { 0x02, 0x00 };
				hid_write(PrimaryGamepad.HidHandle, checkBT, sizeof(checkBT));

				// BT detection for compatible gamepads that output USB VID/PID on BT connection
				unsigned char buf[64];
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read_timeout(PrimaryGamepad.HidHandle, buf, sizeof(buf), 100);
				if (bytesRead > 0 && buf[0] == 0x11)
					PrimaryGamepad.USBConnection = false;

				//printf("Detected device ID: 0x%X\n", cur_dev->product_id);
				//if (PrimaryGamepad.USBConnection) printf("USB"); else printf("Wireless");

			} else if (cur_dev->product_id == SONY_DS4_BT) { // ?
				PrimaryGamepad.ControllerType = SONY_DUALSHOCK4;
				PrimaryGamepad.USBConnection = false;
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
			PrimaryGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(PrimaryGamepad.HidHandle, 1);
			PrimaryGamepad.USBConnection = true;
			PrimaryGamepad.ControllerType = SONY_DUALSHOCK4;
			break;
		}
		cur_dev = cur_dev->next;
	}

	// Nintendo compatible controllers
	cur_dev = hid_enumerate(NINTENDO_VENDOR, 0x0);
	while (cur_dev) {
		if (cur_dev->product_id == NINTENDO_JOYCON_L)
		{
			PrimaryGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(PrimaryGamepad.HidHandle, 1);
			PrimaryGamepad.USBConnection = false;
			PrimaryGamepad.ControllerType = NINTENDO_JOYCONS;
		
		} else if (cur_dev->product_id == NINTENDO_JOYCON_R) {
			PrimaryGamepad.HidHandle2 = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(PrimaryGamepad.HidHandle2, 1);
			PrimaryGamepad.USBConnection = false;
			PrimaryGamepad.ControllerType = NINTENDO_JOYCONS;
		
		} else if (cur_dev->product_id == NINTENDO_SWITCH_PRO) {
			PrimaryGamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			PrimaryGamepad.ControllerType = NINTENDO_SWITCH_PRO;
			hid_set_nonblocking(PrimaryGamepad.HidHandle, 1);
			PrimaryGamepad.USBConnection = true;

			// BT detection
			unsigned char buf[64];
			memset(buf, 0, sizeof(buf));
			int bytesRead = hid_read_timeout(PrimaryGamepad.HidHandle, buf, sizeof(buf), 100);
			if (bytesRead > 0 && buf[0] == 0x11)
				PrimaryGamepad.USBConnection = false;
		}
		cur_dev = cur_dev->next;
	}

	if (cur_dev)
		hid_free_enumeration(cur_dev);
}

void GamepadSetState(InputOutState OutState)
{
	if (PrimaryGamepad.HidHandle != NULL) {
		if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) { // https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/

			unsigned char PlayersDSPacket = 0;

			if (OutState.PlayersCount == 0) PlayersDSPacket = 0;
			else if (OutState.PlayersCount == 1) PlayersDSPacket = 4;
			else if (OutState.PlayersCount == 2) PlayersDSPacket = 2; // Center 2
			else if (OutState.PlayersCount == 5) PlayersDSPacket = 1; // Both 2
			else if (OutState.PlayersCount == 3) PlayersDSPacket = 5;
			else if (OutState.PlayersCount == 4) PlayersDSPacket = 3;

			OutState.LEDRed = (OutState.LEDColor >> 16) & 0xFF;
			OutState.LEDGreen = (OutState.LEDColor >> 8) & 0xFF;
			OutState.LEDBlue = OutState.LEDColor & 0xFF;

			if (PrimaryGamepad.USBConnection) {
				unsigned char outputReport[48];
				memset(outputReport, 0, 48);

				outputReport[0] = 0x02;
				outputReport[1] = 0xff;
				outputReport[2] = 0x15;
				outputReport[3] = (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[4] = (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[5] = 0xff;
				outputReport[6] = 0xff;
				outputReport[7] = 0xff;
				outputReport[8] = 0x0c;
				//outputReport[9] = OutState.MicLED;
				outputReport[38] = 0x07;
				outputReport[44] = PlayersDSPacket;
				outputReport[45] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[46] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[47] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				hid_write(PrimaryGamepad.HidHandle, outputReport, 48);

			// DualSense BT
			} else { 
				unsigned char outputReport[79]; // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds5_rumble_light_bt)
				memset(outputReport, 0, 79);

				outputReport[0] = 0xa2;
				outputReport[1] = 0x31;
				outputReport[2] = 0x02;
				outputReport[3] = 0x03;
				outputReport[4] = 0x54;
				outputReport[5] = (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[6] = (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[11] = 0x00; // OutState.MicLED - not working
				outputReport[41] = 0x02;
				outputReport[44] = 0x02;
				outputReport[45] = 0x02;
				outputReport[46] = PlayersDSPacket;
				//outputReport[46] &= ~(1 << 7);
				//outputReport[46] &= ~(1 << 8);
				outputReport[47] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[48] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[49] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				uint32_t crc = crc_32(outputReport, 75);
				memcpy(&outputReport[75], &crc, 4);

				hid_write(PrimaryGamepad.HidHandle, &outputReport[1], 78);
			}

		} else if (PrimaryGamepad.ControllerType == SONY_DUALSHOCK4) { // JoyShockLibrary rumble working for USB DS4 ??? 
			OutState.LEDRed = (OutState.LEDColor >> 16) & 0xFF;
			OutState.LEDGreen = (OutState.LEDColor >> 8) & 0xFF;
			OutState.LEDBlue = OutState.LEDColor & 0xFF;

			if (PrimaryGamepad.USBConnection) {
				unsigned char outputReport[31];
				memset(outputReport, 0, 31);

				outputReport[0] = 0x05;
				outputReport[1] = 0xff;
				outputReport[4] = (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[5] = (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[6] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[7] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[8] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				hid_write(PrimaryGamepad.HidHandle, outputReport, 31);
			
			// DualShock 4 BT
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

				outputReport[7] = (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength / 100;
				outputReport[8] = (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength / 100;

				outputReport[9] = std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255);
				outputReport[10] = std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255);
				outputReport[11] = std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255);

				outputReport[12] = 0xff;
				outputReport[13] = 0x00;

				uint32_t crc = crc_32(outputReport, 75);
				memcpy(&outputReport[75], &crc, 4);

				hid_write(PrimaryGamepad.HidHandle, &outputReport[1], 78);
			}
		}
		else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS) {
			if (PrimaryGamepad.RumbleStrength != 0) {
				// Left Joycon
				/*unsigned char outputReport[64] = { 0 };
				outputReport[0] = 0x10;
				outputReport[1] = (++PrimaryGamepad.PacketCounter) & 0xF; if (PrimaryGamepad.PacketCounter > 0xF) PrimaryGamepad.PacketCounter = 0x0;
				outputReport[2] = std::clamp(OutState.SmallMotor - 0, 0, 229); // It seems that it is not recommended to use the Nintendo Switch motors at 100 % , there is a risk of damaging them, so we will limit ourselves to 90%

				hid_write(PrimaryGamepad.HidHandle, outputReport, 10);

				// Right Joycon
				if (PrimaryGamepad.HidHandle2 != NULL) {
					outputReport[6] = std::clamp(OutState.LargeMotor - 0, 0, 229);
					hid_write(PrimaryGamepad.HidHandle2, outputReport, 10);
				}*/

				// Left JoyCon
				unsigned char outputReportLeft[64] = { 0 };
				outputReportLeft[0] = 0x10;
				outputReportLeft[1] = (++PrimaryGamepad.PacketCounter) & 0xF; if (PrimaryGamepad.PacketCounter > 0xF) PrimaryGamepad.PacketCounter = 0x0;
				outputReportLeft[2] = (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength * 90 / 10000; // std::clamp(OutState.SmallMotor - 0, 0, 229); // It seems that it is not recommended to use the Nintendo Switch motors at 100 % , there is a risk of damaging them, so we will limit ourselves to 90%
				outputReportLeft[3] = 0x00;
				outputReportLeft[4] = OutState.SmallMotor == 0 ? 0x00 : 0x01;
				outputReportLeft[5] = 0x40;

				hid_write(PrimaryGamepad.HidHandle, outputReportLeft, 64);

				// Right JoyCon
				if (PrimaryGamepad.HidHandle2 != NULL) {
					unsigned char outputReportRight[64] = { 0 };
					outputReportRight[0] = 0x10;
					outputReportRight[1] = (PrimaryGamepad.PacketCounter) & 0xF;
					outputReportRight[2] = 0x00;
					outputReportRight[3] = 0x00;
					outputReportRight[4] = OutState.LargeMotor == 0 ? 0x00 : 0x01;
					outputReportRight[5] = 0x40;
					outputReportRight[6] = OutState.LargeMotor == 0 ? 0x00 : std::clamp(OutState.LargeMotor - 0, 0, 229);

					hid_write(PrimaryGamepad.HidHandle2, outputReportRight, 64);
				}
			
				if (OutState.SmallMotor == 0 && OutState.LargeMotor == 0) PrimaryGamepad.RumbleOffCounter = 2; // Looks like Nintendo needs some "0" rumble packets to stop it
			}
		} else if (PrimaryGamepad.ControllerType == NINTENDO_SWITCH_PRO) { // Working only one motor :(
			if (PrimaryGamepad.RumbleStrength != 0) {
				unsigned char outputReport[64] = { 0 };

				/* // BT ???
				if (!PrimaryGamepad.USBConnection) {
					outputReport[0] = 0x80; // Заголовок для Bluetooth
					outputReport[1] = 0x92; // Команда вибрации для Bluetooth
					outputReport[3] = 0x31; // Подкоманда для вибрации
					outputReport[8] = 0x10; // Дополнительные данные для Bluetooth
				}*/

				outputReport[0] = 0x10;
				outputReport[1] = (++PrimaryGamepad.PacketCounter) & 0xF; if (PrimaryGamepad.PacketCounter > 0xF) PrimaryGamepad.PacketCounter = 0x0;

				// Amplitudes
				unsigned char hf = 0x20; // High frequency
				unsigned char lf = 0x28; // Low frequency
				unsigned char h_amp = (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength * 90 / 10000; // std::clamp(OutState.SmallMotor * 2 / 229, 0, 255); // It seems that it is not recommended to use the Nintendo Switch motors at 100 % , there is a risk of damaging them, so we will limit ourselves to 90%
				unsigned char l_amp1 = (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength * 90 / 10000; // std::clamp(OutState.LargeMotor / 229, 0, 255);
				unsigned char l_amp2 = ((l_amp1 % 2) * 128);
				l_amp1 = (l_amp1 / 2) + 64;

				outputReport[2] = hf;
				outputReport[3] = h_amp;
				outputReport[4] = lf + l_amp2;
				outputReport[5] = l_amp1;

				if (!PrimaryGamepad.USBConnection) {
					outputReport[0] = 0x80;
					outputReport[1] = 0x92;
					outputReport[3] = 0x31;
					outputReport[8] = 0x10;
				} 

				hid_write(PrimaryGamepad.HidHandle, outputReport, 64);

				if (OutState.SmallMotor == 0 && OutState.LargeMotor == 0) PrimaryGamepad.RumbleOffCounter = 2; // Looks like Nintendo needs some "0" rumble packets to stop it
			}
		} else {
			//if (JslGetControllerType(0) == JS_TYPE_DS || JslGetControllerType(0) == JS_TYPE_DS4)
				//JslSetLightColour(0, (std::clamp(OutState.LEDRed - OutState.LEDBrightness, 0, 255) << 16) + (std::clamp(OutState.LEDGreen - OutState.LEDBrightness, 0, 255) << 8) + std::clamp(OutState.LEDBlue - OutState.LEDBrightness, 0, 255)); // https://github.com/CyberPlaton/_Nautilus_/blob/master/Engine/PSGamepad.cpp
			JslSetRumble(0, (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength / 100, (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength / 100); // Not working with DualSense USB connection
		}
	} else // Unknown controllers - Pro controller, Joy-cons
		JslSetRumble(0, (unsigned int)OutState.LargeMotor * PrimaryGamepad.RumbleStrength / 100, (unsigned int)OutState.SmallMotor * PrimaryGamepad.RumbleStrength / 100);
}

void GetBatteryInfo() {
	if (PrimaryGamepad.HidHandle != NULL) {
		if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(PrimaryGamepad.HidHandle, buf, 64);
			if (PrimaryGamepad.USBConnection) {
				PrimaryGamepad.LEDBatteryLevel = (buf[53] & 0x0f) / 2 + 1; // "+1" for the LED to be responsible for 25%. Each unit of battery data corresponds to 10%, 0 = 0 - 9 % , 1 = 10 - 19 % , .. and 10 = 100 %
				//??? in charge mode, need to show animation within a few seconds 
				PrimaryGamepad.BatteryMode = ((buf[52] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT; // 0x0 - discharging, 0x1 - full, 0x2 - charging, 0xa & 0xb - not-charging, 0xf - unknown
				//printf(" Battery status: %d\n", PrimaryGamepad.BatteryMode); // if there is charging, then we do not add 1 led
				PrimaryGamepad.BatteryLevel = (buf[53] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
			} else { // BT
				PrimaryGamepad.LEDBatteryLevel = (buf[54] & 0x0f) / 2 + 1;
				PrimaryGamepad.BatteryMode = ((buf[53] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT;
				PrimaryGamepad.BatteryLevel = (buf[54] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
			}
			if (PrimaryGamepad.LEDBatteryLevel > 4) // min(data * 10 + 5, 100);
				PrimaryGamepad.LEDBatteryLevel = 4;
		} else if (PrimaryGamepad.ControllerType == SONY_DUALSHOCK4) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(PrimaryGamepad.HidHandle, buf, 64);
			if (PrimaryGamepad.USBConnection)
				PrimaryGamepad.BatteryLevel = (buf[30] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS4_USB_BATTERY_MAX;
			else
				PrimaryGamepad.BatteryLevel = (buf[32] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
		} else if (PrimaryGamepad.ControllerType == NINTENDO_JOYCONS || PrimaryGamepad.ControllerType == NINTENDO_SWITCH_PRO) {
			unsigned char buf[64];
			memset(buf, 0, sizeof(buf));
			hid_read(PrimaryGamepad.HidHandle, buf, 64);
			PrimaryGamepad.BatteryLevel = ((buf[2] >> 4) & 0x0F) * 100 / 8;
			
			if (PrimaryGamepad.HidHandle2 != NULL) {
				memset(buf, 0, sizeof(buf));
				hid_read(PrimaryGamepad.HidHandle2, buf, 64);
				PrimaryGamepad.BatteryLevel2 = ((buf[2] >> 4) & 0x0F) * 100 / 8;
			}
		}
		if (PrimaryGamepad.BatteryLevel > 100) PrimaryGamepad.BatteryLevel = 100; // It looks like something is not right, once it gave out 125%
	}	
}

void ExternalPedalsDInputSearch() {
	ExternalPedalsConnected = false;
	for (int JoyID = 0; JoyID < 4; ++JoyID) { // JOYSTICKID4 - 3
		if (joyGetPosEx(JoyID, &AppStatus.ExternalPedalsJoyInfo) == JOYERR_NOERROR && // JoyID - JOYSTICKID1..4
			joyGetDevCaps(JoyID, &AppStatus.ExternalPedalsJoyCaps, sizeof(AppStatus.ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
			(AppStatus.ExternalPedalsJoyCaps.wMid != 1406 ||
			(AppStatus.ExternalPedalsJoyCaps.wPid != 8198 && AppStatus.ExternalPedalsJoyCaps.wPid != 8199)) && // Exclude Pro Controller и JoyCon
			AppStatus.ExternalPedalsJoyCaps.wNumButtons == 16) { // DualSense - 15, DigiJoy - 16
			AppStatus.ExternalPedalsJoyIndex = JoyID;
			AppStatus.ExternalPedalsDInputConnected = true;
			break;
		}
	}
}

void ExternalPedalsArduinoRead()
{
	DWORD bytesRead;

	while (AppStatus.ExternalPedalsArduinoConnected) {
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
	// PrimaryGamepad
	int gamepadID = (int)(intptr_t)UserData;
	if (gamepadID == 1) {
		GamepadOutState.LargeMotor = LargeMotor;
		GamepadOutState.SmallMotor = SmallMotor;
		GamepadSetState(GamepadOutState);
	
	// SecondaryGamepad
	} else if (gamepadID == 2 && SecondaryGamepad.DeviceIndex != 0) { // SecondaryGamepad.DeviceIndex != 0 - Don't send to first gamepad in case of mixed index gamepads
		if (SecondaryGamepad.DeviceIndex != -1)
			JslSetRumble(SecondaryGamepad.DeviceIndex, SmallMotor, LargeMotor);
		if (SecondaryGamepad.DeviceIndex2 != -1)
			JslSetRumble(SecondaryGamepad.DeviceIndex2, SmallMotor, LargeMotor);
	}
	m.unlock();
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

void KMStickMode(float StickX, float StickY, int Mode) {
	if (Mode == WASDStickMode) {
		KeyPress('W', StickY > PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Up);
		KeyPress('S', StickY < -PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Down);
		KeyPress('A', StickX < -PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Left);
		KeyPress('D', StickX > PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Right);
	} else if (Mode == ArrowsStickMode) {
		KeyPress(VK_UP, StickY > PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Up);
		KeyPress(VK_DOWN, StickY < -PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Down);
		KeyPress(VK_RIGHT, StickX > PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Right);
		KeyPress(VK_LEFT, StickX < -PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Left);
	} else if (Mode == MouseLookStickMode)
		MouseMove(StickX * PrimaryGamepad.KMEmu.JoySensX, -StickY * PrimaryGamepad.KMEmu.JoySensY);
	else if (Mode == MouseWheelStickMode)
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, StickY * 50, 0);
	else if (Mode == NumpadsStickMode) {
		KeyPress(VK_NUMPAD8, StickY > PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Up);
		KeyPress(VK_NUMPAD2, StickY < -PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Down);
		KeyPress(VK_NUMPAD4, StickX < -PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Left);
		KeyPress(VK_NUMPAD6, StickX > PrimaryGamepad.KMEmu.StickValuePressKey, &ButtonsStates.Right);
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

	ButtonsStates.DPADUpLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-UP-LEFT", "NONE"));
	ButtonsStates.DPADUpRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-UP-RIGHT", "NONE"));
	ButtonsStates.DPADDownLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-DOWN-LEFT", "NONE"));
	ButtonsStates.DPADDownRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "DPAD-DOWN-RIGHT", "NONE"));
	ButtonsStates.DPADAdvancedMode = !(ButtonsStates.DPADUpLeft.KeyCode == 0 && ButtonsStates.DPADUpRight.KeyCode == 0 && ButtonsStates.DPADDownLeft.KeyCode == 0 && ButtonsStates.DPADDownRight.KeyCode == 0);

	ButtonsStates.Y.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "TRIANGLE-Y", "E"));
	ButtonsStates.X.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "SQUARE-X", "R"));
	ButtonsStates.A.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "CROSS-A", "SPACE"));
	ButtonsStates.B.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "CIRCLE-B", "CTRL"));

	ButtonsStates.LeftStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "LEFT-STICK-CLICK", "NONE"));
	ButtonsStates.RightStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "RIGHT-STICK-CLICK", "NONE"));

	PrimaryGamepad.KMEmu.LeftStickMode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "LEFT-STICK", "WASD"));
	PrimaryGamepad.KMEmu.RightStickMode = KeyNameToKeyCode(IniFile.ReadString("GAMEPAD", "RIGHT-STICK", "MOUSE-LOOK"));

	PrimaryGamepad.KMEmu.JoySensX = IniFile.ReadFloat("MOUSE", "SensitivityX", 100) * 0.21f; // Crysis 2, Last of Us 2 calibration
	PrimaryGamepad.KMEmu.JoySensY = IniFile.ReadFloat("MOUSE", "SensitivityY", 100) * 0.21f; // Crysis 2, Last of Us 2 calibration
}

void DefaultMainText() {
	if (AppStatus.ControllerCount < 1)
		printf("\n Connect DualSense, DualShock 4, Pro controller or Joycons and reset.");
	else {
		printf("\n Connected controllers: ");
		switch (PrimaryGamepad.ControllerType) {
			case SONY_DUALSENSE:
				printf("Sony DualSense (all functions)");
				break;
			case SONY_DUALSHOCK4:
				printf("Sony DualShock 4 (all functions)");
				break;
			case NINTENDO_JOYCONS:
				printf("Nintendo Joy-Cons (");
				if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf("left & right");
				else if (PrimaryGamepad.HidHandle != NULL) printf("left - not enough");
				else if (PrimaryGamepad.HidHandle2 != NULL) printf("right - not enough");
				printf(") (all functions)");
				break;
			case NINTENDO_SWITCH_PRO:
				printf("Nintendo Switch Pro (all functions)");
				break;
		}
		if (SecondaryGamepad.DeviceIndex != -1) {
			printf(", ");
			switch (SecondaryGamepad.JSMControllerType) {
			case JS_TYPE_DS:
				printf("Sony DualSense (simplified)");
				break;
			case JS_TYPE_DS4:
				printf("Sony DualShock 4 (simplified)");
				break;
			case JS_TYPE_JOYCON_LEFT:
				printf("Nintendo Joy-Cons (");
				if (SecondaryGamepad.DeviceIndex2 != -1) printf("left & right");
				else printf("left - not enough");
				printf(") (simplified)");
				break;
			case JS_TYPE_PRO_CONTROLLER:
				printf("Nintendo Switch Pro Controller (simplified)");
				break;
			}

		}
		printf(".");
	}
	printf("\n Press \"CTRL + R\" or \"%s\" to reset or search for controllers.\n", AppStatus.HotKeys.ResetKeyName.c_str());
	if (AppStatus.ControllerCount > 0 && AppStatus.ShowBatteryStatus) {
		printf(" Connection type:");
		if (PrimaryGamepad.USBConnection) printf(" wired"); else printf(" wireless");
		if (PrimaryGamepad.ControllerType != NINTENDO_JOYCONS)
			printf(", battery charge: %d\%%.", PrimaryGamepad.BatteryLevel);
		else {
			if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf(", battery charge: %d\%%, %d\%%.", PrimaryGamepad.BatteryLevel, PrimaryGamepad.BatteryLevel2);
			else if (PrimaryGamepad.HidHandle != NULL) printf(", battery charge: %d\%%.", PrimaryGamepad.BatteryLevel);
			else if (PrimaryGamepad.HidHandle2 != NULL) printf(", battery charge: %d\%%.", PrimaryGamepad.BatteryLevel2);
		}

		if (PrimaryGamepad.BatteryMode == 0x2)
			printf(" (charging)", PrimaryGamepad.BatteryLevel);
		printf("\n");
	}

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled)
		printf(" Emulation: Xbox gamepad.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)
		printf(" Emulation: Xbox gamepad (only driving) & mouse aiming.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled)
		printf(" Emulation: Only mouse (for mouse aiming).\n");
	else if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse)
		printf_s(" Emulation: Keyboard and mouse (%s).\n Change profiles with \"ALT + Up/Down\" or \"PS/Home + DPAD Up/Down\".\n", KMProfiles[ProfileIndex].c_str());
	printf(" Press \"ALT + Q/Left/Right\", \"PS/Home + DPAD Left/Right\" to switch emulation.\n");
	printf(" Press touchpad areas or \"Capture/Home\" buttons to change operating modes.\n");
	printf(" If there's no touch panel, switch using a touchpad press (enabled in the config) or use \"ALT + 1/2\".\n");
	printf(" Pressing \"Home\" or \"ALT + 2\" again - switches aim mode (always/L2), \"Capture\" - resets.\n");

	if (AppStatus.ExternalPedalsDInputConnected) {
		printf(" External DInput pedals are connected. Mode: ");
		if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing)
			printf("always pedals.");
		else
			printf("dependent (driving/aiming).");
		printf(" Press \"ALT + E\" to switch.");
	}
	if (AppStatus.ExternalPedalsArduinoConnected)
		printf(" External pedals Arduino connected.\n");

	if (AppStatus.AimMode == AimMouseMode) printf("\n Aiming mode = Mouse"); else printf("\n Aiming mode = Mouse-Joystick");
	printf(", press \"ALT + A\" or \"PS/Capture + R1\" to switch.\n");

	printf(" Rumble strength is %d%%, press \"ALT + </>\", \"PS + Options\", or \"Capture + Plus\" to adjust.\n", PrimaryGamepad.RumbleStrength);

	printf(" %s touchpad press for mode switching - \"ALT + W\" or \"PS + Share\" (Sony only).\n", AppStatus.ChangeModesWithClick ? "Disable" : "Enable");

	if (AppStatus.LeftStickMode == LeftStickDefaultMode)
		printf(" Left stick mode: Default");
	else if (AppStatus.LeftStickMode == LeftStickAutoPressMode)
		printf(" Left stick mode: Auto pressing by value");
	else if (AppStatus.LeftStickMode == LeftStickInvertPressMode)
		printf(" Left stick mode: Invert pressed");
	printf(", press \"ALT + S\" or \"PS/Home + LS\" to switch.\n");

	if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode)
		printf(" Screenshot mode: Custom key (%s)", &AppStatus.MicCustomKeyName);
	else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode)
		printf(" Screenshot mode: Xbox Game Bar");
	else if (AppStatus.ScreenshotMode == ScreenShotSteamMode)
		printf(" Screenshot mode: Steam (%s)", &AppStatus.SteamScrKeyName);
	else if (AppStatus.ScreenshotMode == ScreenShotMultiMode)
		printf(" Screenshot mode: Xbox Game Bar & Steam (F12)");
	printf(", press \"ALT + X\" to switch.\n");

	printf("\n Press \"PS\" or \"Capture + Home\" to open Xbox Game Bar.\n");
	printf(" Press \"PS/Capture + X\" or microphone button (Sony DualSense) for a screenshot, or hold to record video.\n");
	printf(" Press \"PS + /\\\" or \"Capture + X\" to adjust aiming sensitivity. Press \"PS/Capture + RS\" to reset.\n");
	printf(" Press \"PS + []/O\" or \"Capture + Y/A\" to control Windows volume.\n");

	printf("\n Press \"ALT + F9\" to view stick and trigger dead zones.\n");
	printf(" Press \"ALT + I\" or the center of the touchpad (Sony only) to view battery status.\n");
	printf(" Press \"ALT + B\" or \"PS + L1\" to toggle backlight (Sony only).\n");
	printf(" Press \"ALT + Escape\" to exit.\n");
}

void RussianMainText() {
	if (AppStatus.ControllerCount < 1)
		printf("\n Подключите DualSense, DualShock 4, Pro контроллер или джойконы и сделайте сброс.");
	else {
		printf("\n Подключенные контроллеры: ");
		switch (PrimaryGamepad.ControllerType) {
			case SONY_DUALSENSE:
				printf("Sony DualSense (все функции)");
				break;
			case SONY_DUALSHOCK4:
				printf("Sony DualShock 4 (все функции)");
				break;
			case NINTENDO_JOYCONS:
				printf("Nintendo Joy-Cons (");
				if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf("левый и правый");
				else if (PrimaryGamepad.HidHandle != NULL) printf("левый - недостаточно");
				else if (PrimaryGamepad.HidHandle2 != NULL) printf("правый - недостаточно");
				printf(") (все функции)");
				break;
			case NINTENDO_SWITCH_PRO:
				printf("Nintendo Switch Pro Controller (все функции)");
				break;
		}
		if (SecondaryGamepad.DeviceIndex != -1) {
			printf(", ");
			switch (SecondaryGamepad.JSMControllerType) {
				case JS_TYPE_DS:
					printf("Sony DualSense (упрощённый)");
					break;
				case JS_TYPE_DS4:
					printf("Sony DualShock 4 (упрощённый)");
					break;
				case JS_TYPE_JOYCON_LEFT:
					printf("Nintendo Joy-Cons (");
					if (SecondaryGamepad.DeviceIndex2 != -1) printf("левый и правый");
					else printf("левый - недостаточно");
					printf(") (упрощённый)");
					break;
				case JS_TYPE_PRO_CONTROLLER:
					printf("Nintendo Switch Pro Controller (упрощённый)");
					break;
				}
		
		}
		printf(".");
	}
	printf("\n Нажмите \"CTRL + R\" или \"%s\" для сброса или поиска контроллеров.\n", AppStatus.HotKeys.ResetKeyName.c_str());
	if (AppStatus.ControllerCount > 0 && AppStatus.ShowBatteryStatus) {
		printf(" Тип подключения:");
		if (PrimaryGamepad.USBConnection) printf(" проводной"); else printf(" беспроводной");
		if (PrimaryGamepad.ControllerType != NINTENDO_JOYCONS)
			printf(", заряд батареи: %d\%%.", PrimaryGamepad.BatteryLevel);
		else {
			if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf(", заряд батареи: %d\%%, %d\%%.", PrimaryGamepad.BatteryLevel, PrimaryGamepad.BatteryLevel2);
			else if (PrimaryGamepad.HidHandle != NULL) printf(", заряд батареи: %d\%%.", PrimaryGamepad.BatteryLevel);
			else if (PrimaryGamepad.HidHandle2 != NULL) printf(", заряд батареи: %d\%%.", PrimaryGamepad.BatteryLevel2);
		}

		if (PrimaryGamepad.BatteryMode == 0x2)
			printf(" (зарядка)");
		printf("\n");
	}

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled)
		printf(" Эмуляция: Xbox геймпад.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)
		printf(" Эмуляция: Xbox геймпад (только вождение) и прицеливание мышкой.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled)
		printf(" Эмуляция: только мышь (для прицеливанию мышкой).\n");
	else if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse)
		printf_s(" Эмуляция: клавиатура и мышь (%s).\n Измените профиль, с помощью \"ALT + Up/Down\" или \"PS/Home + DPAD Up/Down\".\n", KMProfiles[ProfileIndex].c_str());
	printf(" Нажмите \"ALT + Q/Влево/Вправо\" или \"PS/Home + DPAD Влево/Вправо\" для переключения режима эмуляции.\n");
	printf(" Нажмите области на сенсорной панели или кнопки \"Capture/Home\" для переключения режимов работы.\n");
	printf(" Если сенсорной панели нет, переключайтесь нажатием тачпада (включив в конфиге) или на \"ALT + 1/2\".\n");
	printf(" Повторное нажатие \"Home\" или \"ALT + 2\" - переключает режим прицеливания (всегда/L2), \"Capture\" - сброс.\n");

	if (AppStatus.ExternalPedalsDInputConnected) {
		printf(" Подключены внешние DInput-педали. Режим: ");
		if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing)
			printf("всегда педали.");
		else
			printf("зависимый (вождение/прицеливание).");
		printf(" Переключение на \"ALT + E\".");
	}
	if (AppStatus.ExternalPedalsArduinoConnected)
		printf(" Подключены внешние Arduino-педали.\n");

	if (AppStatus.AimMode == AimMouseMode) printf("\n Режим прицеливания: мышь"); else printf("\n Режим прицеливания: джойстик-мышь");
	printf(", нажмите \"ALT + A\" или \"PS/Capture + R1\" для переключения.\n");

	printf(" Сила вибрации %d%%, нажмите \"ALT + </>\", \"PS + Options\" или \"Capture + Плюс\" для изменения.\n", PrimaryGamepad.RumbleStrength);

	printf(" %s нажатие тачпада для переключения режимов - \"ALT + W\" или \"PS + Share\" (только Sony).\n", AppStatus.ChangeModesWithClick ? "Выключить" : "Включить");

	if (AppStatus.LeftStickMode == LeftStickDefaultMode)
		printf(" Режим левого стика: по умолчанию");
	else if (AppStatus.LeftStickMode == LeftStickAutoPressMode)
		printf(" Режим левого стика: автонажатие по значению");
	else if (AppStatus.LeftStickMode == LeftStickInvertPressMode)
		printf(" Режим левого стика: инверсия нажатия");
	printf(", нажмите \"ALT + S\" или \"PS/Home + L1\" для переключения.\n");

	if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode)
		printf(" Режим скриншота: своя кнопка (%s)", &AppStatus.MicCustomKeyName);
	else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode)
		printf(" Режим скриншота: Игровая панель Xbox");
	else if (AppStatus.ScreenshotMode == ScreenShotSteamMode)
		printf(" Режим скриншота: Steam (%s)", &AppStatus.SteamScrKeyName);
	else if (AppStatus.ScreenshotMode == ScreenShotMultiMode)
		printf(" Режим скриншота: Игровая панель Xbox и Steam (F12)");
	printf(", нажмите \"ALT + X\" для переключения.\n");

	printf("\n Нажмите \"PS\" или \"Capture + Home\" для открытия Xbox Game Bar.\n");
	printf(" Нажмите \"PS/Capture + X\" или кнопку микрофона (Sony DualSense) для скриншота, удерживайте для записи видео.\n");
	printf(" Нажмите \"PS + /\\\" или \"Capture + X\" для настройки чувствительности прицела. Нажмите \"PS/Capture + RS\" для сброса.\n");
	printf(" Нажмите \"PS + []/O\" или \"Capture + Y/A\" для управления громкостью Windows.\n");

	printf("\n Нажмите \"ALT + F9\" для просмотра мёртвых зон стиков и триггеров.\n");
	printf(" Нажмите \"ALT + I\" или центр тачпада (только Sony) для просмотра заряда батареи.\n");
	printf(" Нажмите \"ALT + B\" или \"PS + L1\" для включения или выключения световой панели (только Sony).\n");
	printf(" Нажмите \"ALT + Escape\" для выхода.\n");
	// printf("%d %d\n", PrimaryGamepad.DeviceIndex, SecondaryGamepad.DeviceIndex);
}

void MainTextUpdate() {
	system("cls");
	if (AppStatus.Lang == LANG_RUSSIAN)
		RussianMainText();
	else
		DefaultMainText();
	//system("cls"); DefaultMainText();
}

void DevicesRefresh() {
	AppStatus.ControllerCount = JslConnectDevices();
	JslGetConnectedDeviceHandles(GamepadsID, AppStatus.ControllerCount);
	if (AppStatus.ControllerCount > 0) {
		//JslSetGyroSpace(GamepadsID[PrimaryGamepad.DeviceIndex], 2);
		JslSetAutomaticCalibration(GamepadsID[PrimaryGamepad.DeviceIndex], true);
	}

	GamepadSearch();
	GamepadSetState(GamepadOutState);

	PrimaryGamepad.DeviceIndex = -1;
	PrimaryGamepad.DeviceIndex2 = -1;
	SecondaryGamepad.JSMControllerType = -1;
	SecondaryGamepad.DeviceIndex = -1;
	SecondaryGamepad.DeviceIndex2 = -1;
	SecondaryGamepad.JSMControllerType = -1;
	int PrimaryControllerType = 0;
	switch (PrimaryGamepad.ControllerType) {
		case SONY_DUALSENSE: PrimaryControllerType = JS_TYPE_DS; break;
		case SONY_DUALSHOCK4: PrimaryControllerType = JS_TYPE_DS4; break;
		case NINTENDO_JOYCONS: PrimaryControllerType = JS_TYPE_JOYCON_LEFT; break;
		case NINTENDO_SWITCH_PRO: PrimaryControllerType = JS_TYPE_PRO_CONTROLLER; break;
	}

	for (int i = 0; i < AppStatus.ControllerCount; i++) {
		int ControllerType = JslGetControllerType(GamepadsID[i]);
		if (ControllerType == JS_TYPE_DS || ControllerType == JS_TYPE_DS4 || ControllerType == JS_TYPE_JOYCON_LEFT || ControllerType == JS_TYPE_PRO_CONTROLLER) {
			if (PrimaryGamepad.DeviceIndex == -1) {
				PrimaryGamepad.DeviceIndex = i;
				PrimaryGamepad.JSMControllerType = ControllerType;

			} else if (SecondaryGamepad.DeviceIndex == -1) {
				SecondaryGamepad.DeviceIndex = i;
				SecondaryGamepad.JSMControllerType = ControllerType; // To display in the list of controllers
			}
		}
	}

	// Glue right joycon
	for (int i = 0; i < AppStatus.ControllerCount; i++) {
		int ControllerType = JslGetControllerType(GamepadsID[i]);
		if (ControllerType != JS_TYPE_JOYCON_RIGHT) continue;
		if (PrimaryGamepad.JSMControllerType == JS_TYPE_JOYCON_LEFT)
			PrimaryGamepad.DeviceIndex2 = i;
		else
			SecondaryGamepad.DeviceIndex2 = i;
	}

	if (AppStatus.ControllerCount > 1 && PrimaryControllerType != 0 && PrimaryGamepad.JSMControllerType != PrimaryControllerType) {
		std::swap(PrimaryGamepad.DeviceIndex, SecondaryGamepad.DeviceIndex);
		std::swap(PrimaryGamepad.DeviceIndex2, SecondaryGamepad.DeviceIndex2);
		std::swap(PrimaryGamepad.JSMControllerType, SecondaryGamepad.JSMControllerType);
	}

	if (SecondaryGamepad.DeviceIndex != -1) {
		JslSetAutomaticCalibration(SecondaryGamepad.DeviceIndex, true);
		if (SecondaryGamepad.DeviceIndex2 != -1)
			JslSetAutomaticCalibration(SecondaryGamepad.DeviceIndex2, true);
		SecondaryGamepad.LEDRed = std::clamp(int(((SecondaryGamepad.LEDColor >> 16) & 0xFF) - SecondaryGamepad.LEDBrightness), 0, 255);
		SecondaryGamepad.LEDGreen = std::clamp(int(((SecondaryGamepad.LEDColor >> 8) & 0xFF) - SecondaryGamepad.LEDBrightness), 0, 255);
		SecondaryGamepad.LEDBlue = std::clamp(int((SecondaryGamepad.LEDColor & 0xFF) - SecondaryGamepad.LEDBrightness), 0, 255);
		JslSetLightColour(SecondaryGamepad.DeviceIndex, (SecondaryGamepad.LEDRed << 16) + (SecondaryGamepad.LEDGreen << 8) + SecondaryGamepad.LEDBlue);
	}

	AppStatus.Gamepad.BTReset = false;
	if (AppStatus.ExternalPedalsDInputSearch) ExternalPedalsDInputSearch();
	MainTextUpdate();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DEVICECHANGE: // The list of devices has changed
			if (wParam == DBT_DEVNODES_CHANGED)
				DevicesRefresh();
			break;
		/*case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;*/
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char **argv)
{
	SetConsoleTitle("DSAdvance 1.3");
	WindowToCenter();

	// if (false)
	if (PRIMARYLANGID(GetUserDefaultLangID()) == LANG_RUSSIAN) {
		AppStatus.Lang = LANG_RUSSIAN;
		setlocale(LC_ALL, ""); // Output locale
		setlocale(LC_NUMERIC, "C"); // Numbers with a dot
		system("chcp 65001 > nul"); // Console UTF8 output 
	}

	WNDCLASS AppWndClass = {};
	AppWndClass.lpfnWndProc = WindowProc;
	AppWndClass.hInstance = GetModuleHandle(NULL);
	AppWndClass.lpszClassName = "DSAdvanceApp";
	RegisterClass(&AppWndClass);
	HWND AppWindow = CreateWindowEx(0, AppWndClass.lpszClassName, "DSAdvanceApp", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
	MSG WindowMsgs = {};

	// Config parameters
	CIniReader IniFile("Config.ini");
	PrimaryGamepad.Sticks.InvertLeftX = IniFile.ReadBoolean("Gamepad", "InvertLeftStickX", false);
	PrimaryGamepad.Sticks.InvertLeftY = IniFile.ReadBoolean("Gamepad", "InvertLeftStickY", false);
	PrimaryGamepad.Sticks.InvertRightX = IniFile.ReadBoolean("Gamepad", "InvertRightStickX", false);
	PrimaryGamepad.Sticks.InvertRightY = IniFile.ReadBoolean("Gamepad", "InvertRightStickY", false);
	AppStatus.HotKeys.ResetKeyName = IniFile.ReadString("Gamepad", "ResetKey", "NONE");
	AppStatus.HotKeys.ResetKey = KeyNameToKeyCode(AppStatus.HotKeys.ResetKeyName);
	AppStatus.ShowBatteryStatusOnLightBar = IniFile.ReadBoolean("Gamepad", "ShowBatteryStatusOnLightBar", true);
	AppStatus.SleepTimeOut = IniFile.ReadInteger("Gamepad", "SleepTimeOut", 1);
	
	PrimaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickX", 0);
	PrimaryGamepad.Sticks.DeadZoneLeftY = IniFile.ReadFloat("Gamepad", "DeadZoneLeftStickY", 0);
	PrimaryGamepad.Sticks.DeadZoneRightX = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickX", 0);
	PrimaryGamepad.Sticks.DeadZoneRightY = IniFile.ReadFloat("Gamepad", "DeadZoneRightStickY", 0);
	PrimaryGamepad.Triggers.DeadZoneLeft = IniFile.ReadFloat("Gamepad", "DeadZoneLeftTrigger", 0);
	PrimaryGamepad.Triggers.DeadZoneRight = IniFile.ReadFloat("Gamepad", "DeadZoneRightTrigger", 0);

	PrimaryGamepad.TouchSticksOn = IniFile.ReadBoolean("Gamepad", "TouchSticksOn", false);
	PrimaryGamepad.TouchSticks.LeftX = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensX", 5.0f);
	PrimaryGamepad.TouchSticks.LeftY = IniFile.ReadFloat("Gamepad", "TouchLeftStickSensY", 5.0f);
	PrimaryGamepad.TouchSticks.RightX = IniFile.ReadFloat("Gamepad", "TouchRightStickSensX", 1.0f);
	PrimaryGamepad.TouchSticks.RightY = IniFile.ReadFloat("Gamepad", "TouchRightStickSensY", 1.0f);

	PrimaryGamepad.AutoPressStickValue = IniFile.ReadFloat("Gamepad", "AutoPressStickValue", 99) * 0.01f;

	PrimaryGamepad.DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("Gamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	PrimaryGamepad.RumbleStrength = IniFile.ReadInteger("Gamepad", "RumbleStrength", 100);
	AppStatus.LockedChangeBrightness = IniFile.ReadBoolean("Gamepad", "LockChangeBrightness", false);
	AppStatus.ChangeModesWithClick = IniFile.ReadBoolean("Gamepad", "ChangeModesWithClick", true);
	AppStatus.ChangeModesWithoutAreas = IniFile.ReadBoolean("Gamepad", "ChangeModesWithoutAreas", false);

	AppStatus.AimMode = IniFile.ReadBoolean("Motion", "AimMode", AimMouseMode);
	AppStatus.AimingWithL2 = IniFile.ReadBoolean("Motion", "AimingWithL2", true);

	PrimaryGamepad.Motion.WheelAngle = IniFile.ReadFloat("Motion", "WheelAngle", 150) / 2.0f;
	PrimaryGamepad.Motion.WheelPitch = IniFile.ReadBoolean("Motion", "WheelPitch", false);
	PrimaryGamepad.Motion.WheelRoll = IniFile.ReadBoolean("Motion", "WheelRoll", true);
	PrimaryGamepad.Motion.WheelInvertPitch = IniFile.ReadBoolean("Motion", "WheelInvertPitch", false) ? 1 : -1;
	PrimaryGamepad.Motion.SensX = IniFile.ReadFloat("Motion", "MouseSensX", 100) * 0.005f;   // Calibration with Crysis 2, old 0.01
	PrimaryGamepad.Motion.SensY = IniFile.ReadFloat("Motion", "MouseSensY", 90) * 0.005f;
	PrimaryGamepad.Motion.SensAvg = (PrimaryGamepad.Motion.SensX + PrimaryGamepad.Motion.SensY) * 0.5f; // Sens Average
	PrimaryGamepad.Motion.JoySensX = IniFile.ReadFloat("Motion", "JoySensX", 100) * 0.0025f; // Calibration with Crysis 2, old 0.0013;
	PrimaryGamepad.Motion.JoySensY = IniFile.ReadFloat("Motion", "JoySensY", 90) * 0.0025f;  // Calibration with Crysis 2, old 0.0013;
	PrimaryGamepad.Motion.JoySensAvg = (PrimaryGamepad.Motion.JoySensX + PrimaryGamepad.Motion.JoySensY) * 0.5f; // Sens Average

	PrimaryGamepad.DefaultModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DefaultModeColor", "0000ff"));
	PrimaryGamepad.DrivingModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DrivingModeColor", "ff0000"));
	PrimaryGamepad.AimingModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "AimingModeColor", "00ff00"));
	PrimaryGamepad.AimingModeL2Color = WebColorToRGB(IniFile.ReadString("Gamepad", "AimingModeL2Color", "00ffff"));
	PrimaryGamepad.DesktopModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DesktopModeColor", "ff00ff"));
	PrimaryGamepad.TouchSticksModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "TouchSticksModeColor", "ff00ff"));

	PrimaryGamepad.KMEmu.StickValuePressKey = IniFile.ReadFloat("KeyboardMouse", "StickValuePressKey", 0.2f);
	PrimaryGamepad.KMEmu.TriggerValuePressKey = IniFile.ReadFloat("KeyboardMouse", "TriggerValuePressKey", 0.2f);

	bool SecondaryGamepadEnabled = IniFile.ReadBoolean("SecondaryGamepad", "Enabled", false);
	SecondaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickX", 0);
	SecondaryGamepad.Sticks.DeadZoneLeftY = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickY", 0);
	SecondaryGamepad.Sticks.DeadZoneRightX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightStickX", 0);
	SecondaryGamepad.Sticks.DeadZoneRightY = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightStickY", 0);
	SecondaryGamepad.Triggers.DeadZoneLeft = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftTrigger", 0);
	SecondaryGamepad.Triggers.DeadZoneRight = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightTrigger", 0);
	SecondaryGamepad.DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("SecondaryGamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	SecondaryGamepad.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
	SecondaryGamepad.DefaultModeColor = WebColorToRGB(IniFile.ReadString("SecondaryGamepad", "DefaultModeColor", "00ff00"));
	SecondaryGamepad.LEDColor = SecondaryGamepad.DefaultModeColor;

	AppStatus.MicCustomKeyName = IniFile.ReadString("Gamepad", "MicCustomKey", "NONE");
	AppStatus.MicCustomKey = KeyNameToKeyCode(AppStatus.MicCustomKeyName);
	if (AppStatus.MicCustomKey == 0)
		AppStatus.ScreenshotMode = ScreenShotXboxGameBarMode; // If not set, then hide this mode
	else
		AppStatus.ScreenShotKey = AppStatus.MicCustomKey;

	AppStatus.SteamScrKeyName = IniFile.ReadString("Gamepad", "SteamScrKey", "NONE");
	AppStatus.SteamScrKey = KeyNameToKeyCode(AppStatus.SteamScrKeyName);

	// External pedals
	AppStatus.ExternalPedalsDInputSearch = IniFile.ReadBoolean("ExternalPedals", "DInput", false);
	AppStatus.ExternalPedalsCOMPort = IniFile.ReadInteger("ExternalPedals", "COMPort", 0);
	AppStatus.ExternalPedalsMode = IniFile.ReadInteger("ExternalPedals", "DefaultMode", 0);
	AppStatus.ExternalPedalsXboxModePedal1 = XboxKeyNameToXboxKeyCode(IniFile.ReadString("ExternalPedals", "AimingPedal1", "NONE"));
	AppStatus.ExternalPedalsXboxModePedal1Analog = (AppStatus.ExternalPedalsXboxModePedal1 == XINPUT_GAMEPAD_LEFT_TRIGGER) || (AppStatus.ExternalPedalsXboxModePedal1 == XINPUT_GAMEPAD_RIGHT_TRIGGER);
	AppStatus.ExternalPedalsXboxModePedal2 = XboxKeyNameToXboxKeyCode(IniFile.ReadString("ExternalPedals", "AimingPedal2", "NONE"));
	AppStatus.ExternalPedalsXboxModePedal2Analog = (AppStatus.ExternalPedalsXboxModePedal2 == XINPUT_GAMEPAD_LEFT_TRIGGER) || (AppStatus.ExternalPedalsXboxModePedal2 == XINPUT_GAMEPAD_RIGHT_TRIGGER);
	AppStatus.ExternalPedalsValuePress = 65536 * ClampFloat(IniFile.ReadFloat("ExternalPedals", "PedalValuePress", 20.0f) * 0.01f, 0, 1.0f);
	for (int i = 0; i < 16; ++i) AppStatus.ExternalPedalsButtons[i] = XboxKeyNameToXboxKeyCode(IniFile.ReadString("ExternalPedals", "Button" + std::to_string(i + 1), "NONE"));
	AppStatus.ExternalPedalsJoyInfo.dwFlags = JOY_RETURNALL;
	AppStatus.ExternalPedalsJoyInfo.dwSize = sizeof(AppStatus.ExternalPedalsJoyInfo);

	if (AppStatus.ExternalPedalsDInputSearch) // Dinput in priority
		ExternalPedalsDInputSearch();
	else if (AppStatus.ExternalPedalsCOMPort != 0) {
		char sPortName[32];
		sprintf_s(sPortName, "\\\\.\\COM%d", AppStatus.ExternalPedalsCOMPort);

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
					AppStatus.ExternalPedalsArduinoConnected = true;
					PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
					pArduinoReadThread = new std::thread(ExternalPedalsArduinoRead);
				}
			}
		}
	}

	// Sound for switching profiles
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
	GamepadOutState.LEDColor = PrimaryGamepad.DefaultModeColor;
	GamepadOutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;

	GamepadSetState(GamepadOutState);

	int SkipPollCount = 0;
	EulerAngles MotionAngles, AnglesOffset;

	AppStatus.ControllerCount = JslConnectDevices();
	JslGetConnectedDeviceHandles(GamepadsID, AppStatus.ControllerCount);
	
	if (AppStatus.ControllerCount > 0) {
		//JslSetGyroSpace(GamepadsID[PrimaryGamepad.DeviceIndex], 2);
		JslSetAutomaticCalibration(GamepadsID[PrimaryGamepad.DeviceIndex], true);
		if (AppStatus.ControllerCount > 1)
			JslSetAutomaticCalibration(GamepadsID[1], true);
		if (AppStatus.ControllerCount > 2)
			JslSetAutomaticCalibration(GamepadsID[2], true);
	}

	MainTextUpdate();

	MOTION_STATE MotionState;
	TOUCH_STATE TouchState;

	const auto client = vigem_alloc();
	auto ret = vigem_connect(client);

	const auto x360 = vigem_target_x360_alloc();
	ret = vigem_target_add(client, x360);
	ret = vigem_target_x360_register_notification(client, x360, &notification, (void*)1);
	XUSB_REPORT report;

	const auto client2 = vigem_alloc();
	const auto x3602 = vigem_target_x360_alloc();
	if (SecondaryGamepadEnabled) {
		ret = vigem_connect(client2);
		ret = vigem_target_add(client2, x3602);
		ret = vigem_target_x360_register_notification(client2, x3602, &notification, (void*)2);
	}
	XUSB_REPORT report2;

	float velocityX, velocityY, velocityZ;
	TouchpadTouch FirstTouch, SecondTouch;

	//auto previous_time = std::chrono::high_resolution_clock::now();
	//static DWORD lastTime = GetTickCount();

	while (!(GetAsyncKeyState(VK_LMENU) & 0x8000 && GetAsyncKeyState(VK_ESCAPE) & 0x8000))
	{
		if (PeekMessage(&WindowMsgs, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&WindowMsgs);
			DispatchMessage(&WindowMsgs);
			if (WindowMsgs.message == WM_QUIT) break;
		}

		// Reset
		if ((SkipPollCount == 0 && (IsKeyPressed(VK_CONTROL) && IsKeyPressed('R')) || IsKeyPressed(AppStatus.HotKeys.ResetKey)) || AppStatus.Gamepad.BTReset)
		{
			DevicesRefresh();
			SkipPollCount = SkipPollTimeOut;
		}

		XUSB_REPORT_INIT(&report);
		if (SecondaryGamepadEnabled)
			XUSB_REPORT_INIT(&report2);

		if (AppStatus.ControllerCount < 1) { // We don't process anything during idle time
			report.sThumbLX = 1; // helps with crash, maybe power saving turns off the controller
			ret = vigem_target_x360_update(client, x360, report); // Vigem always mode only
			
			if (SecondaryGamepadEnabled) {
				report2.sThumbLX = 1;
				ret = vigem_target_x360_update(client2, x360, report2);
			}

			Sleep(AppStatus.SleepTimeOut);
			continue;
		}

		// Primary controller
		if (PrimaryGamepad.DeviceIndex2 == -1) {
			PrimaryGamepad.InputState = JslGetSimpleState(GamepadsID[PrimaryGamepad.DeviceIndex]);
			MotionState = JslGetMotionState(GamepadsID[PrimaryGamepad.DeviceIndex]);
			JslGetAndFlushAccumulatedGyro(GamepadsID[PrimaryGamepad.DeviceIndex], velocityX, velocityY, velocityZ);
		} else { // Split contoller (Joycons)
			PrimaryGamepad.InputState = JslGetSimpleState(GamepadsID[PrimaryGamepad.DeviceIndex]);
			JOY_SHOCK_STATE tempState = JslGetSimpleState(GamepadsID[PrimaryGamepad.DeviceIndex2]);
			MotionState = JslGetMotionState(GamepadsID[PrimaryGamepad.DeviceIndex2]);
			PrimaryGamepad.InputState.stickRX = tempState.stickRX;
			PrimaryGamepad.InputState.stickRY = tempState.stickRY;
			PrimaryGamepad.InputState.rTrigger = tempState.rTrigger;
			JslGetAndFlushAccumulatedGyro(GamepadsID[PrimaryGamepad.DeviceIndex2], velocityX, velocityY, velocityZ);
			PrimaryGamepad.InputState.buttons |= tempState.buttons;
		}

		// Secondary controller
		if (SecondaryGamepad.DeviceIndex2 == -1) {
			SecondaryGamepad.InputState = JslGetSimpleState(GamepadsID[SecondaryGamepad.DeviceIndex]);
			//MotionState = JslGetMotionState(GamepadsID[SecondaryGamepad.DeviceIndex]);
			//JslGetAndFlushAccumulatedGyro(GamepadsID[SecondaryGamepad.DeviceIndex], velocityX, velocityY, velocityZ);
		}
		else { // Split contoller (Joycons)
			SecondaryGamepad.InputState = JslGetSimpleState(GamepadsID[SecondaryGamepad.DeviceIndex]);
			JOY_SHOCK_STATE tempState = JslGetSimpleState(GamepadsID[SecondaryGamepad.DeviceIndex2]);
			//MotionState = JslGetMotionState(GamepadsID[SecondaryGamepad.DeviceIndex2]);
			SecondaryGamepad.InputState.stickRX = tempState.stickRX;
			SecondaryGamepad.InputState.stickRY = tempState.stickRY;
			SecondaryGamepad.InputState.rTrigger = tempState.rTrigger;
			SecondaryGamepad.InputState.buttons |= tempState.buttons;
			//JslGetAndFlushAccumulatedGyro(GamepadsID[SecondaryGamepad.DeviceIndex2], velocityX, velocityY, velocityZ);
		}
		
		MotionAngles = QuaternionToEulerAngle(MotionState.quatW, MotionState.quatZ, MotionState.quatX, MotionState.quatY);

		// Stick dead zones
		if (SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed(VK_F9) != 0)
		{
			AppStatus.DeadZoneMode = !AppStatus.DeadZoneMode;
			if (AppStatus.DeadZoneMode == false) MainTextUpdate(); else { system("cls"); printf("\n"); }
			SkipPollCount = SkipPollTimeOut;
		}
		if (AppStatus.DeadZoneMode) {
			if (AppStatus.Lang == LANG_RUSSIAN) {
				printf(" Левый стик X=%6.2f, ", abs(PrimaryGamepad.InputState.stickLX));
				printf("Y=%6.2f\t", abs(PrimaryGamepad.InputState.stickLY));
				printf("Правый стик X=%6.2f ", abs(PrimaryGamepad.InputState.stickRX));
				printf("Y=%6.2f\t", abs(PrimaryGamepad.InputState.stickRY));
				printf("Левый триггер=%6.2f\t", abs(PrimaryGamepad.InputState.lTrigger));
				printf("Правый триггер=%6.2f\n", abs(PrimaryGamepad.InputState.rTrigger));
			} else {
				printf(" Left stick X=%6.2f, ", abs(PrimaryGamepad.InputState.stickLX));
				printf("Y=%6.2f\t", abs(PrimaryGamepad.InputState.stickLY));
				printf("Right stick X=%6.2f ", abs(PrimaryGamepad.InputState.stickRX));
				printf("Y=%6.2f\t", abs(PrimaryGamepad.InputState.stickRY));
				printf("Left trigger=%6.2f\t", abs(PrimaryGamepad.InputState.lTrigger));
				printf("Right trigger=%6.2f\n", abs(PrimaryGamepad.InputState.rTrigger));
			}
		}

		// Switch emulation mode
		if (SkipPollCount == 0 && ( (IsKeyPressed(VK_MENU) && (IsKeyPressed('Q') || IsKeyPressed(VK_LEFT) || IsKeyPressed(VK_RIGHT))) || ((PrimaryGamepad.InputState.buttons & JSMASK_LEFT || PrimaryGamepad.InputState.buttons & JSMASK_RIGHT) && PrimaryGamepad.InputState.buttons & JSMASK_PS) ) ) // Disable Xbox controller emulation for games that support DualSense, DualShock, Nintendo controllers or enable only driving & mouse aiming
		{
			SkipPollCount = 30; // 15 is seems not enough to enable or disable Xbox virtual gamepad

			if (PrimaryGamepad.InputState.buttons & JSMASK_LEFT || IsKeyPressed(VK_LEFT)) {
				if (AppStatus.GamepadEmulationMode == 0) AppStatus.GamepadEmulationMode = EmuGamepadMaxModes; else AppStatus.GamepadEmulationMode--;
			} else
				AppStatus.GamepadEmulationMode++;

			if (AppStatus.GamepadEmulationMode > EmuGamepadMaxModes) AppStatus.GamepadEmulationMode = EmuGamepadEnabled;
			if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled || AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) {
				if (AppStatus.XboxGamepadAttached) {
					//vigem_target_x360_unregister_notification(x360);
					//vigem_target_remove(client, x360);
					AppStatus.XboxGamepadAttached = false;
				}
			} else if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) {
				if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) AppStatus.AimMode = AimMouseMode;
				if (AppStatus.XboxGamepadAttached == false) {
					//ret = vigem_target_add(client, x360);
					//ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);
					AppStatus.XboxGamepadAttached = true;
				}
			}
			
			if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse)
				LoadKMProfile(KMProfiles[ProfileIndex]);

			//if ( ( (LastGamepadEmuMode == EmuGamepadEnabled && AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) || (LastGamepadEmuMode == EmuGamepadOnlyDriving && AppStatus.GamepadEmulationMode == EmuGamepadEnabled) ) ||
				 //( (LastGamepadEmuMode == EmuGamepadDisabled && AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) || (LastGamepadEmuMode == EmuKeyboardAndMouse && AppStatus.GamepadEmulationMode == EmuGamepadDisabled) ) )
				PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);

			MainTextUpdate();
		}

		// Switch aiming mode: mouse / joymouse
		if (SkipPollCount == 0 && ( (IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('A')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_R) || (PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_R)) )
		{
			AppStatus.AimMode = !AppStatus.AimMode;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch modes by pressing or touching
		if (SkipPollCount == 0 && (( IsKeyPressed(VK_MENU) && IsKeyPressed('W') ) || 
			((JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS4) && 
			(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_SHARE))) )
		{
			AppStatus.ChangeModesWithClick = !AppStatus.ChangeModesWithClick;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch left stick mode
		if (SkipPollCount == 0 && ( (IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('S')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_LCLICK))) // PS - Home (Switch)
		{
			AppStatus.LeftStickMode++; if (AppStatus.LeftStickMode > LeftStickMaxModes) AppStatus.LeftStickMode = LeftStickDefaultMode;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch screenshot mode
		if (SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('X'))
		{
			AppStatus.ScreenshotMode++; if (AppStatus.ScreenshotMode > ScreenShotMaxModes) AppStatus.ScreenshotMode = AppStatus.MicCustomKey == 0 ? ScreenShotXboxGameBarMode : ScreenShotCustomKeyMode;
			if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode) AppStatus.ScreenShotKey = AppStatus.MicCustomKey;
			else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode) AppStatus.ScreenShotKey = VK_GAMEBAR_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotSteamMode) AppStatus.ScreenShotKey = VK_STEAM_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotMultiMode) AppStatus.ScreenShotKey = VK_MULTI_SCREENSHOT;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Enable or disable lightbar
		if (SkipPollCount == 0 && ( (IsKeyPressed(VK_MENU) && IsKeyPressed('B')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_L) ))
		{
			if (GamepadOutState.LEDBrightness == 255) GamepadOutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;
			else {
				if (AppStatus.LockedChangeBrightness == false && GamepadOutState.LEDBrightness > 4) // 5 is the minimum brightness
					PrimaryGamepad.DefaultLEDBrightness = GamepadOutState.LEDBrightness; // Save the new selected value as default
				GamepadOutState.LEDBrightness = 255;
			} 
			GamepadSetState(GamepadOutState);
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch keyboard and mouse profile
		if (SkipPollCount == 0 && AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse)
			if ((PrimaryGamepad.InputState.buttons & JSMASK_PS && (PrimaryGamepad.InputState.buttons & JSMASK_UP || PrimaryGamepad.InputState.buttons & JSMASK_DOWN)) ||
				((IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_UP) || IsKeyPressed(VK_DOWN))) && GetConsoleWindow() == GetForegroundWindow()))
			{
				SkipPollCount = SkipPollTimeOut;
				if (IsKeyPressed(VK_UP) || PrimaryGamepad.InputState.buttons & JSMASK_UP) if (ProfileIndex > 0) ProfileIndex--; else ProfileIndex = KMProfiles.size() - 1;
				if (IsKeyPressed(VK_DOWN) || PrimaryGamepad.InputState.buttons & JSMASK_DOWN) if (ProfileIndex < KMProfiles.size() - 1) ProfileIndex++; else ProfileIndex = 0;
				LoadKMProfile(KMProfiles[ProfileIndex]);
				MainTextUpdate();
				PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
			}

		// Switch external pedals mode
		if (SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('E'))))
		{
			if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing)
				AppStatus.ExternalPedalsMode = ExPedalsDependentMode;
			else
				AppStatus.ExternalPedalsMode = ExPedalsAlwaysRacing;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Changing the Rumble strength
		if (SkipPollCount == 0 && IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_OEM_COMMA) || IsKeyPressed(VK_OEM_PERIOD)))
		{
			if (IsKeyPressed(VK_OEM_COMMA) && PrimaryGamepad.RumbleStrength > 0)
				PrimaryGamepad.RumbleStrength -= 10;
			if (IsKeyPressed(VK_OEM_PERIOD) && PrimaryGamepad.RumbleStrength < 100)
				PrimaryGamepad.RumbleStrength += 10;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		bool IsCombinedRumbleChange = false;
		if ((JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS4) && 
			(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS))
			IsCombinedRumbleChange = true;
		if ((JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_RIGHT || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_PRO_CONTROLLER) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_PLUS))
			IsCombinedRumbleChange = true;
		if (SkipPollCount == 0 && IsCombinedRumbleChange) {
			if (PrimaryGamepad.RumbleStrength == 100)
				PrimaryGamepad.RumbleStrength = 0;
			else
				PrimaryGamepad.RumbleStrength += 10;
			MainTextUpdate();
			SkipPollCount = SkipPollTimeOut;
		}

		// Switch modes by touchpad
		if (JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS4)

			// Regular controllers with touchpads
			if (AppStatus.ChangeModesWithoutAreas == false) {

				TouchState = JslGetTouchState(GamepadsID[PrimaryGamepad.DeviceIndex]);

				if (AppStatus.LockChangeBrightness == false && TouchState.t0Down && TouchState.t0Y <= 0.1 && TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA) { // Brightness change
					GamepadOutState.LEDBrightness = 255 - std::clamp((int)((TouchState.t0X - TOUCHPAD_LEFT_AREA - 0.020) * 255 * 4), 0, 255);
					//printf("%5.2f %d\n", (TouchState.t0X - TOUCHPAD_LEFT_AREA - 0.020) * 255 * 4, GamepadOutState.LEDBrightness);
					GamepadSetState(GamepadOutState);
				}

				if ((PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK && AppStatus.ChangeModesWithClick) || (TouchState.t0Down && AppStatus.ChangeModesWithClick == false)) {
				
					// [O--] - Driving mode
					if (TouchState.t0X > 0 && TouchState.t0X <= TOUCHPAD_LEFT_AREA && PrimaryGamepad.GamepadActionMode != TouchpadSticksMode) {
						PrimaryGamepad.GamepadActionMode = MotionDrivingMode;
						AnglesOffset = MotionAngles;
		
						PrimaryGamepad.Motion.OffsetAxisX = atan2f(MotionState.gravX, MotionState.gravZ);
						PrimaryGamepad.Motion.OffsetAxisY = atan2f(MotionState.gravY, MotionState.gravZ);
						
						GamepadOutState.LEDColor = PrimaryGamepad.DrivingModeColor;
					
					// [-O-] // Default & touch sticks modes
					} else if (TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA) {

						// Brightness area
						if (TouchState.t0Y <= 0.1) { 
						
							if (SkipPollCount == 0) { 
								AppStatus.BrightnessAreaPressed++;
								if (AppStatus.BrightnessAreaPressed > 1) {
									if (AppStatus.LockedChangeBrightness) {
										if (GamepadOutState.LEDBrightness == 255) GamepadOutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness; else GamepadOutState.LEDBrightness = 255;
									} else
										AppStatus.LockChangeBrightness = !AppStatus.LockChangeBrightness;
									AppStatus.BrightnessAreaPressed = 0;
								}
								SkipPollCount = SkipPollTimeOut;
							}
							GamepadOutState.LEDColor = PrimaryGamepad.DefaultModeColor;

						// Default mode
						} else if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.7) { 
							PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
							// Show battery level
							GetBatteryInfo(); if (PrimaryGamepad.BackOutStateCounter == 0) PrimaryGamepad.BackOutStateCounter = 40; // It is executed many times, so it is done this way, it is necessary to save the old brightness value for return
							if (AppStatus.ShowBatteryStatusOnLightBar) {
								if (PrimaryGamepad.BackOutStateCounter == 40) PrimaryGamepad.LastLEDBrightness = GamepadOutState.LEDBrightness; // Save on first click (tick)
								if (PrimaryGamepad.BatteryLevel >= 30) // Battery fine 30%-100%
									GamepadOutState.LEDColor = AppStatus.BatteryFineColor;
								else if (PrimaryGamepad.BatteryLevel >= 10) // Battery warning 10..29%
									GamepadOutState.LEDColor = AppStatus.BatteryWarningColor; 
								else // battery critical 10%
									GamepadOutState.LEDColor = AppStatus.BatteryCriticalColor;
								GamepadOutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;
							}
							GamepadOutState.PlayersCount = PrimaryGamepad.LEDBatteryLevel; // JslSetPlayerNumber(GamepadsID[PrimaryGamepad.DeviceIndex], 5);
							AppStatus.ShowBatteryStatus = true;
							MainTextUpdate();
							//printf(" %d %d\n", PrimaryGamepad.LastLEDBrightness, GamepadOutState.LEDBrightness);
					
						// Desktop / Touch sticks mode
						} else {  
							if (PrimaryGamepad.TouchSticksOn) {
								PrimaryGamepad.GamepadActionMode = TouchpadSticksMode;
								GamepadOutState.LEDColor = PrimaryGamepad.TouchSticksModeColor;
						
							} else if (!PrimaryGamepad.SwitchedToDesktopMode && SkipPollCount == 0) {
								PrimaryGamepad.GamepadActionMode = DesktopMode;
								GamepadOutState.LEDColor = PrimaryGamepad.DesktopModeColor;
								if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse)
									AppStatus.LastGamepadEmulationMode = AppStatus.GamepadEmulationMode;
								AppStatus.GamepadEmulationMode = EmuKeyboardAndMouse;
								LoadKMProfile(KMProfiles[0]); // First profile Desktop.ini
								PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
								PrimaryGamepad.SwitchedToDesktopMode = true;
								MainTextUpdate();
								SkipPollCount = SkipPollTimeOut;
								//printf("Desktop turn on\n");
							}
						}

					// [--O] Aiming mode
					} else if (TouchState.t0X > TOUCHPAD_RIGHT_AREA && TouchState.t0X <= 1 && PrimaryGamepad.GamepadActionMode != TouchpadSticksMode) {
					
						// Switch motion aiming mode
						if (SkipPollCount == 0 && TouchState.t0Y < 0.3) { 
							PrimaryGamepad.GamepadActionMode = PrimaryGamepad.GamepadActionMode != MotionAimingMode ? MotionAimingMode : MotionAimingModeOnlyPressed;
							PrimaryGamepad.LastMotionAIMMode = PrimaryGamepad.GamepadActionMode;
							SkipPollCount = SkipPollTimeOut;
						} 

						// Motion aiming
						if (TouchState.t0Y >= 0.3 && TouchState.t0Y <= 1) { 
							PrimaryGamepad.GamepadActionMode = PrimaryGamepad.LastMotionAIMMode;
							GamepadOutState.LEDColor = PrimaryGamepad.AimingModeColor;
						}
					
						GamepadOutState.LEDColor = PrimaryGamepad.GamepadActionMode == MotionAimingMode ? PrimaryGamepad.AimingModeColor : PrimaryGamepad.AimingModeL2Color;
					}

					// Reset desktop mode and return action mode
					if (!PrimaryGamepad.TouchSticksOn && PrimaryGamepad.GamepadActionMode != DesktopMode && PrimaryGamepad.SwitchedToDesktopMode) {
						AppStatus.GamepadEmulationMode = AppStatus.LastGamepadEmulationMode;
						MainTextUpdate();
						PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
						PrimaryGamepad.SwitchedToDesktopMode = false;
						//printf("Desktop turn off\n");
					}

					// Reset lock brightness if clicked in another area
					if (!(TouchState.t0Y <= 0.1 && TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA)) {
						AppStatus.BrightnessAreaPressed = 0; 
						if (AppStatus.LockChangeBrightness == false) AppStatus.LockChangeBrightness = true;
					}

					GamepadSetState(GamepadOutState);
					//printf("current mode = %d\r\n", PrimaryGamepad.GamepadActionMode);
					if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && PrimaryGamepad.GamepadActionMode != MotionDrivingMode) AppStatus.XboxGamepadReset = true; // Reset last state
				}

			// Controllers without touchpads (AppStatus.ChangeModesWithoutAreas == true)
			} else if ((PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) && SkipPollCount == 0) {

				if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {
					
					if (PrimaryGamepad.GamepadActionMode == MotionAimingMode)
					{
						PrimaryGamepad.GamepadActionMode = MotionAimingModeOnlyPressed; PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
					} else { 
						PrimaryGamepad.GamepadActionMode = MotionAimingMode; PrimaryGamepad.LastMotionAIMMode = MotionAimingMode; 
					}

				} else
					PrimaryGamepad.GamepadActionMode = MotionDrivingMode;

				SkipPollCount = SkipPollTimeOut;
			}

		if (SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('I') && GetConsoleWindow() == GetForegroundWindow())
		{
			SkipPollCount = SkipPollTimeOut;
			GetBatteryInfo(); if (PrimaryGamepad.BackOutStateCounter == 0) PrimaryGamepad.BackOutStateCounter = 40; // ↑
			if (PrimaryGamepad.BackOutStateCounter == 40) PrimaryGamepad.LastLEDBrightness = GamepadOutState.LEDBrightness; // Save on first click (tick)
			AppStatus.ShowBatteryStatus = true;
			MainTextUpdate();
		}

		//printf("%5.2f\t%5.2f\r\n", PrimaryGamepad.InputState.stickLX, DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX));
		report.sThumbLX = PrimaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX) * 32767;
		report.sThumbLY = PrimaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY) * 32767;
		report.sThumbRX = PrimaryGamepad.Sticks.InvertRightX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX) * 32767;
		report.sThumbRY = PrimaryGamepad.Sticks.InvertRightY == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY) * 32767;

		// Auto stick pressing when value is exceeded
		if (AppStatus.LeftStickMode == LeftStickAutoPressMode && ( sqrt(PrimaryGamepad.InputState.stickLX * PrimaryGamepad.InputState.stickLX + PrimaryGamepad.InputState.stickLY * PrimaryGamepad.InputState.stickLY) >= PrimaryGamepad.AutoPressStickValue))
			report.wButtons |= JSMASK_LCLICK;

		report.bLeftTrigger = DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) * 255;
		report.bRightTrigger = DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) * 255;
		
		// External pedals
		if (AppStatus.ExternalPedalsDInputConnected) {
			if (joyGetPosEx(AppStatus.ExternalPedalsJoyIndex, &AppStatus.ExternalPedalsJoyInfo) == JOYERR_NOERROR) {

				// Always racing mode - analog triggers
				if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing) {
					if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
						report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
					if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0)
						report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
				
				} else { // ExPedalsModeDependent
					// In motion driving mode - analog triggers
					if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {
						if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
							report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
						if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0)
							report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
					
					} else {
						// Pedal 1
						if (!AppStatus.ExternalPedalsXboxModePedal1Analog) { // Pedal 1 button
							if (AppStatus.ExternalPedalsJoyInfo.dwVpos > AppStatus.ExternalPedalsValuePress) 
								if ((report.wButtons & AppStatus.ExternalPedalsXboxModePedal1) == 0)
									report.wButtons |= AppStatus.ExternalPedalsXboxModePedal1;
						} else {
							if (AppStatus.ExternalPedalsXboxModePedal1 == XINPUT_GAMEPAD_LEFT_TRIGGER) {
								if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
									report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
							} else {
								if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0)
									report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
							}
						}

						// Pedal 2
						if (!AppStatus.ExternalPedalsXboxModePedal2Analog) { // Pedal 2 button
							if (AppStatus.ExternalPedalsJoyInfo.dwUpos > AppStatus.ExternalPedalsValuePress)
								if ((report.wButtons & AppStatus.ExternalPedalsXboxModePedal2) == 0)
									report.wButtons |= AppStatus.ExternalPedalsXboxModePedal2;
						} else {
							if (AppStatus.ExternalPedalsXboxModePedal2 == XINPUT_GAMEPAD_LEFT_TRIGGER) {
								if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
									report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
							} else {
								if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0)
									report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
							}
						}
					}
				}

				// Press buttons
				for (int i = 0; i < 16; ++i)
					if (AppStatus.ExternalPedalsJoyInfo.dwButtons & (1 << (i))) { //JOY_BUTTON1-32
						
						if (AppStatus.ExternalPedalsButtons[i] == XINPUT_GAMEPAD_LEFT_TRIGGER)
							report.bLeftTrigger = 255;
						else if (AppStatus.ExternalPedalsButtons[i] == XINPUT_GAMEPAD_RIGHT_TRIGGER)
							report.bRightTrigger = 255;
						else if ((report.wButtons & AppStatus.ExternalPedalsButtons[i]) == 0)
							report.wButtons |= AppStatus.ExternalPedalsButtons[i];
					}

			} else
				AppStatus.ExternalPedalsDInputConnected = false;

		} else if (AppStatus.ExternalPedalsArduinoConnected) {
			if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
				report.bLeftTrigger = PedalsValues[0] * 255;
			if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0)
				report.bRightTrigger = PedalsValues[1] * 255;
		}

		if (JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_DS4) {
			if (!(PrimaryGamepad.InputState.buttons & JSMASK_PS)) {
				report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
				report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
			}
		} else if (JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_RIGHT) {
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_MINUS ? XINPUT_GAMEPAD_BACK : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_PLUS ? XINPUT_GAMEPAD_START : 0;
		}

		if (!(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE)) { // During special functions, nothing is pressed in the game
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_L ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_R ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0;
			if (AppStatus.LeftStickMode != LeftStickInvertPressMode) // Invert stick mode
				report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_LCLICK ? XINPUT_GAMEPAD_LEFT_THUMB : 0;
			else
				report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_LCLICK ? 0 : XINPUT_GAMEPAD_LEFT_THUMB;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_RCLICK ? XINPUT_GAMEPAD_RIGHT_THUMB : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_UP ? XINPUT_GAMEPAD_DPAD_UP : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_DOWN ? XINPUT_GAMEPAD_DPAD_DOWN : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_LEFT ? XINPUT_GAMEPAD_DPAD_LEFT : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_RIGHT ? XINPUT_GAMEPAD_DPAD_RIGHT : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_N ? XINPUT_GAMEPAD_Y : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_W ? XINPUT_GAMEPAD_X : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_S ? XINPUT_GAMEPAD_A : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_E ? XINPUT_GAMEPAD_B : 0;
		}

		// Nintendo controllers buttons: Capture & Home - changing working mode + another controllers (with additional buttons with keyboard emulation)
		if ((IsKeyPressed(VK_MENU) && IsKeyPressed('1')) || (IsKeyPressed(VK_MENU) && IsKeyPressed('2')) ||
			JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_PRO_CONTROLLER || 
			JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_LEFT || 
			JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_RIGHT) {
			
			if (SkipPollCount == 0 && ( (PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) || (IsKeyPressed(VK_MENU) && IsKeyPressed('1')) ) ) {
				if (PrimaryGamepad.GamepadActionMode == 1)
					PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
				else { PrimaryGamepad.GamepadActionMode = MotionDrivingMode; AnglesOffset = MotionAngles; }
				SkipPollCount = SkipPollTimeOut; 
			}
			
			if (SkipPollCount == 0 && ( (PrimaryGamepad.InputState.buttons & JSMASK_HOME) || (IsKeyPressed(VK_MENU) && IsKeyPressed('2')) ) ) {
				if (PrimaryGamepad.GamepadActionMode == GamepadDefaultMode || PrimaryGamepad.GamepadActionMode == MotionDrivingMode)
					PrimaryGamepad.GamepadActionMode = PrimaryGamepad.LastMotionAIMMode;
				else if (PrimaryGamepad.GamepadActionMode == MotionAimingMode)
					{ PrimaryGamepad.GamepadActionMode = MotionAimingModeOnlyPressed; PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed; }
				else { PrimaryGamepad.GamepadActionMode = MotionAimingMode; PrimaryGamepad.LastMotionAIMMode = MotionAimingMode; }
				SkipPollCount = SkipPollTimeOut;
			}

		// Sony
		} else {
			// GameBar & multi keys
			// PS without any keys
			if (PrimaryGamepad.PSReleasedCount == 0 && PrimaryGamepad.InputState.buttons == JSMASK_PS) { PrimaryGamepad.PSOnlyCheckCount = 20; PrimaryGamepad.PSOnlyPressed = true; }
			if (PrimaryGamepad.PSOnlyCheckCount > 0) {
				if (PrimaryGamepad.PSOnlyCheckCount == 1 && PrimaryGamepad.PSOnlyPressed)
					PrimaryGamepad.PSReleasedCount = PSReleasedTimeOut; // Timeout to release the PS button and don't execute commands
				PrimaryGamepad.PSOnlyCheckCount--;
				if (PrimaryGamepad.InputState.buttons != JSMASK_PS && PrimaryGamepad.InputState.buttons != 0) { PrimaryGamepad.PSOnlyPressed = false; PrimaryGamepad.PSOnlyCheckCount = 0; }
			}
			if (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons != JSMASK_PS) PrimaryGamepad.PSReleasedCount = PSReleasedTimeOut; // printf("PS + any button\n"); }
			if (PrimaryGamepad.PSReleasedCount > 0) PrimaryGamepad.PSReleasedCount--;
		}

		KeyPress(VK_GAMEBAR, (PrimaryGamepad.PSOnlyCheckCount == 1 && PrimaryGamepad.PSOnlyPressed) || (PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_HOME), &ButtonsStates.PS);

		if (JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(GamepadsID[PrimaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_RIGHT) {
			KeyPress(VK_VOLUME_DOWN2, PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_W, &ButtonsStates.VolumeDown);
			KeyPress(VK_VOLUME_UP2, PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_E, &ButtonsStates.VolumeUP);
		} else {
			KeyPress(VK_VOLUME_DOWN2, PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_W, &ButtonsStates.VolumeDown);
			KeyPress(VK_VOLUME_UP2, PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_E, &ButtonsStates.VolumeUP);
		}

		// Screenshot / record key
		bool IsSharePressed = PrimaryGamepad.InputState.buttons & JSMASK_MIC || ((PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_S); // + DualShock 4 & Nintendo
		bool IsScreenshotPressed = false;
		bool IsRecordPressed = false;

		// Microphone (custom key)
		if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode) 
			KeyPress(AppStatus.ScreenShotKey, IsSharePressed, &ButtonsStates.Screenshot);
		
		// Microphone (screenshots / record)
		else { 

			if (IsSharePressed && PrimaryGamepad.ShareHandled == false && PrimaryGamepad.ShareOnlyCheckCount == 0) {
				PrimaryGamepad.ShareHandled = true;
				PrimaryGamepad.ShareOnlyCheckCount = 20;  
				PrimaryGamepad.ShareCheckUnpressed = false;
				// printf(" Start\n");
			}

			if (PrimaryGamepad.ShareOnlyCheckCount > 0) { // Checking start
				if (IsSharePressed == false) PrimaryGamepad.ShareCheckUnpressed = true;

				if (PrimaryGamepad.ShareOnlyCheckCount == 1) {
					// Screenshot
					if (PrimaryGamepad.ShareCheckUnpressed) {
						IsScreenshotPressed = true;
						//printf(" Screenshot\n");

					// Record
					} else { 
						IsRecordPressed = true;
						//printf(" Record\n");
						//PrimaryGamepad.ShareIsRecording = !PrimaryGamepad.ShareIsRecording;
						//GamepadOutState.MicLED = PrimaryGamepad.ShareIsRecording ? MIC_LED_PULSE : MIC_LED_OFF; // doesn't work via BT :(
						//GamepadSetState(GamepadOutState);
					}
				}

				PrimaryGamepad.ShareOnlyCheckCount--;
			}

			if (!IsSharePressed && PrimaryGamepad.ShareHandled && PrimaryGamepad.ShareOnlyCheckCount == 0)
				PrimaryGamepad.ShareHandled = false;
			  
			KeyPress(AppStatus.ScreenShotKey, IsScreenshotPressed, &ButtonsStates.Screenshot);
			KeyPress(VK_GAMEBAR_RECORD, IsRecordPressed, &ButtonsStates.Record);
		}

		// Custom sens
		if (SkipPollCount == 0 && (PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE)  && PrimaryGamepad.InputState.buttons & JSMASK_N) {
			PrimaryGamepad.Motion.CustomMulSens += 0.2f;
			if (PrimaryGamepad.Motion.CustomMulSens > 2.4f)
				PrimaryGamepad.Motion.CustomMulSens = 0.2f;
 			SkipPollCount = SkipPollTimeOut;
		}
		if ((PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_RCLICK) PrimaryGamepad.Motion.CustomMulSens = 1.0f; //printf("%5.2f\n", CustomMulSens);

		// Gamepad modes
		// Motion racing  [O--]
		if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {
			if (PrimaryGamepad.Motion.WheelRoll)
				report.sThumbLX = CalcMotionStick(MotionState.gravX, MotionState.gravZ, PrimaryGamepad.Motion.WheelAngle, PrimaryGamepad.Motion.OffsetAxisX);
			else
				report.sThumbLX = ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Yaw), RadToDeg(AnglesOffset.Yaw)) * -1, PrimaryGamepad.Motion.WheelAngle); // Not tested, axes swap roles

			if (PrimaryGamepad.Motion.WheelPitch)
				report.sThumbLY = ToLeftStick(OffsetYPR(RadToDeg(MotionAngles.Pitch), RadToDeg(AnglesOffset.Pitch)) * PrimaryGamepad.Motion.WheelInvertPitch, PrimaryGamepad.Motion.WheelAngle); // Not tested, axes swap roles
		
		// Motion aiming  [--X}]
		} else if (PrimaryGamepad.GamepadActionMode == MotionAimingMode || (PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed &&
				  (AppStatus.AimingWithL2 && DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) > 0) || // Classic L2 aiming
				  (AppStatus.AimingWithL2 == false && (PrimaryGamepad.InputState.buttons & JSMASK_L)))) { // PS games with emulators

			//DWORD currentTime = GetTickCount();
			//float FrameTime = (currentTime - lastTime) / 1000.f; // Some problems with this method, we remain on a static value
			//lastTime = currentTime;
			
			// Snippet by JibbSmart https://gist.github.com/JibbSmart/8cbaba568c1c2e1193771459aa5385df
			const float InputSize = sqrtf(velocityX * velocityX + velocityY * velocityY + velocityZ * velocityZ);
			
			float TightenedSensitivity = AppStatus.AimMode == AimMouseMode ? PrimaryGamepad.Motion.SensAvg * PrimaryGamepad.Motion.CustomMulSens * 50.f : PrimaryGamepad.Motion.JoySensAvg * PrimaryGamepad.Motion.CustomMulSens * 50.f;
			
			if (InputSize < Tightening && Tightening > 0)
				TightenedSensitivity *= InputSize / Tightening;
			
			if (AppStatus.AimMode == AimMouseMode) {
				MouseMove(-velocityY * TightenedSensitivity * FrameTime * PrimaryGamepad.Motion.SensX * PrimaryGamepad.Motion.CustomMulSens, -velocityX * TightenedSensitivity * FrameTime * PrimaryGamepad.Motion.SensY * PrimaryGamepad.Motion.CustomMulSens);
			} else { // Mouse-Joystick
				report.sThumbRX = std::clamp((int)(ClampFloat(-(velocityY * TightenedSensitivity * FrameTime * PrimaryGamepad.Motion.JoySensX * PrimaryGamepad.Motion.CustomMulSens), -1, 1) * 32767 + report.sThumbRX), -32767, 32767);
				report.sThumbRY = std::clamp((int)(ClampFloat(velocityX * TightenedSensitivity * FrameTime * PrimaryGamepad.Motion.JoySensY * PrimaryGamepad.Motion.CustomMulSens, -1, 1) * 32767 + report.sThumbRY), -32767, 32767);
			}

		// [-_-] Touchpad sticks
		} else if (PrimaryGamepad.GamepadActionMode == TouchpadSticksMode) { 

			if (TouchState.t0Down) {
				if (FirstTouch.Touched == false) {
					FirstTouch.InitAxisX = TouchState.t0X;
					FirstTouch.InitAxisY = TouchState.t0Y;
					FirstTouch.Touched = true;
				}
				FirstTouch.AxisX = TouchState.t0X - FirstTouch.InitAxisX;
				FirstTouch.AxisY = TouchState.t0Y - FirstTouch.InitAxisY;

				if (FirstTouch.InitAxisX < 0.5 ) {
					report.sThumbLX = ClampFloat(FirstTouch.AxisX * PrimaryGamepad.TouchSticks.LeftX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-FirstTouch.AxisY * PrimaryGamepad.TouchSticks.LeftY, -1, 1) * 32767;
					if (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
				} else {
					report.sThumbRX = ClampFloat((TouchState.t0X - FirstTouch.LastAxisX) * PrimaryGamepad.TouchSticks.RightX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t0Y - FirstTouch.LastAxisY) * PrimaryGamepad.TouchSticks.RightY * 200, -1, 1) * 32767;
					FirstTouch.LastAxisX = TouchState.t0X; FirstTouch.LastAxisY = TouchState.t0Y;
					if (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
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
					report.sThumbLX = ClampFloat(SecondTouch.AxisX * PrimaryGamepad.TouchSticks.LeftX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-SecondTouch.AxisY * PrimaryGamepad.TouchSticks.LeftY, -1, 1) * 32767;
				} else {
					report.sThumbRX = ClampFloat((TouchState.t1X - SecondTouch.LastAxisX) * PrimaryGamepad.TouchSticks.RightX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t1Y - SecondTouch.LastAxisY) * PrimaryGamepad.TouchSticks.RightY * 200, -1, 1) * 32767;
					SecondTouch.LastAxisX = TouchState.t1X; SecondTouch.LastAxisY = TouchState.t1Y;
				}
			} else {
				SecondTouch.AxisX = 0;
				SecondTouch.AxisY = 0;
				SecondTouch.Touched = false;
			}
		}

		// Keyboard and mouse mode
		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse && !(PrimaryGamepad.InputState.buttons & JSMASK_PS)) {
			KeyPress(ButtonsStates.LeftTrigger.KeyCode, PrimaryGamepad.InputState.lTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey, &ButtonsStates.LeftTrigger);
			KeyPress(ButtonsStates.RightTrigger.KeyCode, PrimaryGamepad.InputState.rTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey, &ButtonsStates.RightTrigger);

			KeyPress(ButtonsStates.LeftBumper.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_L, &ButtonsStates.LeftBumper);
			KeyPress(ButtonsStates.RightBumper.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_R, &ButtonsStates.RightBumper);

			KeyPress(ButtonsStates.Back.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_SHARE || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE, &ButtonsStates.Back);
			KeyPress(ButtonsStates.Start.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS || PrimaryGamepad.InputState.buttons & JSMASK_HOME, &ButtonsStates.Start);

			if (ButtonsStates.DPADAdvancedMode == false) { // Regular mode  ↑ → ↓ ←
				KeyPress(ButtonsStates.DPADUp.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_UP, &ButtonsStates.DPADUp);
				KeyPress(ButtonsStates.DPADDown.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_DOWN, &ButtonsStates.DPADDown);
				KeyPress(ButtonsStates.DPADLeft.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &ButtonsStates.DPADLeft);
				KeyPress(ButtonsStates.DPADRight.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &ButtonsStates.DPADRight);
			 
			} else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖ for switching in retro games
				KeyPress(ButtonsStates.DPADUp.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_UP && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &ButtonsStates.DPADUp);
				KeyPress(ButtonsStates.DPADLeft.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_LEFT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &ButtonsStates.DPADLeft);
				KeyPress(ButtonsStates.DPADRight.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_RIGHT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &ButtonsStates.DPADRight);
				KeyPress(ButtonsStates.DPADDown.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_DOWN && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &ButtonsStates.DPADDown);

				KeyPress(ButtonsStates.DPADUpLeft.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &ButtonsStates.DPADUpLeft);
				KeyPress(ButtonsStates.DPADUpRight.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &ButtonsStates.DPADUpRight);
				KeyPress(ButtonsStates.DPADDownLeft.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &ButtonsStates.DPADDownLeft);
				KeyPress(ButtonsStates.DPADDownRight.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &ButtonsStates.DPADDownRight);
			}

			KeyPress(ButtonsStates.Y.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_N, &ButtonsStates.Y);
			KeyPress(ButtonsStates.A.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_S, &ButtonsStates.A);
			KeyPress(ButtonsStates.X.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_W, &ButtonsStates.X);
			KeyPress(ButtonsStates.B.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_E, &ButtonsStates.B);

			KeyPress(ButtonsStates.LeftStick.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_LCLICK, &ButtonsStates.LeftStick);
			KeyPress(ButtonsStates.RightStick.KeyCode, PrimaryGamepad.InputState.buttons & JSMASK_RCLICK, &ButtonsStates.RightStick);

			KMStickMode(DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX), DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY), PrimaryGamepad.KMEmu.LeftStickMode);
			KMStickMode(DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX), DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY), PrimaryGamepad.KMEmu.RightStickMode);
		}

		if (SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != 0) {
			report2.sThumbLX = SecondaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX) * 32767;
			report2.sThumbLY = SecondaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY) * 32767;
			report2.sThumbRX = SecondaryGamepad.Sticks.InvertRightX == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX) * 32767;
			report2.sThumbRY = SecondaryGamepad.Sticks.InvertRightY == false ? DeadZoneAxis(SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY) * 32767 : DeadZoneAxis(-SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY) * 32767;

			report2.bLeftTrigger = DeadZoneAxis(SecondaryGamepad.InputState.lTrigger, SecondaryGamepad.Triggers.DeadZoneLeft) * 255;
			report2.bRightTrigger = DeadZoneAxis(SecondaryGamepad.InputState.rTrigger, SecondaryGamepad.Triggers.DeadZoneRight) * 255;

			if (!(SecondaryGamepad.InputState.buttons & JSMASK_PS && SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE && SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE)) { // During special functions, nothing is pressed in the game
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_L ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_R ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_LCLICK ? XINPUT_GAMEPAD_LEFT_THUMB : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_RCLICK ? XINPUT_GAMEPAD_RIGHT_THUMB : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_UP ? XINPUT_GAMEPAD_DPAD_UP : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_DOWN ? XINPUT_GAMEPAD_DPAD_DOWN : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_LEFT ? XINPUT_GAMEPAD_DPAD_LEFT : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_RIGHT ? XINPUT_GAMEPAD_DPAD_RIGHT : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_N ? XINPUT_GAMEPAD_Y : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_W ? XINPUT_GAMEPAD_X : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_S ? XINPUT_GAMEPAD_A : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_E ? XINPUT_GAMEPAD_B : 0;
			}

			if (JslGetControllerType(GamepadsID[SecondaryGamepad.DeviceIndex]) == JS_TYPE_DS || JslGetControllerType(GamepadsID[SecondaryGamepad.DeviceIndex]) == JS_TYPE_DS4) {
				if (!(SecondaryGamepad.InputState.buttons & JSMASK_PS)) {
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
				}
			}
			else if (JslGetControllerType(GamepadsID[SecondaryGamepad.DeviceIndex]) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(GamepadsID[SecondaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(GamepadsID[SecondaryGamepad.DeviceIndex]) == JS_TYPE_JOYCON_RIGHT) {
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_MINUS ? XINPUT_GAMEPAD_BACK : 0;
				report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_PLUS ? XINPUT_GAMEPAD_START : 0;
			}

			ret = vigem_target_x360_update(client2, x3602, report2);
		}

		if (AppStatus.LastConnectionType != PrimaryGamepad.USBConnection) AppStatus.Gamepad.BTReset = true; AppStatus.LastConnectionType = PrimaryGamepad.USBConnection; // Reset if the connection has been changed. Fixes BT bug.

		if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && PrimaryGamepad.GamepadActionMode == MotionDrivingMode) || AppStatus.XboxGamepadReset) {
			if (AppStatus.XboxGamepadReset) { AppStatus.XboxGamepadReset = false; XUSB_REPORT_INIT(&report); }
			//if (AppStatus.XboxGamepadAttached)
				//ret = vigem_target_x360_update(client, x360, report);
		}

		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) { // Temporary hack(Vigem always, no removal)
			XUSB_REPORT_INIT(&report);
			report.sThumbLX = 1; // Maybe the crash is due to power saving? temporary test
		} 
		ret = vigem_target_x360_update(client, x360, report);

		// Battery level display
		if (PrimaryGamepad.BackOutStateCounter > 0) { 
			if (PrimaryGamepad.BackOutStateCounter == 1) {
				GamepadOutState.LEDColor = PrimaryGamepad.DefaultModeColor;
				GamepadOutState.PlayersCount = 0;
				if (AppStatus.ShowBatteryStatusOnLightBar) GamepadOutState.LEDBrightness = PrimaryGamepad.LastLEDBrightness;
				GamepadSetState(GamepadOutState);
				AppStatus.ShowBatteryStatus = false;
				MainTextUpdate(); 
			}
			PrimaryGamepad.BackOutStateCounter--;
		}

		// Nintendo rumble hack
		if (PrimaryGamepad.RumbleOffCounter > 0) {
			if (PrimaryGamepad.RumbleOffCounter == 1) {
				GamepadOutState.SmallMotor = 0;
				GamepadOutState.LargeMotor = 0;
				GamepadSetState(GamepadOutState);
			}
			PrimaryGamepad.RumbleOffCounter--;
		}

		if (SkipPollCount > 0) SkipPollCount--;
		Sleep(AppStatus.SleepTimeOut);
	}

	JslDisconnectAndDisposeAll();
	if (PrimaryGamepad.HidHandle != NULL)
		hid_close(PrimaryGamepad.HidHandle);

	if (AppStatus.ExternalPedalsArduinoConnected) {
		AppStatus.ExternalPedalsArduinoConnected = false;
		pArduinoReadThread->join();
		delete pArduinoReadThread;
		pArduinoReadThread = nullptr;
		CloseHandle(hSerial);
	}

	//if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)) {
	vigem_target_x360_unregister_notification(x360);
	vigem_target_remove(client, x360);
	vigem_target_free(x360);
	//}

	vigem_disconnect(client);
	vigem_free(client);

	if (SecondaryGamepadEnabled) {
		vigem_target_x360_unregister_notification(x3602);
		vigem_target_remove(client2, x3602);
		vigem_target_free(x3602);

		vigem_disconnect(client2);
		vigem_free(client2);
	}
}
