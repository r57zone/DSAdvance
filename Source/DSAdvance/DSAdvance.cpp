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

void GamepadSearch(AdvancedGamepad &Gamepad, std::string SkipDevPath) {
	struct hid_device_info *cur_dev;

	// Sony controllers
	cur_dev = hid_enumerate(SONY_VENDOR, 0x0);
	while (cur_dev) {
		if (!SkipDevPath.empty() && SkipDevPath == cur_dev->path) { cur_dev = cur_dev->next; continue; }
		if (cur_dev->product_id == SONY_DS5 ||
			cur_dev->product_id == SONY_DS5_EDGE ||
			cur_dev->product_id == SONY_DS4_USB ||
			cur_dev->product_id == SONY_DS4_V2_USB ||
			cur_dev->product_id == SONY_DS4_BT ||
			cur_dev->product_id == SONY_DS4_DONGLE)
		{
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			Gamepad.DevicePath = cur_dev->path;
			hid_set_nonblocking(Gamepad.HidHandle, 1);

			if (cur_dev->product_id == SONY_DS5 || cur_dev->product_id == SONY_DS5_EDGE) {
				Gamepad.ControllerType = SONY_DUALSENSE;
				Gamepad.USBConnection = true;

				// BT detection https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp
				unsigned char buf[64];
				memset(buf, 0, 64);
				hid_read_timeout(Gamepad.HidHandle, buf, 64, 100);
				if (buf[0] == 0x31)
					Gamepad.USBConnection = false;

			} else if (cur_dev->product_id == SONY_DS4_USB || cur_dev->product_id == SONY_DS4_V2_USB || cur_dev->product_id == SONY_DS4_DONGLE) {
				Gamepad.ControllerType = SONY_DUALSHOCK4;
				Gamepad.USBConnection = true;

				// JoyShock Library apparently sent something, so it worked without a package (needed for BT detection to work, does not affect USB)
				unsigned char checkBT[2] = { 0x02, 0x00 };
				hid_write(Gamepad.HidHandle, checkBT, sizeof(checkBT));

				// BT detection for compatible gamepads that output USB VID/PID on BT connection
				unsigned char buf[64];
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read_timeout(Gamepad.HidHandle, buf, sizeof(buf), 100);
				if (bytesRead > 0 && buf[0] == 0x11)
					Gamepad.USBConnection = false;

				//printf("Detected device ID: 0x%X\n", cur_dev->product_id);
				//if (Gamepad.USBConnection) printf("USB"); else printf("Wireless");

			} else if (cur_dev->product_id == SONY_DS4_BT) { // ?
				Gamepad.ControllerType = SONY_DUALSHOCK4;
				Gamepad.USBConnection = false;
			}
			break;
		}
		cur_dev = cur_dev->next;
	}

	// Sony compatible controllers
	cur_dev = hid_enumerate(BROOK_DS4_VENDOR, 0x0);
	while (cur_dev) {
		if (!SkipDevPath.empty() && SkipDevPath == cur_dev->path) { cur_dev = cur_dev->next; continue; }
		if (cur_dev->product_id == BROOK_DS4_USB)
		{
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			hid_set_nonblocking(Gamepad.HidHandle, 1);
			Gamepad.USBConnection = true;
			Gamepad.ControllerType = SONY_DUALSHOCK4;
			break;
		}
		cur_dev = cur_dev->next;
	}

	// Nintendo compatible controllers
	cur_dev = hid_enumerate(NINTENDO_VENDOR, 0x0);
	while (cur_dev) {
		if (!SkipDevPath.empty() && SkipDevPath == cur_dev->path) { cur_dev = cur_dev->next; continue; }
		if (cur_dev->product_id == NINTENDO_JOYCON_L)
		{
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			Gamepad.DevicePath = cur_dev->path;
			hid_set_nonblocking(Gamepad.HidHandle, 1);
			Gamepad.USBConnection = false;
			Gamepad.ControllerType = NINTENDO_JOYCONS;
		}
		else if (cur_dev->product_id == NINTENDO_JOYCON_R) {
			Gamepad.HidHandle2 = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			Gamepad.DevicePath2 = cur_dev->path;
			hid_set_nonblocking(Gamepad.HidHandle2, 1);
			Gamepad.USBConnection = false;
			Gamepad.ControllerType = NINTENDO_JOYCONS;
		}
		else if (cur_dev->product_id == NINTENDO_SWITCH_PRO) {
			Gamepad.HidHandle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
			Gamepad.DevicePath = cur_dev->path;
			Gamepad.ControllerType = NINTENDO_SWITCH_PRO;
			hid_set_nonblocking(Gamepad.HidHandle, 1);
			//Gamepad.USBConnection = true;
			Gamepad.RumbleSkipCounter = 300;

			// Conflict with JoyShock Library ???
			unsigned char buf[64] = {};
			buf[0] = 0x80;
			buf[1] = 0x01;

			int written = hid_write(Gamepad.HidHandle, buf, 2);
			if (written > 0) {
				Gamepad.USBConnection = true;
				/*unsigned char buf[64];
				memset(buf, 0, sizeof(buf));
				int bytesRead = hid_read_timeout(Gamepad.HidHandle, buf, sizeof(buf), 100);
				if (bytesRead > 0 && (buf[0] == 0x81 || buf[0] == 0x21 || buf[0] == 0x30))
					Gamepad.USBConnection = true;*/
			} else
				Gamepad.USBConnection = false;

			//Gamepad.USBConnection = (cur_dev->serial_number != NULL);
		}
		cur_dev = cur_dev->next;
	}

	//printf("\nFound: %d\n", Gamepad.ControllerType);

	if (cur_dev)
		hid_free_enumeration(cur_dev);
}

static float MotorFreqFromStrength(unsigned char motorValue) { // Dynamic frequency
	if (motorValue == 0) return 40.0f;
	return 40.0f + (motorValue / 255.0f) * (320.0f - 40.0f);
}

static void EncodeRumble(unsigned char* data, float freq, float amp) {
	if (freq < 41.0f) freq = 41.0f;
	if (freq > 1253.0f) freq = 1253.0f;
	if (amp < 0.0f) amp = 0.0f;
	if (amp > 1.0f) amp = 1.0f;

	uint16_t hf = (uint16_t)(320.0f * log2f(freq * 0.1f) + 0.5f);
	uint8_t hf_byte = (hf - (hf % 4)) / 4;
	uint8_t lf_byte = (uint8_t)((freq * 0.1f) / powf(2.0f, (hf_byte - 0x60) / 32.0f));

	uint16_t amp_enc = (uint16_t)(amp * 0x7FFF);
	uint8_t amp_hi = (amp_enc >> 8) & 0xFF;
	uint8_t amp_lo = amp_enc & 0xFF;

	data[0] = lf_byte;
	data[1] = hf_byte;
	data[2] = amp_lo;
	data[3] = amp_hi;
}

// https://github.com/fossephate/JoyCon-Driver/blob/main/joycon-driver/include/Joycon.hpp
void JoyConSimpleRumble(hid_device* jcHandle, bool IsLeft, unsigned char MotorValue)
{
	unsigned char outputReport[64] = { 0 };

	outputReport[0] = 0x10;
	outputReport[1] = PrimaryGamepad.PacketCounter++ & 0x0f;

	if (IsLeft) {
		if (MotorValue == 0) {
			outputReport[2] = 0x00;
			outputReport[3] = 0x01;
			outputReport[4] = 0x40;
			outputReport[5] = 0x40;
		}
		else
			EncodeRumble(&outputReport[2], MotorFreqFromStrength(MotorValue), (MotorValue * PrimaryGamepad.RumbleStrength * 0.9f) / 25500.0f); // It seems that values above 90% may cause wear on the motors of Nintendo controllers.
	}
	else { // Is right
		if (MotorValue == 0) {
			outputReport[6] = 0x00;
			outputReport[7] = 0x01;
			outputReport[8] = 0x40;
			outputReport[9] = 0x40;
		}
		else
			EncodeRumble(&outputReport[6], MotorFreqFromStrength(MotorValue), (MotorValue * PrimaryGamepad.RumbleStrength * 0.9f) / 25500.0f);
	}

	hid_write(jcHandle, outputReport, sizeof(outputReport));
}

void GamepadSetState(AdvancedGamepad &Gamepad)
{
	if (Gamepad.HidHandle != NULL) {
		if (Gamepad.ControllerType == SONY_DUALSENSE) { // https://www.reddit.com/r/gamedev/comments/jumvi5/dualsense_haptics_leds_and_more_hid_output_report/

			unsigned char PlayersDSPacket = 0;

			if (Gamepad.OutState.PlayersCount == 0) PlayersDSPacket = 0;
			else if (Gamepad.OutState.PlayersCount == 1) PlayersDSPacket = 4;
			else if (Gamepad.OutState.PlayersCount == 2) PlayersDSPacket = 2; // Center 2
			else if (Gamepad.OutState.PlayersCount == 5) PlayersDSPacket = 1; // Both 2
			else if (Gamepad.OutState.PlayersCount == 3) PlayersDSPacket = 5;
			else if (Gamepad.OutState.PlayersCount == 4) PlayersDSPacket = 3;

			Gamepad.OutState.LEDRed = (Gamepad.OutState.LEDColor >> 16) & 0xFF;
			Gamepad.OutState.LEDGreen = (Gamepad.OutState.LEDColor >> 8) & 0xFF;
			Gamepad.OutState.LEDBlue = Gamepad.OutState.LEDColor & 0xFF;

			if (Gamepad.USBConnection) {
				unsigned char outputReport[48];
				memset(outputReport, 0, 48);

				outputReport[0] = 0x02;
				outputReport[1] = 0xff;
				outputReport[2] = 0x15;
				outputReport[3] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
				outputReport[4] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;
				outputReport[5] = 0xff;
				outputReport[6] = 0xff;
				outputReport[7] = 0xff;
				outputReport[8] = 0x0c;
				//outputReport[9] = OutState.MicLED;
				outputReport[38] = 0x07;
				outputReport[44] = PlayersDSPacket;
				outputReport[45] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[46] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[47] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

				// Adaptive triggers
				if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RUMBLE_MODE) // Rumble translation
				{
					// Left trigger
					outputReport[21] = 0x06;   // Continuous Resistance
					outputReport[22] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 2300; // 2300 - softer
					outputReport[23] = 0x09;   // Начало триггера (почти с нуля)
					outputReport[24] = 0xFF;   // Конец триггера (100%)
					outputReport[25] = 0x00;   // Без вибрации

					// Right trigger
					outputReport[11] = 0x06;      // Pulse mode
					outputReport[12] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 2300; // 2300 - softer
					outputReport[13] = 3;          // старт чуть позже — не так резко
					outputReport[14] = 8;          // короткая серия импульсов
					outputReport[15] = 0x18;       // частота импульсов ниже — мягкая отдача
				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_PISTOL_MODE) // Pistol / Пистолет
				{
					// Пистолет: плавное сопротивление по всему ходу
					outputReport[11] = 0x02;   // Continuous Resistance
					outputReport[12] = 35;     // Средняя сила сопротивления
					outputReport[13] = 0x09;   // Начало триггера (почти с нуля)
					outputReport[14] = 0xFF;   // Конец триггера (100%)
					outputReport[15] = 0x00;   // Без вибрации

				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_AUTOMATIC_MODE) // Automatic / Machine Gun — серия коротких импульсов
				{
					outputReport[11] = 0x06;   // Pulse mode
					outputReport[12] = 15;     // Сила каждого импульса (легкая)
					outputReport[13] = 2;      // Старт почти сразу при лёгком нажатии
					outputReport[14] = 10;     // Конец короткой серии импульсов
					outputReport[15] = 0x20;   // Частота импульсов (выше — имитация очереди)

				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RIFLE_MODE) // Sniper Rifle — винтовка с усилием
				{
					/* Более легкий вариант
					outputReport[11] = 0x26;   // Resistance + лёгкая вибрация
					outputReport[12] = 120;    // более сильное сопротивление
					outputReport[13] = 0x00;   // начало
					outputReport[14] = 0xE0;   // конец почти полной
					outputReport[15] = 0x05;   // частота вибрации
					outputReport[16] = 0xF0;   // короткий резкий толчок
					outputReport[17] = 0x40;   // сила толчка — ощущается реально*/

					outputReport[11] = 0x25;
					outputReport[12] = 0x04; // low (1<<2)
					outputReport[13] = 0x01; // high(1<<8)
					outputReport[14] = 0x06; // strength-1 (7-1)
					outputReport[15] = 0x00;
					outputReport[16] = 0x00;
					outputReport[17] = 0x00;
					outputReport[18] = 0x00;
					outputReport[19] = 0x00;
					outputReport[20] = 0x00;
					outputReport[21] = 0x00;

				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_BOW_MODE) // Лук — прогрессивное натяжение
				{
					outputReport[11] = 0x22;
					outputReport[12] = 0x01; // low (1<<0)
					outputReport[13] = 0x01; // high(1<<8)
					outputReport[14] = 0x33; // (strength-1) | ((snap-1)<<3) => (4-1)=3, (7-1)=6 => 0x03 | (0x06<<3)=0x33
					outputReport[15] = 0x00;
					outputReport[16] = 0x00;
					outputReport[17] = 0x00;
					outputReport[18] = 0x00;
					outputReport[19] = 0x00;
					outputReport[20] = 0x00;
					outputReport[21] = 0x00;

				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_CAR_MODE) // Педаль авто
				{
					/* Слишком сильное
					outputReport[11] = 0x02;   // Continuous resistance
					outputReport[12] = 0x10;   // слабое в начале
					outputReport[13] = 0xFF;   // конец хода
					outputReport[14] = 0x20;   // начальная сила
					outputReport[15] = 0xF0;   // максимальная сила — реально чувствуется
					*/

					outputReport[11] = 0x21;
					outputReport[12] = 0xFF; // активные зоны 0..9
					outputReport[13] = 0x03;
					outputReport[14] = 0x24; // amplitude zones для strength=5 (повтор "100")
					outputReport[15] = 0x92;
					outputReport[16] = 0x49;
					outputReport[17] = 0x24;
					outputReport[18] = 0x00;
					outputReport[19] = 0x00;
					outputReport[20] = 0x00;
					outputReport[21] = 0x00;

				}

				// Left trigger
				if (Gamepad.AdaptiveTriggersOutputMode > 1)
				{
					outputReport[21] = 0x02;   // Continuous Resistance
					outputReport[22] = 35;     // Средняя сила сопротивления
					outputReport[23] = 0x09;   // Начало триггера (почти с нуля)
					outputReport[24] = 0xFF;   // Конец триггера (100%)
					outputReport[25] = 0x00;   // Без вибрации

				}

				hid_write(Gamepad.HidHandle, outputReport, 48);

				
			}
			// DualSense BT
			else {
				unsigned char outputReport[79]; // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds5_rumble_light_bt)
				memset(outputReport, 0, 79);

				outputReport[0] = 0xa2;
				outputReport[1] = 0x31;
				outputReport[2] = 0x02;
				outputReport[3] = 0x03;
				outputReport[4] = 0x54;
				outputReport[5] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
				outputReport[6] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;
				outputReport[11] = 0x00; // Gamepad.OutState.MicLED - not working
				outputReport[41] = 0x02;
				outputReport[44] = 0x02;
				outputReport[45] = 0x02;
				outputReport[46] = PlayersDSPacket;
				//outputReport[46] &= ~(1 << 7);
				//outputReport[46] &= ~(1 << 8);
				outputReport[47] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[48] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[49] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

				// https://github.com/Valkirie/JoyShockLibrary/commit/f4fffb6faa53f0839130b093690ca292f23f115e
				if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RUMBLE_MODE) // Rumble translation
				{
					// Left trigger (USB: 21..25) -> BT: 24..28
					outputReport[3] |= 0x08;              // dirty L2
					outputReport[24] = 0x06;              // Continuous Resistance
					outputReport[25] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 2300;
					outputReport[26] = 0x09;              // начало
					outputReport[27] = 0xFF;              // конец
					outputReport[28] = 0x00;              // без вибрации

					// Right trigger (USB: 11..15) -> BT: 13..17
					outputReport[3] |= 0x04;              // dirty R2
					outputReport[13] = 0x06;              // Pulse mode
					outputReport[14] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 2300;
					outputReport[15] = 3;                 // старт чуть позже
					outputReport[16] = 8;                 // короткая серия
					outputReport[17] = 0x18;              // частота
				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_PISTOL_MODE) // Пистолет
				{
					// Только правый, как по USB
					outputReport[3] |= 0x04;              // dirty R2
					outputReport[13] = 0x02;              // Continuous Resistance
					outputReport[14] = 35;                // сила
					outputReport[15] = 0x09;              // начало
					outputReport[16] = 0xFF;              // конец
					outputReport[17] = 0x00;              // без вибрации
				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_AUTOMATIC_MODE) // Automatic / очередь
				{
					outputReport[3] |= 0x04;              // dirty R2
					outputReport[13] = 0x06;              // Pulse mode
					outputReport[14] = 15;                // сила импульса
					outputReport[15] = 2;                 // старт
					outputReport[16] = 10;                // конец серии
					outputReport[17] = 0x20;              // частота
				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_RIFLE_MODE) // Винтовка
				{
					outputReport[3] |= 0x04;              // dirty R2
					outputReport[13] = 0x25;
					outputReport[14] = 0x04;
					outputReport[15] = 0x01;
					outputReport[16] = 0x06;
					outputReport[17] = 0x00;
					outputReport[18] = 0x00;
					outputReport[19] = 0x00;
					outputReport[20] = 0x00;
					outputReport[21] = 0x00;
					outputReport[22] = 0x00;
					outputReport[23] = 0x00;
				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_BOW_MODE) // Лук
				{
					outputReport[3] |= 0x04;              // dirty R2
					outputReport[13] = 0x22;
					outputReport[14] = 0x01;              // low
					outputReport[15] = 0x01;              // high
					outputReport[16] = 0x33;              // как по USB
					outputReport[17] = 0x00;
					outputReport[18] = 0x00;
					outputReport[19] = 0x00;
					outputReport[20] = 0x00;
					outputReport[21] = 0x00;
					outputReport[22] = 0x00;
					outputReport[23] = 0x00;
				}
				else if (Gamepad.AdaptiveTriggersOutputMode == ADAPTIVE_TRIGGERS_CAR_MODE) // Педаль авто
				{
					outputReport[3] |= 0x04;              // dirty R2
					outputReport[13] = 0x21;
					outputReport[14] = 0xFF;              // активные зоны 0..9
					outputReport[15] = 0x03;
					outputReport[16] = 0x24;              // amplitude zones
					outputReport[17] = 0x92;
					outputReport[18] = 0x49;
					outputReport[19] = 0x24;
					outputReport[20] = 0x00;
					outputReport[21] = 0x00;
					outputReport[22] = 0x00;
					outputReport[23] = 0x00;
				}
				else
				{
					// режим 0 / неизвестный — сброс обоих триггеров
					outputReport[3] |= 0x0C;              // dirty R2+L2
				}

				// Left trigger
				if (Gamepad.AdaptiveTriggersOutputMode > 1)
				{
					outputReport[3] |= 0x08;              // dirty L2
					outputReport[24] = 0x02;              // Continuous Resistance
					outputReport[25] = 35;                // Средняя сила
					outputReport[26] = 0x09;              // Начало
					outputReport[27] = 0xFF;              // Конец
					outputReport[28] = 0x00;              // Без вибрации
				}

				uint32_t crc = crc_32(outputReport, 75);
				memcpy(&outputReport[75], &crc, 4);

				hid_write(Gamepad.HidHandle, &outputReport[1], 78);
			}

		}
		else if (Gamepad.ControllerType == SONY_DUALSHOCK4) { // JoyShockLibrary rumble working for USB DS4 ??? 
			Gamepad.OutState.LEDRed = (Gamepad.OutState.LEDColor >> 16) & 0xFF;
			Gamepad.OutState.LEDGreen = (Gamepad.OutState.LEDColor >> 8) & 0xFF;
			Gamepad.OutState.LEDBlue = Gamepad.OutState.LEDColor & 0xFF;

			if (Gamepad.USBConnection) {
				unsigned char outputReport[31];
				memset(outputReport, 0, 31);

				outputReport[0] = 0x05;
				outputReport[1] = 0xff;
				outputReport[4] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
				outputReport[5] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;
				outputReport[6] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[7] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[8] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

				hid_write(Gamepad.HidHandle, outputReport, 31);

				// DualShock 4 BT
			}
			else { // https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/JoyShock.cpp (set_ds4_rumble_light_bt)
				unsigned char outputReport[79];
				memset(outputReport, 0, 79);

				outputReport[0] = 0xa2;
				outputReport[1] = 0x11;
				outputReport[2] = 0xc0;
				outputReport[3] = 0x20;
				outputReport[4] = 0x07;
				outputReport[5] = 0x00;
				outputReport[6] = 0x00;

				outputReport[7] = (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100;
				outputReport[8] = (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100;

				outputReport[9] = std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[10] = std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255);
				outputReport[11] = std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255);

				outputReport[12] = 0xff;
				outputReport[13] = 0x00;

				uint32_t crc = crc_32(outputReport, 75);
				memcpy(&outputReport[75], &crc, 4);

				hid_write(Gamepad.HidHandle, &outputReport[1], 78);
			}

		}
		else if (Gamepad.ControllerType == NINTENDO_JOYCONS && !Gamepad.USBConnection) {
			if (Gamepad.RumbleStrength != 0) {
				// Left JoyCon
				/*unsigned char outputReportLeft[64] = { 0 };
				outputReportLeft[0] = 0x10;
				outputReportLeft[1] = Gamepad.PacketCounter++ & 0x0f;
				outputReportLeft[6] = 0x00;
				outputReportLeft[7] = 0x01;
				outputReportLeft[8] = 0x40;
				outputReportLeft[9] = 0x40;

				if (OutState.LargeMotor == 0) {
					outputReportLeft[2] = 0x00;
					outputReportLeft[3] = 0x01;
					outputReportLeft[4] = 0x40;
					outputReportLeft[5] = 0x40;
				} else
					EncodeRumble(&outputReportLeft[2], MotorFreqFromStrength(Gamepad.OutState.LargeMotor), (Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength * 0.9f) / 25500.0f);
				hid_write(Gamepad.HidHandle, outputReportLeft, sizeof(outputReportLeft));

				// Right JoyCon
				if (Gamepad.HidHandle2) {
					unsigned char outputReportRight[64] = { 0 };
					outputReportRight[0] = 0x10;
					outputReportRight[1] = Gamepad.PacketCounter++ & 0x0f;

					outputReportRight[6] = 0x00;
					outputReportRight[7] = 0x01;
					outputReportRight[8] = 0x40;
					outputReportRight[9] = 0x40;

					if (OutState.SmallMotor == 0) {
						outputReportRight[6] = 0x00;
						outputReportRight[7] = 0x01;
						outputReportRight[8] = 0x40;
						outputReportRight[9] = 0x40;
					}
					else
						EncodeRumble(&outputReportRight[6], MotorFreqFromStrength(Gamepad.OutState.SmallMotor), (Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength * 0.9f) / 25500.0f);

					hid_write(Gamepad.HidHandle2, outputReportRight, sizeof(outputReportRight));
				}*/

				JoyConSimpleRumble(Gamepad.HidHandle, true, AppStatus.JoyconRumbleMerge == false ? Gamepad.OutState.LargeMotor : (Gamepad.OutState.LargeMotor + Gamepad.OutState.SmallMotor) / 2);
				if (Gamepad.HidHandle2)
					JoyConSimpleRumble(Gamepad.HidHandle2, false, AppStatus.JoyconRumbleMerge == false ? Gamepad.OutState.SmallMotor : (Gamepad.OutState.LargeMotor + Gamepad.OutState.SmallMotor) / 2);

			}
		}
		else if (Gamepad.ControllerType == NINTENDO_SWITCH_PRO) { // && !Gamepad.USBConnection
			//printf("rumble\n");
			//JslSetRumble(0, (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100, (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100);
			if (Gamepad.RumbleStrength != 0) {
				if (!Gamepad.USBConnection || Gamepad.RumbleSkipCounter == 0) { // Wireless or wired with skip JoyShockLibrary init
					unsigned char outputReport[64] = { 0 };
					outputReport[0] = 0x10;
					outputReport[1] = Gamepad.PacketCounter++ & 0x0f;
					EncodeRumble(&outputReport[2], MotorFreqFromStrength(Gamepad.OutState.SmallMotor), (Gamepad.OutState.SmallMotor / 255.0f) * (Gamepad.RumbleStrength / 100.0f));
					EncodeRumble(&outputReport[6], MotorFreqFromStrength(Gamepad.OutState.LargeMotor), (Gamepad.OutState.LargeMotor / 255.0f) * (Gamepad.RumbleStrength / 100.0f));
					hid_write(Gamepad.HidHandle, outputReport, 64);
				}
			}
		} //else {
			//if (JslGetControllerType(0) == JS_TYPE_DS || JslGetControllerType(0) == JS_TYPE_DS4)
				//JslSetLightColour(0, (std::clamp(Gamepad.OutState.LEDRed - Gamepad.OutState.LEDBrightness, 0, 255) << 16) + (std::clamp(Gamepad.OutState.LEDGreen - Gamepad.OutState.LEDBrightness, 0, 255) << 8) + std::clamp(Gamepad.OutState.LEDBlue - Gamepad.OutState.LEDBrightness, 0, 255)); // https://github.com/CyberPlaton/_Nautilus_/blob/master/Engine/PSGamepad.cpp
			//JslSetRumble(0, (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100, (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100); // Not working with DualSense USB connection
		//}
	}
	//else // Unknown controllers - Pro controller, Joy-cons
		//JslSetRumble(0, (unsigned int)Gamepad.OutState.LargeMotor * Gamepad.RumbleStrength / 100, (unsigned int)Gamepad.OutState.SmallMotor * Gamepad.RumbleStrength / 100);
}

void UpdateBatteryInfo(AdvancedGamepad &Gamepad) {
	if (Gamepad.HidHandle != NULL) {
		if (Gamepad.ControllerType == SONY_DUALSENSE) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(Gamepad.HidHandle, buf, 64);
			if (Gamepad.USBConnection) {
				Gamepad.LEDBatteryLevel = (buf[53] & 0x0f) / 2 + 1; // "+1" for the LED to be responsible for 25%. Each unit of battery data corresponds to 10%, 0 = 0 - 9 % , 1 = 10 - 19 % , .. and 10 = 100 %
				//??? in charge mode, need to show animation within a few seconds 
				Gamepad.BatteryMode = ((buf[52] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT; // 0x0 - discharging, 0x1 - full, 0x2 - charging, 0xa & 0xb - not-charging, 0xf - unknown
				//printf(" Battery status: %d\n", Gamepad.BatteryMode); // if there is charging, then we don't add 1 led
				Gamepad.BatteryLevel = (buf[53] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
			}
			else { // BT
				Gamepad.LEDBatteryLevel = (buf[54] & 0x0f) / 2 + 1;
				Gamepad.BatteryMode = ((buf[53] & 0x0f) & DS_STATUS_CHARGING) >> DS_STATUS_CHARGING_SHIFT;
				Gamepad.BatteryLevel = (buf[54] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
				//printf(" Battery status: %d\n", Gamepad.BatteryMode); // if there is charging, then we don't add 1 led
			}
			if (Gamepad.LEDBatteryLevel > 4) // min(data * 10 + 5, 100);
				Gamepad.LEDBatteryLevel = 4;
		}
		else if (Gamepad.ControllerType == SONY_DUALSHOCK4) {
			unsigned char buf[64];
			memset(buf, 0, 64);
			hid_read(Gamepad.HidHandle, buf, 64);
			if (Gamepad.USBConnection)
				Gamepad.BatteryLevel = (buf[30] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS4_USB_BATTERY_MAX;
			else
				Gamepad.BatteryLevel = (buf[32] & DS_STATUS_BATTERY_CAPACITY) * 100 / DS_BATTERY_MAX;
		}
		else if (Gamepad.ControllerType == NINTENDO_JOYCONS || Gamepad.ControllerType == NINTENDO_SWITCH_PRO) {
			unsigned char buf[64];
			memset(buf, 0, sizeof(buf));
			hid_read(Gamepad.HidHandle, buf, 64);
			Gamepad.BatteryLevel = ((buf[2] >> 4) & 0x0F) * 100 / 8;

			if (Gamepad.HidHandle2 != NULL) {
				memset(buf, 0, sizeof(buf));
				hid_read(Gamepad.HidHandle2, buf, 64);
				Gamepad.BatteryLevel2 = ((buf[2] >> 4) & 0x0F) * 100 / 8;
			}
		}
		if (Gamepad.BatteryLevel > 100) Gamepad.BatteryLevel = 100; // It looks like something is not right, once it gave out 125%
	}
}

void GetBatteryInfo() {
	UpdateBatteryInfo(PrimaryGamepad);
	if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1)
		UpdateBatteryInfo(SecondaryGamepad);
}

void ShowBatteryLevels() {
	GetBatteryInfo(); if (AppStatus.BackOutStateCounter == 0) AppStatus.BackOutStateCounter = 40; // It is executed many times, so it is done this way, it is necessary to save the old brightness value for return
	if (AppStatus.ShowBatteryStatusOnLightBar) {
		// Primary gamepad
		if (AppStatus.BackOutStateCounter == 40) PrimaryGamepad.LastLEDBrightness = PrimaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
		if (PrimaryGamepad.BatteryLevel >= 30) // Battery fine 30%-100%
			PrimaryGamepad.OutState.LEDColor = AppStatus.BatteryFineColor;
		else if (PrimaryGamepad.BatteryLevel >= 10) // Battery warning 10..29%
			PrimaryGamepad.OutState.LEDColor = AppStatus.BatteryWarningColor;
		else // battery critical 10%
			PrimaryGamepad.OutState.LEDColor = AppStatus.BatteryCriticalColor;
		PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;

		// Secondary gamepad
		if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			if (AppStatus.BackOutStateCounter == 40) SecondaryGamepad.LastLEDBrightness = SecondaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
			if (SecondaryGamepad.BatteryLevel >= 30) // Battery fine 30%-100%
				SecondaryGamepad.OutState.LEDColor = AppStatus.BatteryFineColor;
			else if (SecondaryGamepad.BatteryLevel >= 10) // Battery warning 10..29%
				SecondaryGamepad.OutState.LEDColor = AppStatus.BatteryWarningColor;
			else // battery critical 10%
				SecondaryGamepad.OutState.LEDColor = AppStatus.BatteryCriticalColor;
			SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
		}

	}
	PrimaryGamepad.OutState.PlayersCount = PrimaryGamepad.LEDBatteryLevel; // JslSetPlayerNumber(PrimaryGamepad.DeviceIndex, 5);
	if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
		SecondaryGamepad.OutState.PlayersCount = SecondaryGamepad.LEDBatteryLevel;
}

// wMid - Vendor id, wPid - Product id
void ExternalPedalsDInputSearch() {
	ExternalPedalsConnected = false;
	for (int JoyID = 0; JoyID < 4; ++JoyID) { // JOYSTICKID4 - 3
		if (joyGetPosEx(JoyID, &AppStatus.ExternalPedalsJoyInfo) == JOYERR_NOERROR && // JoyID - JOYSTICKID1..4
			joyGetDevCaps(JoyID, &AppStatus.ExternalPedalsJoyCaps, sizeof(AppStatus.ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
			(AppStatus.ExternalPedalsJoyCaps.wMid != 1406) && // Exclude Pro Controller и JoyCon
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
		PrimaryGamepad.OutState.LargeMotor = LargeMotor;
		PrimaryGamepad.OutState.SmallMotor = SmallMotor;
		GamepadSetState(PrimaryGamepad);

		// SecondaryGamepad
	}
	else if (gamepadID == 2 && AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
		SecondaryGamepad.OutState.LargeMotor = LargeMotor;
		SecondaryGamepad.OutState.SmallMotor = SmallMotor;
		GamepadSetState(SecondaryGamepad);
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

void KMStickMode(AdvancedGamepad &Gamepad, bool DontResetInputState, bool StickIsLeft, float StickX, float StickY, int Mode) {
	if (Mode == WASDStickMode) {
		KeyPress('W', DontResetInputState && StickY > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Up, true);
		KeyPress('S', DontResetInputState && StickY < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Down, true);
		KeyPress('A', DontResetInputState && StickX < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Left, true);
		KeyPress('D', DontResetInputState && StickX > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Right, true);
	} else if (Mode == ArrowsStickMode) {
		KeyPress(VK_UP, DontResetInputState && StickY > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Up, true);
		KeyPress(VK_DOWN, DontResetInputState && StickY < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Down, true);
		KeyPress(VK_LEFT, DontResetInputState && StickX < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Left, true);
		KeyPress(VK_RIGHT, DontResetInputState && StickX > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Right, true);
	} else if (Mode == MouseLookStickMode)
		MouseMove(StickX * Gamepad.KMEmu.JoySensX, -StickY * Gamepad.KMEmu.JoySensY);
	else if (Mode == MouseWheelStickMode)
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, StickY * 50, 0);
	else if (Mode == NumpadsStickMode) {
		KeyPress(VK_NUMPAD8, DontResetInputState && StickY > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Up, true);
		KeyPress(VK_NUMPAD2, DontResetInputState && StickY < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Down, true);
		KeyPress(VK_NUMPAD4, DontResetInputState && StickX < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Left, true);
		KeyPress(VK_NUMPAD6, DontResetInputState && StickX > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.Right, true);
	} else if (Mode == CustomStickMode) {
		if (StickIsLeft) {
			KeyPress(Gamepad.ButtonsStates.LeftStickUp.KeyCode, DontResetInputState && StickY > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.LeftStickUp, true);
			KeyPress(Gamepad.ButtonsStates.LeftStickDown.KeyCode, DontResetInputState && StickY < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.LeftStickDown, true);
			KeyPress(Gamepad.ButtonsStates.LeftStickLeft.KeyCode, DontResetInputState && StickX < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.LeftStickLeft, true);
			KeyPress(Gamepad.ButtonsStates.LeftStickRight.KeyCode, DontResetInputState && StickX > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.LeftStickRight, true);
		} else {
			KeyPress(Gamepad.ButtonsStates.RightStickUp.KeyCode, DontResetInputState && StickY > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.RightStickUp, true);
			KeyPress(Gamepad.ButtonsStates.RightStickDown.KeyCode, DontResetInputState && StickY < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.RightStickDown, true);
			KeyPress(Gamepad.ButtonsStates.RightStickLeft.KeyCode, DontResetInputState && StickX < -Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.RightStickLeft, true);
			KeyPress(Gamepad.ButtonsStates.RightStickRight.KeyCode, DontResetInputState && StickX > Gamepad.KMEmu.StickValuePressKey, &Gamepad.ButtonsStates.RightStickRight, true);
		}
	}
}

void LoadKMProfile(std::string ProfileFile) {
	CIniReader IniFile("KMProfiles\\" + ProfileFile);

	// Primary gamepad
	PrimaryGamepad.ButtonsStates.LeftTrigger.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LT", "MOUSE-RIGHT"));
	PrimaryGamepad.ButtonsStates.RightTrigger.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RT", "MOUSE-LEFT"));

	PrimaryGamepad.ButtonsStates.LeftBumper.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "L1-LB", "F"));
	PrimaryGamepad.ButtonsStates.RightBumper.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "R1-RB", "G"));

	PrimaryGamepad.ButtonsStates.Back.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "BACK", "ESCAPE"));
	PrimaryGamepad.ButtonsStates.Start.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "START", "ENTER"));

	PrimaryGamepad.ButtonsStates.DPADUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "UP", "2"));
	PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LEFT", "1"));
	PrimaryGamepad.ButtonsStates.DPADRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RIGHT", "3"));
	PrimaryGamepad.ButtonsStates.DPADDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "DOWN", "4"));

	PrimaryGamepad.ButtonsStates.DPADUpLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "UP-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADUpRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "UP-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADDownLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "DOWN-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADDownRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "DOWN-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.DPADAdvancedMode = !(PrimaryGamepad.ButtonsStates.DPADUpLeft.KeyCode == 0 && PrimaryGamepad.ButtonsStates.DPADUpRight.KeyCode == 0 && PrimaryGamepad.ButtonsStates.DPADDownLeft.KeyCode == 0 && PrimaryGamepad.ButtonsStates.DPADDownRight.KeyCode == 0);

	PrimaryGamepad.ButtonsStates.Y.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "TRIANGLE-Y", "E"));
	PrimaryGamepad.ButtonsStates.X.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "SQUARE-X", "R"));
	PrimaryGamepad.ButtonsStates.A.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "CROSS-A", "SPACE"));
	PrimaryGamepad.ButtonsStates.B.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "CIRCLE-B", "CTRL"));

	PrimaryGamepad.ButtonsStates.LeftStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LS", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LS-UP", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LS-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LS-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.LeftStickDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LS-DOWN", "NONE"));
	
	PrimaryGamepad.ButtonsStates.RightStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RS", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RS-UP", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RS-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RS-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.RightStickDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RS-DOWN", "NONE"));

	PrimaryGamepad.KMEmu.LeftStickMode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "LS-MODE", "WASD"));
	PrimaryGamepad.KMEmu.RightStickMode = KeyNameToKeyCode(IniFile.ReadString("FIRST-GAMEPAD", "RS-MODE", "MOUSE-LOOK"));

	PrimaryGamepad.KMEmu.JoySensX = IniFile.ReadFloat("MOUSE", "SensitivityX", 100) * 0.21f; // Crysis 2, Last of Us 2 calibration
	PrimaryGamepad.KMEmu.JoySensY = IniFile.ReadFloat("MOUSE", "SensitivityY", 100) * 0.21f; // Crysis 2, Last of Us 2 calibration

	// Steering wheel
	PrimaryGamepad.KMEmu.SteeringWheelDeadZone = IniFile.ReadFloat("MOTION", "SteeringWheelDeadZone", 20) * 0.01f;
	PrimaryGamepad.KMEmu.SteeringWheelReleaseThreshold = IniFile.ReadFloat("MOTION", "SteeringWheelReleaseThreshold", 1) * 0.01f;

	// Motion wheel
	PrimaryGamepad.ButtonsStates.WheelActivationGamepadButton.KeyCode = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("MOTION", "WHEEL-ACTIVATION", "L2"));
	PrimaryGamepad.ButtonsStates.WheelDefault.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-DEFAULT", "0"));
	PrimaryGamepad.ButtonsStates.WheelUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP", "2"));
	PrimaryGamepad.ButtonsStates.WheelLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-LEFT", "1"));
	PrimaryGamepad.ButtonsStates.WheelRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-RIGHT", "3"));
	PrimaryGamepad.ButtonsStates.WheelDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN", "4"));

	PrimaryGamepad.ButtonsStates.WheelUpLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelUpRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelDownLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN-LEFT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelDownRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN-RIGHT", "NONE"));
	PrimaryGamepad.ButtonsStates.WheelAdvancedMode = !(PrimaryGamepad.ButtonsStates.WheelUpLeft.KeyCode == 0 && PrimaryGamepad.ButtonsStates.WheelUpRight.KeyCode == 0 && PrimaryGamepad.ButtonsStates.WheelDownLeft.KeyCode == 0 && PrimaryGamepad.ButtonsStates.WheelDownRight.KeyCode == 0);

	// Secondary gamepad
	SecondaryGamepad.ButtonsStates.LeftTrigger.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LT", "NONE"));
	SecondaryGamepad.ButtonsStates.RightTrigger.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RT", "NONE"));

	SecondaryGamepad.ButtonsStates.LeftBumper.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "L1-LB", "NONE"));
	SecondaryGamepad.ButtonsStates.RightBumper.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "R1-RB", "NONE"));

	SecondaryGamepad.ButtonsStates.Back.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "BACK", "NONE"));
	SecondaryGamepad.ButtonsStates.Start.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "START", "NONE"));

	SecondaryGamepad.ButtonsStates.DPADUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "UP", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LEFT", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RIGHT", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "DOWN", "NONE"));

	SecondaryGamepad.ButtonsStates.DPADUpLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "UP-LEFT", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADUpRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "UP-RIGHT", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADDownLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "DOWN-LEFT", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADDownRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "DOWN-RIGHT", "NONE"));
	SecondaryGamepad.ButtonsStates.DPADAdvancedMode = !(SecondaryGamepad.ButtonsStates.DPADUpLeft.KeyCode == 0 && SecondaryGamepad.ButtonsStates.DPADUpRight.KeyCode == 0 && SecondaryGamepad.ButtonsStates.DPADDownLeft.KeyCode == 0 && SecondaryGamepad.ButtonsStates.DPADDownRight.KeyCode == 0);

	SecondaryGamepad.ButtonsStates.Y.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "TRIANGLE-Y", "NONE"));
	SecondaryGamepad.ButtonsStates.X.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "SQUARE-X", "NONE"));
	SecondaryGamepad.ButtonsStates.A.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "CROSS-A", "NONE"));
	SecondaryGamepad.ButtonsStates.B.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "CIRCLE-B", "NONE"));

	SecondaryGamepad.ButtonsStates.LeftStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LS", "NONE"));
	SecondaryGamepad.ButtonsStates.LeftStickUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LS-UP", "NONE"));
	SecondaryGamepad.ButtonsStates.LeftStickLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LS-LEFT", "NONE"));
	SecondaryGamepad.ButtonsStates.LeftStickRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LS-RIGHT", "NONE"));
	SecondaryGamepad.ButtonsStates.LeftStickDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LS-DOWN", "NONE"));

	SecondaryGamepad.ButtonsStates.RightStick.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RS", "NONE"));
	SecondaryGamepad.ButtonsStates.RightStickUp.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RS-UP", "NONE"));
	SecondaryGamepad.ButtonsStates.RightStickLeft.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RS-LEFT", "NONE"));
	SecondaryGamepad.ButtonsStates.RightStickRight.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RS-RIGHT", "NONE"));
	SecondaryGamepad.ButtonsStates.RightStickDown.KeyCode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RS-DOWN", "NONE"));

	SecondaryGamepad.KMEmu.LeftStickMode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "LS-MODE", "NONE"));
	SecondaryGamepad.KMEmu.RightStickMode = KeyNameToKeyCode(IniFile.ReadString("SECOND-GAMEPAD", "RS-MODE", "NONE"));

	SecondaryGamepad.KMEmu.JoySensX = PrimaryGamepad.KMEmu.JoySensX;
	SecondaryGamepad.KMEmu.JoySensY = PrimaryGamepad.KMEmu.JoySensY;
}

void LoadXboxProfile(std::string ProfileFile) {
	CIniReader IniFile("XboxProfiles\\" + ProfileFile);

	CurrentXboxProfile.LeftBumper = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "LB", "LB"));
	CurrentXboxProfile.RightBumper = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RB", "RB"));
	//CurrentXboxProfile.LeftTrigger = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "LT", "LT"));
	//CurrentXboxProfile.RightTrigger = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RT", "RT"));
	CurrentXboxProfile.Back = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "BACK", "BACK"));
	CurrentXboxProfile.Start = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "START", "START"));
	CurrentXboxProfile.DPADUp = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "UP", "UP"));
	CurrentXboxProfile.DPADDown = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "DOWN", "DOWN"));
	CurrentXboxProfile.DPADLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "LEFT", "LEFT"));
	CurrentXboxProfile.DPADRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RIGHT", "RIGHT"));
	CurrentXboxProfile.Y = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "Y", "Y"));
	CurrentXboxProfile.X = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "X", "X"));
	CurrentXboxProfile.A = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "A", "A"));
	CurrentXboxProfile.B = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "B", "B"));
	CurrentXboxProfile.LeftStick = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "LS", "LS"));
	CurrentXboxProfile.RightStick = XboxKeyNameToXboxKeyCode(IniFile.ReadString("XBOX", "RS", "RS"));
	CurrentXboxProfile.SwapSticksAxis = IniFile.ReadBoolean("SETTINGS", "SWAP-STICKS", false);
	CurrentXboxProfile.SwapTriggers = IniFile.ReadBoolean("SETTINGS", "SWAP-TRIGGERS", false);

	// Motion wheel
	CurrentXboxProfile.WheelActivationButton = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("MOTION", "WHEEL-ACTIVATION", "L2"));
	CurrentXboxProfile.WheelDefault = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DEFAULT", "0"));
	CurrentXboxProfile.WheelUp = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP", "2"));
	CurrentXboxProfile.WheelLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-LEFT", "1"));
	CurrentXboxProfile.WheelRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-RIGHT", "3"));
	CurrentXboxProfile.WheelDown = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN", "4"));

	CurrentXboxProfile.WheelUpLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP-LEFT", "NONE"));
	CurrentXboxProfile.WheelUpRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-UP-RIGHT", "NONE"));
	CurrentXboxProfile.WheelDownLeft = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN-LEFT", "NONE"));
	CurrentXboxProfile.WheelDownRight = XboxKeyNameToXboxKeyCode(IniFile.ReadString("MOTION", "WHEEL-DOWN-RIGHT", "NONE"));
	CurrentXboxProfile.WheelAdvancedMode = !(CurrentXboxProfile.WheelUpLeft == 0 && CurrentXboxProfile.WheelUpRight == 0 && CurrentXboxProfile.WheelDownLeft == 0 && CurrentXboxProfile.WheelDownRight == 0);

	// Additional gamepad buttons
	CurrentXboxProfile.JCSL = XboxKeyNameToXboxKeyCode(IniFile.ReadString("JOYCONS", "SL", "NONE"));
	CurrentXboxProfile.JCSR = XboxKeyNameToXboxKeyCode(IniFile.ReadString("JOYCONS", "SR", "NONE"));
	CurrentXboxProfile.DSEdgeL4 = XboxKeyNameToXboxKeyCode(IniFile.ReadString("DUALSENSE-EDGE", "L4", "NONE"));
	CurrentXboxProfile.DSEdgeR4 = XboxKeyNameToXboxKeyCode(IniFile.ReadString("DUALSENSE-EDGE", "R4", "NONE"));
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
			default:
				break;
		}
		if (AppStatus.SecondaryGamepadEnabled) {
			if (SecondaryGamepad.DeviceIndex != -1) {
				printf(", ");
				switch (SecondaryGamepad.ControllerType) {
				case SONY_DUALSENSE:
					printf("Sony DualSense (simplified)");
					break;
				case SONY_DUALSHOCK4:
					printf("Sony DualShock 4 (simplified)");
					break;
				case NINTENDO_JOYCONS:
					printf("Nintendo Joy-Cons (");
					if (SecondaryGamepad.HidHandle != NULL && SecondaryGamepad.HidHandle2 != NULL) printf("left & right");
					else if (SecondaryGamepad.HidHandle != NULL) printf("left - limited input");
					else if (SecondaryGamepad.HidHandle2 != NULL) printf("right - limited input");
					printf(") (simplified)");
					break;
				case NINTENDO_SWITCH_PRO:
					printf("Nintendo Switch Pro Controller (simplified)");
					break;
				default:
					break;
				}
			}
		} else if (AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1)
			printf(", the second gamepad is disabled in the config");
		printf(".");
	}
	printf("\n Press \"CTRL + R\" or \"%s\" to reset/search for controllers, and \"ALT + V\" to swap the first and second ones.\n", AppStatus.HotKeys.ResetKeyName.c_str());
	if (AppStatus.ControllerCount > 0 && AppStatus.ShowBatteryStatus) {
		printf(" Controller 1");
		if (PrimaryGamepad.USBConnection) printf(" wired"); else printf(" wireless");
		if (PrimaryGamepad.ControllerType != NINTENDO_JOYCONS)
			printf(", battery charge: %d\%%", PrimaryGamepad.BatteryLevel);
		else {
			if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf(", battery charge: %d\%%, %d\%%", PrimaryGamepad.BatteryLevel, PrimaryGamepad.BatteryLevel2);
			else if (PrimaryGamepad.HidHandle != NULL) printf(", battery charge: %d\%%", PrimaryGamepad.BatteryLevel);
			else if (PrimaryGamepad.HidHandle2 != NULL) printf(", battery charge: %d\%%", PrimaryGamepad.BatteryLevel2);
		}
		if (PrimaryGamepad.BatteryMode == 0x2)
			printf(" (charging)", PrimaryGamepad.BatteryLevel);

		if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1) {
			printf(". Controller 2");
			if (SecondaryGamepad.USBConnection) printf(" wired"); else printf(" wireless");
			if (SecondaryGamepad.ControllerType != NINTENDO_JOYCONS)
				printf(", battery charge: %d\%%", SecondaryGamepad.BatteryLevel);
			else {
				if (SecondaryGamepad.HidHandle != NULL && SecondaryGamepad.HidHandle2 != NULL) printf(", battery level: %d\%%, %d\%%", SecondaryGamepad.BatteryLevel, SecondaryGamepad.BatteryLevel2);
				else if (SecondaryGamepad.HidHandle != NULL) printf(", battery level: %d\%%", PrimaryGamepad.BatteryLevel);
				else if (SecondaryGamepad.HidHandle2 != NULL) printf(", battery level: %d\%%", PrimaryGamepad.BatteryLevel2);
			}
			if (SecondaryGamepad.BatteryMode == 0x2)
				printf(" (charging)");
		}

		printf(".\n");
	}

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled)
		printf(" Emulation: Xbox gamepad, profile: \"%s\".\n Change profiles with \"ALT + Up/Down\" or \"PS/Home + DPAD Up/Down\".\n", XboxProfiles[XboxProfileIndex].substr(0, XboxProfiles[XboxProfileIndex].size() - 4).c_str());
	else if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)
		printf(" Emulation: Xbox gamepad (only driving) & mouse aiming.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled)
		printf(" Emulation: Only mouse (for mouse aiming).\n");
	else if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) {
		if (AppStatus.IsDesktopMode)
			printf(" Emulation: keyboard and mouse, desktop control, profile: \"Desktop\".\n");
		else
			printf_s(" Emulation: Keyboard and mouse, game profile: \"%s\".\n Change profiles with \"ALT + Up/Down\" or \"PS/Home + DPAD Up/Down\".\n", KMProfiles[KMProfileIndex].substr(0, KMProfiles[KMProfileIndex].size() - 4).c_str());
	}
	printf(" Press \"ALT + Q/Left/Right\", \"PS/Home + DPAD Left/Right\" to switch emulation.\n");
	printf(" Press touchpad areas or \"Capture/Home\" buttons to change operating modes.\n");
	printf(" If there's no touch panel, switch using a touchpad press (enabled in the config) or use \"ALT + 1/2\".\n");
	printf(" Pressing \"Home\" or \"ALT + 2\" again - switches aim mode (always/L2), \"Capture\" - resets.\n");
	if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) {
		printf(" Adaptive triggers mode: ");
		switch (PrimaryGamepad.AdaptiveTriggersMode) {
			case 0:
				printf("none");
				break;
			case 1:
				printf("dependent (driving/aiming - pistol)");
				break;
			case 2:
				printf("dependent (driving/aiming - automatic)");
				break;
			case 3:
				printf("dependent (driving/aiming - rifle)");
				break;
			case 4:
				printf("rumble translation");
				break;
			case 5:
				printf("pistol");
				break;
			case 6:
				printf("automatic");
				break;
			case 7:
				printf("rifle");
				break;
			case 8:
				printf("bow");
				break;
			case 9:
				printf("сar pedal");
				break;
			default:
				printf("unknown");
				break;
			}
		printf(". Press \"ALT + 3/4\" to switch.\n");
	}

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
		printf(" Left stick mode: default");
	else if (AppStatus.LeftStickMode == LeftStickAutoPressMode)
		printf(" Left stick mode: auto-press based on value");
	else if (AppStatus.LeftStickMode == LeftStickPressOnceMode)
		printf(" Left stick mode: single press based on value");
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
				else if (PrimaryGamepad.HidHandle != NULL) printf("левый - ограниченный ввод");
				else if (PrimaryGamepad.HidHandle2 != NULL) printf("правый - ограниченный ввод");
				printf(") (все функции)");
				break;
			case NINTENDO_SWITCH_PRO:
				printf("Nintendo Switch Pro Controller (все функции)");
				break;
			default:
				break;
		}
		if (AppStatus.SecondaryGamepadEnabled) {
			if (SecondaryGamepad.DeviceIndex != -1) {
				printf(", ");
				switch (SecondaryGamepad.ControllerType) {
					case SONY_DUALSENSE:
						printf("Sony DualSense (упрощённый)");
						break;
					case SONY_DUALSHOCK4:
						printf("Sony DualShock 4 (упрощённый)");
						break;
					case NINTENDO_JOYCONS:
						printf("Nintendo Joy-Cons (");
						if (SecondaryGamepad.HidHandle != NULL && SecondaryGamepad.HidHandle2 != NULL) printf("левый и правый");
						else if (SecondaryGamepad.HidHandle != NULL) printf("левый - недостаточно");
						else if (SecondaryGamepad.HidHandle2 != NULL) printf("правый - недостаточно");
						printf(") (упрощённый)");
						break;
					case NINTENDO_SWITCH_PRO:
						printf("Nintendo Switch Pro Controller (упрощённый)");
						break;
					default:
						break;
				}
			}
		} else if (AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1)
			printf(", второй геймпад отключен в конфиге");
		printf(".");
		//printf(". %d %d  Тип1: %d, Тип2: %d", PrimaryGamepad.DeviceIndex, SecondaryGamepad.DeviceIndex, PrimaryGamepad.ControllerType, SecondaryGamepad.ControllerType);
	}

	printf("\n Нажмите \"CTRL + R\" или \"%s\" для сброса/поиска контроллеров, а \"ALT + V\" для обмена первого и второго.\n", AppStatus.HotKeys.ResetKeyName.c_str());
	if (AppStatus.ControllerCount > 0 && AppStatus.ShowBatteryStatus) {
		printf(" Контроллер 1");
		if (PrimaryGamepad.USBConnection) printf(" проводной"); else printf(" беспроводной");
		if (PrimaryGamepad.ControllerType != NINTENDO_JOYCONS)
			printf(", заряд батареи: %d\%%", PrimaryGamepad.BatteryLevel);
		else {
			if (PrimaryGamepad.HidHandle != NULL && PrimaryGamepad.HidHandle2 != NULL) printf(", заряд батареи: %d\%%, %d\%%", PrimaryGamepad.BatteryLevel, PrimaryGamepad.BatteryLevel2);
			else if (PrimaryGamepad.HidHandle != NULL) printf(", заряд батареи: %d\%%", PrimaryGamepad.BatteryLevel);
			else if (PrimaryGamepad.HidHandle2 != NULL) printf(", заряд батареи: %d\%%", PrimaryGamepad.BatteryLevel2);
		}
		if (PrimaryGamepad.BatteryMode == 0x2)
			printf(" (зарядка)");

		if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1 && SecondaryGamepad.DeviceIndex != -1) {
			printf(". Контроллер 2");
			if (SecondaryGamepad.USBConnection) printf(" проводной"); else printf(" беспроводной");
			if (SecondaryGamepad.ControllerType != NINTENDO_JOYCONS)
				printf(", заряд батареи: %d\%%", SecondaryGamepad.BatteryLevel);
			else {
				if (SecondaryGamepad.HidHandle != NULL && SecondaryGamepad.HidHandle2 != NULL) printf(", заряд батареи: %d\%%, %d\%%", SecondaryGamepad.BatteryLevel, SecondaryGamepad.BatteryLevel2);
				else if (SecondaryGamepad.HidHandle != NULL) printf(", заряд батареи: %d\%%", PrimaryGamepad.BatteryLevel);
				else if (SecondaryGamepad.HidHandle2 != NULL) printf(", заряд батареи: %d\%%", PrimaryGamepad.BatteryLevel2);
			}
			if (SecondaryGamepad.BatteryMode == 0x2)
				printf(" (зарядка)");
		}
			
		printf(".\n");
	}

	if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled)
		printf(" Эмуляция: Xbox геймпад, профиль: \"%s\".\n Измените профиль, с помощью \"ALT + Up/Down\" или \"PS/Home + DPAD Up/Down\".\n", XboxProfiles[XboxProfileIndex].substr(0, XboxProfiles[XboxProfileIndex].size() - 4).c_str());
	else if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving)
		printf(" Эмуляция: Xbox геймпад (только вождение) и прицеливание мышкой.\n");
	else if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled)
		printf(" Эмуляция: только мышь (для прицеливания мышкой).\n");
	else if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) {
		if (AppStatus.IsDesktopMode)
			printf(" Эмуляция: клавиатура и мышь, управление рабочим столом, профиль: \"Desktop\".\n");
		else
			printf_s(" Эмуляция: клавиатура и мышь, профиль игры: \"%s\".\n Измените профиль, с помощью \"ALT + Up/Down\" или \"PS/Home + DPAD Up/Down\".\n", KMProfiles[KMProfileIndex].substr(0, KMProfiles[KMProfileIndex].size() - 4).c_str());
	}
	printf(" Нажмите \"ALT + Q/Влево/Вправо\" или \"PS/Home + DPAD Влево/Вправо\" для переключения режима эмуляции.\n");
	printf(" Нажмите области на сенсорной панели или кнопки \"Capture/Home\" для переключения режимов работы.\n");
	printf(" Если сенсорной панели нет, переключайтесь нажатием тачпада (включив в конфиге) или на \"ALT + 1/2\".\n");
	printf(" Повторное нажатие \"Home\" или \"ALT + 2\" - переключает режим прицеливания (всегда/L2), \"Capture\" - сброс.\n");
	if (PrimaryGamepad.ControllerType == SONY_DUALSENSE) {
		printf(" Режим адаптивных триггеров: ");
		switch (PrimaryGamepad.AdaptiveTriggersMode) {
			case 0:
				printf("нет");
				break;
			case 1:
				printf("зависимый (вождение/прицеливание - пистолет)");
				break;
			case 2:
				printf("зависимый (вождение/прицеливание - автомат)");
				break;
			case 3:
				printf("зависимый (вождение/прицеливание - винтовка)");
				break;
			case 4:
				printf("трансляция вибрации");
				break;
			case 5:
				printf("пистолет");
				break;
			case 6:
				printf("автомат");
				break;
			case 7:
				printf("винтовка");
				break;
			case 8:
				printf("лук");
				break;
			case 9:
				printf("педаль авто");
				break;
			default:
				printf("неизвестно");
				break;
			}
		printf(". Переключение на \"ALT + 3/4\".\n");
	}

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
	else if (AppStatus.LeftStickMode == LeftStickPressOnceMode)
		printf(" Режим левого стика: разовое нажатие по значению");
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

void SwapGamepads()
{
	std::swap(PrimaryGamepad.HidHandle, SecondaryGamepad.HidHandle);
	std::swap(PrimaryGamepad.HidHandle2, SecondaryGamepad.HidHandle2);
	std::swap(PrimaryGamepad.DeviceIndex, SecondaryGamepad.DeviceIndex);
	std::swap(PrimaryGamepad.DeviceIndex2, SecondaryGamepad.DeviceIndex2);
	std::swap(PrimaryGamepad.ControllerType, SecondaryGamepad.ControllerType);
	GamepadSetState(PrimaryGamepad);
	GamepadSetState(SecondaryGamepad);
	MainTextUpdate();
}

void SyncGamepadsWithJSL()
{
	struct JSL_SETTINGS JSLSecondaryGamepadInfo = JslGetControllerInfoAndSettings(PrimaryGamepad.DeviceIndex);
	//printf_s("1. %s\n2. %s \n", JSLSecondaryGamepadInfo.controllerPath.c_str(), PrimaryGamepad.DevicePath);
	if (!PrimaryGamepad.DevicePath.empty() && PrimaryGamepad.DevicePath != JSLSecondaryGamepadInfo.controllerPath.c_str()) {
		//printf("swaped\n");
		std::swap(PrimaryGamepad.DeviceIndex, SecondaryGamepad.DeviceIndex);
		std::swap(PrimaryGamepad.DeviceIndex2, SecondaryGamepad.DeviceIndex2);
	}
}

void RefreshDevices() {
	PrimaryGamepad.HidHandle = NULL;
	PrimaryGamepad.DeviceIndex = -1;
	PrimaryGamepad.DeviceIndex2 = -1;
	SecondaryGamepad.HidHandle = NULL;
	SecondaryGamepad.DeviceIndex = -1;
	SecondaryGamepad.DeviceIndex2 = -1;

	AppStatus.ControllerCount = JslConnectDevices();

	bool JoyconLeftFound = false;

	for (int i = 0; i < AppStatus.ControllerCount; i++) {
		// JslSetGyroSpace(i, 2);
		JslSetAutomaticCalibration(i, true); // Calibration all controllers

		int ControllerType = JslGetControllerType(i);
		if (ControllerType == JS_TYPE_DS || ControllerType == JS_TYPE_DS4 || ControllerType == JS_TYPE_JOYCON_LEFT || ControllerType == JS_TYPE_PRO_CONTROLLER) {
			
			if (PrimaryGamepad.DeviceIndex == -1)
				PrimaryGamepad.DeviceIndex = i;
			else if (SecondaryGamepad.DeviceIndex == -1)
				SecondaryGamepad.DeviceIndex = i;
			else // Only two controllers
				break;

			if (ControllerType == JS_TYPE_JOYCON_LEFT)
				JoyconLeftFound = true;
		}
	}

	// Right joycon (glue or self)
	for (int i = 0; i < AppStatus.ControllerCount; i++) {
		int ControllerType = JslGetControllerType(i);
		if (ControllerType != JS_TYPE_JOYCON_RIGHT) continue;

		if (JoyconLeftFound) {
			if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT)
				PrimaryGamepad.DeviceIndex2 = i;
			else
				SecondaryGamepad.DeviceIndex2 = i;
		} else {
			if (PrimaryGamepad.DeviceIndex == -1)
				PrimaryGamepad.DeviceIndex = i;
			else if (SecondaryGamepad.DeviceIndex == -1)
				SecondaryGamepad.DeviceIndex = i;
		}
	}

	// Find first gamepad
	GamepadSearch(PrimaryGamepad, "");
	GamepadSetState(PrimaryGamepad);

	if (AppStatus.SecondaryGamepadEnabled && AppStatus.ControllerCount > 1) {
		GamepadSearch(SecondaryGamepad, PrimaryGamepad.DevicePath);
		SyncGamepadsWithJSL();
		GamepadSetState(SecondaryGamepad);
	}

	if (AppStatus.ExternalPedalsDInputSearch)
		ExternalPedalsDInputSearch();

	AppStatus.BTReset = false;
	MainTextUpdate();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DEVICECHANGE: // The list of devices has changed
		if (wParam == DBT_DEVNODES_CHANGED) {
			RefreshDevices();
			if (!PrimaryGamepad.USBConnection || !SecondaryGamepad.USBConnection)
				AppStatus.BTReset = true; // Bug with Bluetooth controllers, in which in Input Bluetooth controllers random values (JoyShockLibarary?). Resetting again helps.
		}
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
	SetConsoleTitle("DSAdvance 2.0");
	WindowToCenter();

	// if (false)
	if (PRIMARYLANGID(GetUserDefaultLangID()) == LANG_RUSSIAN) { // Resave cpp file with UTF8 BOM
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
	PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;
	PrimaryGamepad.RumbleStrength = IniFile.ReadInteger("Gamepad", "RumbleStrength", 100);
	AppStatus.LockedChangeBrightness = IniFile.ReadBoolean("Gamepad", "LockChangeBrightness", false);
	AppStatus.ChangeModesWithClick = IniFile.ReadBoolean("Gamepad", "ChangeModesWithClick", true);
	AppStatus.ChangeModesWithoutAreas = IniFile.ReadBoolean("Gamepad", "ChangeModesWithoutAreas", false);
	AppStatus.JoyconChangeModesWithButton = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("Gamepad", "JoyconChangeModesWithButton", "NONE"));

	AppStatus.AimMode = IniFile.ReadBoolean("Motion", "AimingMode", AimMouseMode);
	AppStatus.AimingButton = SonyNintendoKeyNameToJoyShockKeyCode(IniFile.ReadString("Motion", "AimingButton", "L2"));
	AppStatus.JoyconRumbleMerge = IniFile.ReadBoolean("Gamepad", "JoyconRumbleMerge", false);

	PrimaryGamepad.Motion.SteeringWheelAngle = IniFile.ReadFloat("Motion", "SteeringWheelAngle", 150) / 2.0f;
	PrimaryGamepad.Motion.AircraftEnabled = IniFile.ReadBoolean("Motion", "AircraftEnabled", false);
	PrimaryGamepad.Motion.AircraftPitchAngle = IniFile.ReadFloat("Motion", "AircraftPitchAngle", 45) / 2.0f;
	PrimaryGamepad.Motion.AircraftPitchInverted = IniFile.ReadBoolean("Motion", "AircraftPitchInverted", false) ? -1 : 1;
	PrimaryGamepad.Motion.AircraftRollSens = IniFile.ReadFloat("Motion", "AircraftRollSens", 100) * 0.11875f;
	PrimaryGamepad.Motion.SensX = IniFile.ReadFloat("Motion", "MouseSensX", 100) * 0.005f;   // Calibration with Crysis 2, old 0.01
	PrimaryGamepad.Motion.SensY = IniFile.ReadFloat("Motion", "MouseSensY", 90) * 0.005f;
	PrimaryGamepad.Motion.SensAvg = (PrimaryGamepad.Motion.SensX + PrimaryGamepad.Motion.SensY) * 0.5f; // Sens Average
	PrimaryGamepad.Motion.JoySensX = IniFile.ReadFloat("Motion", "JoySensX", 100) * 0.0025f; // Calibration with Crysis 2, old 0.0013;
	PrimaryGamepad.Motion.JoySensY = IniFile.ReadFloat("Motion", "JoySensY", 90) * 0.0025f;  // Calibration with Crysis 2, old 0.0013;
	PrimaryGamepad.Motion.JoySensAvg = (PrimaryGamepad.Motion.JoySensX + PrimaryGamepad.Motion.JoySensY) * 0.5f; // Sens Average
	PrimaryGamepad.Motion.MotionWheelButtonsDeadZone = IniFile.ReadFloat("Motion", "MotionWheelButtonsDeadZone", 12.0f);

	PrimaryGamepad.DefaultModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DefaultModeColor", "0000ff"));
	PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;
	PrimaryGamepad.DrivingModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DrivingModeColor", "ff0000"));
	PrimaryGamepad.AimingModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "AimingModeColor", "00ff00"));
	PrimaryGamepad.AimingModeL2Color = WebColorToRGB(IniFile.ReadString("Gamepad", "AimingModeL2Color", "00ffff"));
	PrimaryGamepad.DesktopModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "DesktopModeColor", "ff00ff"));
	PrimaryGamepad.TouchSticksModeColor = WebColorToRGB(IniFile.ReadString("Gamepad", "TouchSticksModeColor", "ff00ff"));

	PrimaryGamepad.KMEmu.StickValuePressKey = IniFile.ReadFloat("KeyboardMouse", "StickValuePressKey", 0.2f);
	PrimaryGamepad.KMEmu.TriggerValuePressKey = IniFile.ReadFloat("KeyboardMouse", "TriggerValuePressKey", 0.2f);

	AppStatus.MicCustomKeyName = IniFile.ReadString("Gamepad", "MicCustomKey", "NONE");
	AppStatus.MicCustomKey = KeyNameToKeyCode(AppStatus.MicCustomKeyName);
	if (AppStatus.MicCustomKey == 0)
		AppStatus.ScreenshotMode = ScreenShotXboxGameBarMode; // If not set, then hide this mode
	else
		AppStatus.ScreenShotKey = AppStatus.MicCustomKey;
	AppStatus.SteamScrKeyName = IniFile.ReadString("Gamepad", "SteamScrKey", "NONE");
	AppStatus.SteamScrKey = KeyNameToKeyCode(AppStatus.SteamScrKeyName);

	AppStatus.SecondaryGamepadEnabled = IniFile.ReadBoolean("SecondaryGamepad", "Enabled", false);
	SecondaryGamepad.Sticks.DeadZoneLeftX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickX", 0);
	SecondaryGamepad.Sticks.DeadZoneLeftY = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftStickY", 0);
	SecondaryGamepad.Sticks.DeadZoneRightX = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightStickX", 0);
	SecondaryGamepad.Sticks.DeadZoneRightY = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightStickY", 0);
	SecondaryGamepad.Triggers.DeadZoneLeft = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneLeftTrigger", 0);

	SecondaryGamepad.Triggers.DeadZoneRight = IniFile.ReadFloat("SecondaryGamepad", "DeadZoneRightTrigger", 0);
	SecondaryGamepad.DefaultLEDBrightness = std::clamp((int)(255 - IniFile.ReadInteger("SecondaryGamepad", "DefaultBrightness", 100) * 2.55), 0, 255);
	SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
	SecondaryGamepad.DefaultModeColor = WebColorToRGB(IniFile.ReadString("SecondaryGamepad", "DefaultModeColor", "00ff00"));
	SecondaryGamepad.OutState.LEDColor = SecondaryGamepad.DefaultModeColor;

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

	// Search keyboard and mouse profiles
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile("KMProfiles\\*.ini", &ffd);
	KMProfiles.push_back("Desktop.ini");
	KMProfiles.push_back("FPS.ini");
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(ffd.cFileName, "Desktop.ini") && strcmp(ffd.cFileName, "FPS.ini")) // Already added to the top of the list
				KMProfiles.push_back(ffd.cFileName);
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	LoadKMProfile(KMProfiles[KMProfileIndex]); // Loading a standard keyboard and mouse profile

	// Search Xbox profiles
	hFind = FindFirstFile("XboxProfiles\\*.ini", &ffd);
	XboxProfiles.push_back("Default.ini");
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(ffd.cFileName, "Default.ini"))
				XboxProfiles.push_back(ffd.cFileName);
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	LoadXboxProfile(XboxProfiles[XboxProfileIndex]); // Loading a standard Xbox profile

	RefreshDevices();

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
	if (AppStatus.SecondaryGamepadEnabled) {
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
		if ((AppStatus.SkipPollCount == 0 && (IsKeyPressed(VK_CONTROL) && IsKeyPressed('R')) || IsKeyPressed(AppStatus.HotKeys.ResetKey)) || AppStatus.BTReset)
 		{
			RefreshDevices();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Swap gamepads
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('V') && AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			SwapGamepads();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		XUSB_REPORT_INIT(&report);
		if (AppStatus.SecondaryGamepadEnabled)
			XUSB_REPORT_INIT(&report2);

		if (AppStatus.ControllerCount < 1) { // We don't process anything during idle time
			report.sThumbLX = 1; // helps with crash, maybe power saving turns off the controller
			ret = vigem_target_x360_update(client, x360, report); // Vigem always mode only

			if (AppStatus.SecondaryGamepadEnabled) {
				report2.sThumbLX = 1;
				ret = vigem_target_x360_update(client2, x360, report2);
			}

			Sleep(AppStatus.SleepTimeOut);
			continue;
		}

		// Primary controller
		if (PrimaryGamepad.DeviceIndex2 == -1) {
			PrimaryGamepad.InputState = JslGetSimpleState(PrimaryGamepad.DeviceIndex);
			MotionState = JslGetMotionState(PrimaryGamepad.DeviceIndex);
			JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex, velocityX, velocityY, velocityZ);
		}
		else { // Split contoller (Joycons)
			PrimaryGamepad.InputState = JslGetSimpleState(PrimaryGamepad.DeviceIndex);
			JOY_SHOCK_STATE tempState = JslGetSimpleState(PrimaryGamepad.DeviceIndex2);
			MotionState = JslGetMotionState(PrimaryGamepad.DeviceIndex2);
			PrimaryGamepad.InputState.stickRX = tempState.stickRX;
			PrimaryGamepad.InputState.stickRY = tempState.stickRY;
			PrimaryGamepad.InputState.rTrigger = tempState.rTrigger;
			JslGetAndFlushAccumulatedGyro(PrimaryGamepad.DeviceIndex2, velocityX, velocityY, velocityZ);
			PrimaryGamepad.InputState.buttons |= tempState.buttons;
		}

		// Secondary controller
		if (SecondaryGamepad.DeviceIndex2 == -1) {
			SecondaryGamepad.InputState = JslGetSimpleState(SecondaryGamepad.DeviceIndex);
			//MotionState = JslGetMotionState(SecondaryGamepad.DeviceIndex);
			//JslGetAndFlushAccumulatedGyro(SecondaryGamepad.DeviceIndex, velocityX, velocityY, velocityZ);

		// Split contoller (Joycons)
		}
		else {
			SecondaryGamepad.InputState = JslGetSimpleState(SecondaryGamepad.DeviceIndex);
			JOY_SHOCK_STATE tempState = JslGetSimpleState(SecondaryGamepad.DeviceIndex2);
			//MotionState = JslGetMotionState(SecondaryGamepad.DeviceIndex2);
			SecondaryGamepad.InputState.stickRX = tempState.stickRX;
			SecondaryGamepad.InputState.stickRY = tempState.stickRY;
			SecondaryGamepad.InputState.rTrigger = tempState.rTrigger;
			SecondaryGamepad.InputState.buttons |= tempState.buttons;
			//JslGetAndFlushAccumulatedGyro(SecondaryGamepad.DeviceIndex2, velocityX, velocityY, velocityZ);
		}

		// Stick dead zones
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed(VK_F9) != 0)
		{
			AppStatus.DeadZoneMode = !AppStatus.DeadZoneMode;
			if (AppStatus.DeadZoneMode == false) MainTextUpdate(); else { system("cls"); printf("\n"); }
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}
		if (AppStatus.DeadZoneMode) {
			if (AppStatus.Lang == LANG_RUSSIAN) {
				printf(" Левый стик X=%.2f, ", abs(PrimaryGamepad.InputState.stickLX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickLY));
				printf("Правый стик X=%.2f, ", abs(PrimaryGamepad.InputState.stickRX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickRY));
				printf("Левый триггер=%.2f | ", abs(PrimaryGamepad.InputState.lTrigger));
				printf("Правый триггер=%.2f\n", abs(PrimaryGamepad.InputState.rTrigger));
			} else {
				printf(" Left stick X=%.2f, ", abs(PrimaryGamepad.InputState.stickLX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickLY));
				printf("Right stick X=%.2f, ", abs(PrimaryGamepad.InputState.stickRX));
				printf("Y=%.2f | ", abs(PrimaryGamepad.InputState.stickRY));
				printf("Left trigger=%.2f | ", abs(PrimaryGamepad.InputState.lTrigger));
				printf("Right trigger=%.2f \n", abs(PrimaryGamepad.InputState.rTrigger));
			}
		}

		// Switch emulation mode
		if (AppStatus.SkipPollCount == 0 && (
			(IsKeyPressed(VK_MENU) && (IsKeyPressed('Q') || IsKeyPressed(VK_LEFT) || IsKeyPressed(VK_RIGHT)) && 
			(!(GetConsoleWindow() != GetForegroundWindow() && AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse && AppStatus.IsDesktopMode == false)) ) || // Don't switch modes with keys when emulating keyboard and mouse for games (for games that use the ALT + ←/→ keys)
			((PrimaryGamepad.InputState.buttons & JSMASK_LEFT || PrimaryGamepad.InputState.buttons & JSMASK_RIGHT) && PrimaryGamepad.InputState.buttons & JSMASK_PS) // Switching modes always works on a gamepad.
			))
		{
			AppStatus.SkipPollCount = SkipPollTimeOut; // 30 for deatached, 15 is seems not enough to enable or disable Xbox virtual gamepad

			if (PrimaryGamepad.InputState.buttons & JSMASK_LEFT || IsKeyPressed(VK_LEFT)) {
				
				if (AppStatus.GamepadEmulationMode == 0) { // For desktop control mode 
					AppStatus.GamepadEmulationMode = EmuGamepadMaxModes;
					KMProfileIndex = 0;
					LoadKMProfile(KMProfiles[KMProfileIndex]);
					AppStatus.IsDesktopMode = true;
				
				} else if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) { // For keyboard and mouse game mode
					if (AppStatus.IsDesktopMode) {
						KMProfileIndex = KMGameProfileIndex;
						LoadKMProfile(KMProfiles[KMProfileIndex]);
						AppStatus.IsDesktopMode = false;
					}
					else
						AppStatus.GamepadEmulationMode--;

				} else
					AppStatus.GamepadEmulationMode--;
				
			
			} else if (PrimaryGamepad.InputState.buttons & JSMASK_RIGHT || IsKeyPressed(VK_RIGHT) || IsKeyPressed('Q')) {
				
				if (AppStatus.GamepadEmulationMode == EmuGamepadMaxModes) { // For keyboard and mouse game mode
					if (!AppStatus.IsDesktopMode) {
						KMProfileIndex = 0;
						LoadKMProfile(KMProfiles[KMProfileIndex]);
						AppStatus.IsDesktopMode = true;
					} else
						AppStatus.GamepadEmulationMode = 0;

				} else if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled) { // For desktop control mode 
					AppStatus.IsDesktopMode = false;
					KMProfileIndex = KMGameProfileIndex;
					LoadKMProfile(KMProfiles[KMProfileIndex]);
					AppStatus.GamepadEmulationMode++;

				} else
					AppStatus.GamepadEmulationMode++;
			}

			if (AppStatus.GamepadEmulationMode == EmuGamepadDisabled || AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) {
				if (AppStatus.XboxGamepadAttached) {
					//vigem_target_x360_unregister_notification(x360);
					//vigem_target_remove(client, x360);
					AppStatus.XboxGamepadAttached = false;
				}
			}
			else if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) {
				if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) AppStatus.AimMode = AimMouseMode;
				if (AppStatus.XboxGamepadAttached == false) {
					//ret = vigem_target_add(client, x360);
					//ret = vigem_target_x360_register_notification(client, x360, &notification, nullptr);
					AppStatus.XboxGamepadAttached = true;
				}
			}

			//if ( ( (LastGamepadEmuMode == EmuGamepadEnabled && AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving) || (LastGamepadEmuMode == EmuGamepadOnlyDriving && AppStatus.GamepadEmulationMode == EmuGamepadEnabled) ) ||
				 //( (LastGamepadEmuMode == EmuGamepadDisabled && AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) || (LastGamepadEmuMode == EmuKeyboardAndMouse && AppStatus.GamepadEmulationMode == EmuGamepadDisabled) ) )
			
			// Return color if Desktop Mode was selected on the touch panel
			if (!PrimaryGamepad.TouchSticksOn && PrimaryGamepad.SwitchedToDesktopMode) {
				PrimaryGamepad.SwitchedToDesktopMode = false;
				PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;
				GamepadSetState(PrimaryGamepad);
			}
			
			PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);

			MainTextUpdate();
		}

		// Switch aiming mode: mouse / joymouse
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('A')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_R) || (PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_R)))
		{
			AppStatus.AimMode = !AppStatus.AimMode;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Switch modes by pressing or touching
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) && IsKeyPressed('W')) ||
			((JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_SHARE))))
		{
			AppStatus.ChangeModesWithClick = !AppStatus.ChangeModesWithClick;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Switch left stick mode
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('S')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_LCLICK))) // PS - Home (Switch)
		{
			AppStatus.LeftStickMode++; if (AppStatus.LeftStickMode > LeftStickMaxModes) AppStatus.LeftStickMode = LeftStickDefaultMode;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Switch screenshot mode
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('X'))
		{
			AppStatus.ScreenshotMode++; if (AppStatus.ScreenshotMode > ScreenShotMaxModes) AppStatus.ScreenshotMode = AppStatus.MicCustomKey == 0 ? ScreenShotXboxGameBarMode : ScreenShotCustomKeyMode;
			if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode) AppStatus.ScreenShotKey = AppStatus.MicCustomKey;
			else if (AppStatus.ScreenshotMode == ScreenShotXboxGameBarMode) AppStatus.ScreenShotKey = VK_GAMEBAR_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotSteamMode) AppStatus.ScreenShotKey = VK_STEAM_SCREENSHOT;
			else if (AppStatus.ScreenshotMode == ScreenShotMultiMode) AppStatus.ScreenShotKey = VK_MULTI_SCREENSHOT;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Enable or disable lightbar
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) && IsKeyPressed('B')) || (PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_L)))
		{
			if (PrimaryGamepad.OutState.LEDBrightness == 255) PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness;
			else {
				if (AppStatus.LockedChangeBrightness == false && PrimaryGamepad.OutState.LEDBrightness > 4) // 5 is the minimum brightness
					PrimaryGamepad.DefaultLEDBrightness = PrimaryGamepad.OutState.LEDBrightness; // Save the new selected value as default
				PrimaryGamepad.OutState.LEDBrightness = 255;
			}
			GamepadSetState(PrimaryGamepad);
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Switch keyboard and mouse profile
		if (AppStatus.SkipPollCount == 0 && (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse || AppStatus.GamepadEmulationMode == EmuGamepadEnabled))
			if ((PrimaryGamepad.InputState.buttons & JSMASK_PS && (PrimaryGamepad.InputState.buttons & JSMASK_UP || PrimaryGamepad.InputState.buttons & JSMASK_DOWN)) ||
				((IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_UP) || IsKeyPressed(VK_DOWN))) && GetConsoleWindow() == GetForegroundWindow()))
			{
				AppStatus.SkipPollCount = SkipPollTimeOut;
				if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled) {
					if (IsKeyPressed(VK_UP) || PrimaryGamepad.InputState.buttons & JSMASK_UP) if (XboxProfileIndex > 0) XboxProfileIndex--; else XboxProfileIndex = XboxProfiles.size() - 1;
					if (IsKeyPressed(VK_DOWN) || PrimaryGamepad.InputState.buttons & JSMASK_DOWN) if (XboxProfileIndex < XboxProfiles.size() - 1) XboxProfileIndex++; else XboxProfileIndex = 0;
					LoadXboxProfile(XboxProfiles[XboxProfileIndex]);

				} else {
					if (!AppStatus.IsDesktopMode) { // EmuKeyboardAndMouse game mode
						if (IsKeyPressed(VK_UP) || PrimaryGamepad.InputState.buttons & JSMASK_UP) if (KMProfileIndex > 0) KMProfileIndex--; else KMProfileIndex = KMProfiles.size() - 1;
						if (IsKeyPressed(VK_DOWN) || PrimaryGamepad.InputState.buttons & JSMASK_DOWN) if (KMProfileIndex < KMProfiles.size() - 1) KMProfileIndex++; else KMProfileIndex = 0;
					}
					LoadKMProfile(KMProfiles[KMProfileIndex]);
					KMGameProfileIndex = KMProfileIndex;
				}
				
				MainTextUpdate();
				PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
			}

		// Switch external pedals mode
		if (AppStatus.SkipPollCount == 0 && ((IsKeyPressed(VK_MENU) != 0 && IsKeyPressed('E'))))
		{
			if (AppStatus.ExternalPedalsMode == ExPedalsAlwaysRacing)
				AppStatus.ExternalPedalsMode = ExPedalsDependentMode;
			else
				AppStatus.ExternalPedalsMode = ExPedalsAlwaysRacing;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Changing the Rumble strength
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && (IsKeyPressed(VK_OEM_COMMA) || IsKeyPressed(VK_OEM_PERIOD)))
		{
			if (IsKeyPressed(VK_OEM_COMMA) && PrimaryGamepad.RumbleStrength > 0)
				PrimaryGamepad.RumbleStrength -= 10;
			if (IsKeyPressed(VK_OEM_PERIOD) && PrimaryGamepad.RumbleStrength < 100)
				PrimaryGamepad.RumbleStrength += 10;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Switch adaptive triigers mode
		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && ((IsKeyPressed('3') || IsKeyPressed('4'))))
		{
			if (IsKeyPressed('3')) {
				PrimaryGamepad.AdaptiveTriggersMode++;
				if (PrimaryGamepad.AdaptiveTriggersMode > ADAPTIVE_TRIGGERS_MODE_MAX)
					PrimaryGamepad.AdaptiveTriggersMode = 0;
			}
			else { //if (IsKeyPressed('4'))
				PrimaryGamepad.AdaptiveTriggersMode--;
				if (PrimaryGamepad.AdaptiveTriggersMode < 0)
					PrimaryGamepad.AdaptiveTriggersMode = ADAPTIVE_TRIGGERS_MODE_MAX;
			}
			if (PrimaryGamepad.AdaptiveTriggersMode > 3)
				PrimaryGamepad.AdaptiveTriggersOutputMode = PrimaryGamepad.AdaptiveTriggersMode - 3; // Output skips "dependent" - "- 3"
			else if (PrimaryGamepad.AdaptiveTriggersMode == 0)
				PrimaryGamepad.AdaptiveTriggersOutputMode = 0;
			else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1)
					PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_PISTOL_MODE;
				else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2)
					PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_AUTOMATIC_MODE;
				else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
					PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_RIFLE_MODE;
			GamepadSetState(PrimaryGamepad);
			
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		bool IsCombinedRumbleChange = false;
		if ((JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS))
			IsCombinedRumbleChange = true;
		if ((JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER) &&
			(PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_PLUS))
			IsCombinedRumbleChange = true;
		if (AppStatus.SkipPollCount == 0 && IsCombinedRumbleChange) {
			if (PrimaryGamepad.RumbleStrength == 100)
				PrimaryGamepad.RumbleStrength = 0;
			else
				PrimaryGamepad.RumbleStrength += 10;
			MainTextUpdate();
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}

		// Switch modes by touchpad
		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4)

			// Regular controllers with touchpads
			if (AppStatus.ChangeModesWithoutAreas == false) {

				TouchState = JslGetTouchState(PrimaryGamepad.DeviceIndex);

				if (AppStatus.LockChangeBrightness == false && TouchState.t0Down && TouchState.t0Y <= 0.1 && TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA) { // Brightness change
					PrimaryGamepad.OutState.LEDBrightness = 255 - std::clamp((int)((TouchState.t0X - TOUCHPAD_LEFT_AREA - 0.020) * 255 * 4), 0, 255);
					//printf("%5.2f %d\n", (TouchState.t0X - TOUCHPAD_LEFT_AREA - 0.020) * 255 * 4, PrimaryGamepad.GamepadOutState.LEDBrightness);
					GamepadSetState(PrimaryGamepad);
				}

				if ((PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK && AppStatus.ChangeModesWithClick) || (TouchState.t0Down && AppStatus.ChangeModesWithClick == false)) {

					// [O--] - Driving mode
					if (TouchState.t0X > 0 && TouchState.t0X <= TOUCHPAD_LEFT_AREA && PrimaryGamepad.GamepadActionMode != TouchpadSticksMode) {
						PrimaryGamepad.GamepadActionMode = MotionDrivingMode;

						PrimaryGamepad.Motion.OffsetAxisX = atan2f(MotionState.gravX, MotionState.gravZ);
						PrimaryGamepad.Motion.OffsetAxisY = atan2f(MotionState.gravY, MotionState.gravZ);

						PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DrivingModeColor;

						if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_CAR_MODE;

					// [-O-] // Default & touch sticks modes
					} else if (TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA) {

						// Brightness area
						if (TouchState.t0Y <= 0.1) {

							if (AppStatus.SkipPollCount == 0) {
								AppStatus.BrightnessAreaPressed++;
								if (AppStatus.BrightnessAreaPressed > 1) {
									if (AppStatus.LockedChangeBrightness) {
										if (PrimaryGamepad.OutState.LEDBrightness == 255) PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.DefaultLEDBrightness; else PrimaryGamepad.OutState.LEDBrightness = 255;
									}
									else
										AppStatus.LockChangeBrightness = !AppStatus.LockChangeBrightness;
									AppStatus.BrightnessAreaPressed = 0;
								}
								AppStatus.SkipPollCount = SkipPollTimeOut;
							}
							PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;

						// Default mode
						} else if (TouchState.t0Y > 0.1 && TouchState.t0Y < 0.7) {
							PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
							// Show battery level
							ShowBatteryLevels();
							AppStatus.ShowBatteryStatus = true;
							if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
								GamepadSetState(SecondaryGamepad);
							MainTextUpdate();
							//printf(" %d %d\n", PrimaryGamepad.LastLEDBrightness, PrimaryGamepad.GamepadOutState.LEDBrightness);

							if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
								PrimaryGamepad.AdaptiveTriggersOutputMode = 0;

						// Desktop / Touch sticks mode
						} else {
							if (PrimaryGamepad.TouchSticksOn) {
								PrimaryGamepad.GamepadActionMode = TouchpadSticksMode;
								PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.TouchSticksModeColor;

							} else if (!PrimaryGamepad.SwitchedToDesktopMode && AppStatus.SkipPollCount == 0) {
								PrimaryGamepad.GamepadActionMode = DesktopMode;
								PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DesktopModeColor;
								if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse)
									AppStatus.LastGamepadEmulationMode = AppStatus.GamepadEmulationMode;
								AppStatus.GamepadEmulationMode = EmuKeyboardAndMouse;

								KMProfileIndex = 0;
								LoadKMProfile(KMProfiles[0]); // First profile Desktop.ini
								PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
								PrimaryGamepad.SwitchedToDesktopMode = true;
								AppStatus.IsDesktopMode = true;
								MainTextUpdate();
								AppStatus.SkipPollCount = SkipPollTimeOut;
								//printf("Desktop turn on\n");
							}
							
							if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2 || PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
								PrimaryGamepad.AdaptiveTriggersOutputMode = 0;
						}

					// [--O] Aiming mode
					} else if (TouchState.t0X > TOUCHPAD_RIGHT_AREA && TouchState.t0X <= 1 && PrimaryGamepad.GamepadActionMode != TouchpadSticksMode) {

						// Switch motion aiming mode
						if (AppStatus.SkipPollCount == 0 && TouchState.t0Y < 0.3) {
							PrimaryGamepad.GamepadActionMode = PrimaryGamepad.GamepadActionMode != MotionAimingMode ? MotionAimingMode : MotionAimingModeOnlyPressed;
							PrimaryGamepad.LastMotionAIMMode = PrimaryGamepad.GamepadActionMode;
							AppStatus.SkipPollCount = SkipPollTimeOut;
						}

						// Motion aiming
						if (TouchState.t0Y >= 0.3 && TouchState.t0Y <= 1) {
							PrimaryGamepad.GamepadActionMode = PrimaryGamepad.LastMotionAIMMode;
							PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.AimingModeColor;
						}

						PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.GamepadActionMode == MotionAimingMode ? PrimaryGamepad.AimingModeColor : PrimaryGamepad.AimingModeL2Color;

						if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_1)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_PISTOL_MODE;
						else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_2)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_AUTOMATIC_MODE;
						else if (PrimaryGamepad.AdaptiveTriggersMode == ADAPTIVE_TRIGGERS_DEPENDENT_MODE_3)
							PrimaryGamepad.AdaptiveTriggersOutputMode = ADAPTIVE_TRIGGERS_RIFLE_MODE;
					}

					// Reset desktop mode and return action mode
					if (!PrimaryGamepad.TouchSticksOn && PrimaryGamepad.GamepadActionMode != DesktopMode && PrimaryGamepad.SwitchedToDesktopMode) {
						AppStatus.GamepadEmulationMode = AppStatus.LastGamepadEmulationMode;
						MainTextUpdate();
						PlaySound(ChangeEmuModeWav, NULL, SND_ASYNC);
						PrimaryGamepad.SwitchedToDesktopMode = false;
						AppStatus.IsDesktopMode = false;
						//printf("Desktop turn off\n");
					}

					// Reset lock brightness if clicked in another area
					if (!(TouchState.t0Y <= 0.1 && TouchState.t0X > TOUCHPAD_LEFT_AREA && TouchState.t0X < TOUCHPAD_RIGHT_AREA)) {
						AppStatus.BrightnessAreaPressed = 0;
						if (AppStatus.LockChangeBrightness == false) AppStatus.LockChangeBrightness = true;
					}

					GamepadSetState(PrimaryGamepad);
					//printf("current mode = %d\r\n", PrimaryGamepad.GamepadActionMode);
					if (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && PrimaryGamepad.GamepadActionMode != MotionDrivingMode) AppStatus.XboxGamepadReset = true; // Reset last state
				}

				// Controllers without touchpads (AppStatus.ChangeModesWithoutAreas == true)
			}
			else if ((PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) && AppStatus.SkipPollCount == 0) {

				if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

					if (PrimaryGamepad.GamepadActionMode == MotionAimingMode)
					{
						PrimaryGamepad.GamepadActionMode = MotionAimingModeOnlyPressed; PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
					} else {
						PrimaryGamepad.GamepadActionMode = MotionAimingMode; PrimaryGamepad.LastMotionAIMMode = MotionAimingMode;
					}

				} else
					PrimaryGamepad.GamepadActionMode = MotionDrivingMode;

				AppStatus.SkipPollCount = SkipPollTimeOut;
			}

		if (AppStatus.SkipPollCount == 0 && IsKeyPressed(VK_MENU) && IsKeyPressed('I') && GetConsoleWindow() == GetForegroundWindow())
		{
			AppStatus.SkipPollCount = SkipPollTimeOut;
			ShowBatteryLevels();
			GamepadSetState(PrimaryGamepad);
			if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
				GamepadSetState(SecondaryGamepad);
			/*GetBatteryInfo(); if (AppStatus.BackOutStateCounter == 0) AppStatus.BackOutStateCounter = 40; // ↑
			if (AppStatus.BackOutStateCounter == 40) {
				PrimaryGamepad.LastLEDBrightness = PrimaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
				if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1)
					SecondaryGamepad.LastLEDBrightness = SecondaryGamepad.OutState.LEDBrightness; // Save on first click (tick)
			}*/
			AppStatus.ShowBatteryStatus = true;
			MainTextUpdate();
		}

		//printf("%5.2f\t%5.2f\r\n", PrimaryGamepad.InputState.stickLX, DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX));
		report.sThumbLX = PrimaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX) * 32767;
		report.sThumbLY = PrimaryGamepad.Sticks.InvertLeftX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY) * 32767;
		report.sThumbRX = PrimaryGamepad.Sticks.InvertRightX == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX) * 32767;
		report.sThumbRY = PrimaryGamepad.Sticks.InvertRightY == false ? DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY) * 32767 : DeadZoneAxis(-PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY) * 32767;

		if (CurrentXboxProfile.SwapSticksAxis) {
			std::swap(report.sThumbLX, report.sThumbRX);
			std::swap(report.sThumbLY, report.sThumbRY);
		}

		// Auto stick pressing when value is exceeded
		if (PrimaryGamepad.GamepadActionMode != MotionDrivingMode) { // Exclude driving mode
			if (AppStatus.LeftStickMode != LeftStickDefaultMode && (sqrt(PrimaryGamepad.InputState.stickLX * PrimaryGamepad.InputState.stickLX + PrimaryGamepad.InputState.stickLY * PrimaryGamepad.InputState.stickLY) >= PrimaryGamepad.AutoPressStickValue)) {
				if (AppStatus.LeftStickMode == LeftStickAutoPressMode)
					report.wButtons |= JSMASK_LCLICK;
				else { // LeftStickPressOnceMode
					if (AppStatus.LeftStickPressOnce == false) {
						report.wButtons |= JSMASK_LCLICK;
						AppStatus.LeftStickPressOnce = true;
						//printf(" LeftStickPressOnce\n");
					}
				}
			} else
				AppStatus.LeftStickPressOnce = false;
		}

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

				// ExPedalsModeDependent
				} else { 
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
						}
						else {
							if (AppStatus.ExternalPedalsXboxModePedal1 == XINPUT_GAMEPAD_LEFT_TRIGGER) {
								if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
									report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
							}
							else {
								if (DeadZoneAxis(PrimaryGamepad.InputState.rTrigger, PrimaryGamepad.Triggers.DeadZoneRight) == 0)
									report.bRightTrigger = AppStatus.ExternalPedalsJoyInfo.dwVpos / 256;
							}
						}

						// Pedal 2
						if (!AppStatus.ExternalPedalsXboxModePedal2Analog) { // Pedal 2 button
							if (AppStatus.ExternalPedalsJoyInfo.dwUpos > AppStatus.ExternalPedalsValuePress)
								if ((report.wButtons & AppStatus.ExternalPedalsXboxModePedal2) == 0)
									report.wButtons |= AppStatus.ExternalPedalsXboxModePedal2;
						}
						else {
							if (AppStatus.ExternalPedalsXboxModePedal2 == XINPUT_GAMEPAD_LEFT_TRIGGER) {
								if (DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) == 0)
									report.bLeftTrigger = AppStatus.ExternalPedalsJoyInfo.dwUpos / 256;
							}
							else {
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

		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_DS4) {
			if (!(PrimaryGamepad.InputState.buttons & JSMASK_PS)) {
				report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_SHARE ? CurrentXboxProfile.Back : 0;
				report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS ? CurrentXboxProfile.Start : 0;
			}
		} else if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_MINUS ? CurrentXboxProfile.Back : 0;
			report.wButtons |= PrimaryGamepad.InputState.buttons & JSMASK_PLUS ? CurrentXboxProfile.Start : 0;
		}

		if (!(PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE)) { // During special functions, nothing is pressed in the game
			DWORD XboxButtons = report.wButtons;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_L ? CurrentXboxProfile.LeftBumper : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_R ? CurrentXboxProfile.RightBumper : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_LCLICK ? CurrentXboxProfile.LeftStick : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_RCLICK ? CurrentXboxProfile.RightStick : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_UP ? CurrentXboxProfile.DPADUp : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_DOWN ? CurrentXboxProfile.DPADDown : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_LEFT ? CurrentXboxProfile.DPADLeft : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_RIGHT ? CurrentXboxProfile.DPADRight : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_N ? CurrentXboxProfile.Y : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_W ? CurrentXboxProfile.X : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_S ? CurrentXboxProfile.A : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_E ? CurrentXboxProfile.B : 0;

			// Aditional buttons
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_SL ? CurrentXboxProfile.JCSL : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_SR ? CurrentXboxProfile.JCSR : 0;

			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_FNL ? CurrentXboxProfile.DSEdgeL4 : 0;
			XboxButtons |= PrimaryGamepad.InputState.buttons & JSMASK_FNR ? CurrentXboxProfile.DSEdgeR4 : 0;

			// Custom keys
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_TRIGGER) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_TRIGGER; report.bLeftTrigger = 255; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_TRIGGER) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_TRIGGER; report.bRightTrigger = 255; }

			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_UP) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_UP; report.sThumbLY = 32767; }
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_DOWN) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_DOWN; report.sThumbLY = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_LEFT) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_LEFT; report.sThumbLX = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_LEFT_STICK_RIGHT) { XboxButtons &= ~XINPUT_GAMEPAD_LEFT_STICK_RIGHT; report.sThumbLX = 32767; }

			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_UP) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_UP; report.sThumbRY = 32767; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_DOWN) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_DOWN; report.sThumbRY = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_LEFT) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_LEFT; report.sThumbRX = -32767; }
			if (XboxButtons & XINPUT_GAMEPAD_RIGHT_STICK_RIGHT) { XboxButtons &= ~XINPUT_GAMEPAD_RIGHT_STICK_RIGHT; report.sThumbRX = 32767; }

			if (CurrentXboxProfile.SwapTriggers)
				std::swap(report.bLeftTrigger, report.bRightTrigger);

			report.wButtons = (WORD)XboxButtons;
		}



		// Motion wheel
		if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {
			if (CurrentXboxProfile.WheelActivationButton != 0 && PrimaryGamepad.InputState.buttons & CurrentXboxProfile.WheelActivationButton && PrimaryGamepad.Motion.WheelCounter == 0) {
				if (report.wButtons & CurrentXboxProfile.WheelActivationButton)
					report.wButtons &= ~CurrentXboxProfile.WheelDefault;
				//printf("start\n");
				PrimaryGamepad.Motion.WheelCounter = SkipPollTimeOut;
				PrimaryGamepad.Motion.WheelActive = true;
				PrimaryGamepad.Motion.WheelAccumX = 0.0f;
				PrimaryGamepad.Motion.WheelAccumY = 0.0f;
			}

			if ((PrimaryGamepad.InputState.buttons & CurrentXboxProfile.WheelActivationButton) == false && PrimaryGamepad.Motion.WheelCounter > 0) {

				if (PrimaryGamepad.Motion.WheelCounter == 1) {

					float MotionLength = sqrtf(PrimaryGamepad.Motion.WheelAccumX * PrimaryGamepad.Motion.WheelAccumX + PrimaryGamepad.Motion.WheelAccumY * PrimaryGamepad.Motion.WheelAccumY);

					if (MotionLength < PrimaryGamepad.Motion.MotionWheelButtonsDeadZone) {
						report.wButtons |= CurrentXboxProfile.WheelDefault;
					}
					else {
						float MotionWheelAngle = atan2f(PrimaryGamepad.Motion.WheelAccumY, PrimaryGamepad.Motion.WheelAccumX) * 57.29578f;

						if (CurrentXboxProfile.WheelAdvancedMode == false) { // Regular mode  ↑ → ↓ ←

							if (fabs(MotionWheelAngle) < 25.f) {
								report.wButtons |= CurrentXboxProfile.WheelUp;
							}
							else if (MotionWheelAngle > 45.f && MotionWheelAngle < 135.f) {
								report.wButtons |= CurrentXboxProfile.WheelLeft;
							}
							else if (MotionWheelAngle < -45.f && MotionWheelAngle > -135.f) {
								report.wButtons |= CurrentXboxProfile.WheelRight;
							}
							else if (MotionWheelAngle > 165.f || MotionWheelAngle < -165.f) {
								report.wButtons |= CurrentXboxProfile.WheelDown;
							}

						}
						else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖
							if (MotionWheelAngle >= -22.5f && MotionWheelAngle < 22.5f) {
								report.wButtons |= CurrentXboxProfile.WheelUp;
							}
							else if (MotionWheelAngle >= 22.5f && MotionWheelAngle < 67.5f) {
								report.wButtons |= CurrentXboxProfile.WheelUpLeft;
							}
							else if (MotionWheelAngle >= 67.5f && MotionWheelAngle < 112.5f) {
								report.wButtons |= CurrentXboxProfile.WheelLeft;
							}
							else if (MotionWheelAngle >= 112.5f && MotionWheelAngle < 157.5f) {
								report.wButtons |= CurrentXboxProfile.WheelDownLeft;
							}
							else if (MotionWheelAngle >= 157.5f || MotionWheelAngle < -157.5f) {
								report.wButtons |= CurrentXboxProfile.WheelDown;
							}
							else if (MotionWheelAngle >= -157.5f && MotionWheelAngle < -112.5f) {
								report.wButtons |= CurrentXboxProfile.WheelDownRight;
							}
							else if (MotionWheelAngle >= -112.5f && MotionWheelAngle < -67.5f) {
								report.wButtons |= CurrentXboxProfile.WheelRight;
							}
							else if (MotionWheelAngle >= -67.5f && MotionWheelAngle < -22.5f) {
								report.wButtons |= CurrentXboxProfile.WheelUpRight;
							}
						}

					}

					PrimaryGamepad.Motion.WheelActive = false;
				}

				PrimaryGamepad.Motion.WheelCounter--;
			}

			if (PrimaryGamepad.Motion.WheelActive) {
				PrimaryGamepad.Motion.WheelAccumX += velocityX * FrameTime;
				PrimaryGamepad.Motion.WheelAccumY += velocityY * FrameTime;
			}
		}

		// Nintendo controllers buttons: Capture & Home - changing working mode + another controllers (with additional buttons with keyboard emulation)
		if ((IsKeyPressed(VK_MENU) && IsKeyPressed('1')) || (IsKeyPressed(VK_MENU) && IsKeyPressed('2')) ||
			JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER ||
			JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT ||
			JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {

			if (AppStatus.SkipPollCount == 0 && ((PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && AppStatus.JoyconChangeModesWithButton == 0) || (IsKeyPressed(VK_MENU) && IsKeyPressed('1')))) {
				if (PrimaryGamepad.GamepadActionMode == 1)
					PrimaryGamepad.GamepadActionMode = GamepadDefaultMode;
				else
					PrimaryGamepad.GamepadActionMode = MotionDrivingMode;
				AppStatus.SkipPollCount = SkipPollTimeOut;
			}

			if (AppStatus.SkipPollCount == 0 && ((PrimaryGamepad.InputState.buttons & JSMASK_HOME && AppStatus.JoyconChangeModesWithButton == 0) || (IsKeyPressed(VK_MENU) && IsKeyPressed('2')))) {
				if (PrimaryGamepad.GamepadActionMode == GamepadDefaultMode || PrimaryGamepad.GamepadActionMode == MotionDrivingMode)
					PrimaryGamepad.GamepadActionMode = PrimaryGamepad.LastMotionAIMMode;
				else if (PrimaryGamepad.GamepadActionMode == MotionAimingMode)
				{
					PrimaryGamepad.GamepadActionMode = MotionAimingModeOnlyPressed; PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
				}
				else { PrimaryGamepad.GamepadActionMode = MotionAimingMode; PrimaryGamepad.LastMotionAIMMode = MotionAimingMode; }
				AppStatus.SkipPollCount = SkipPollTimeOut;
			}

			// Change modes on one Joycon
			if (AppStatus.SkipPollCount == 0 && AppStatus.JoyconChangeModesWithButton != 0 && (PrimaryGamepad.InputState.buttons & AppStatus.JoyconChangeModesWithButton)) {
				if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

					if (PrimaryGamepad.GamepadActionMode == MotionAimingMode)
					{
						PrimaryGamepad.GamepadActionMode = MotionAimingModeOnlyPressed; PrimaryGamepad.LastMotionAIMMode = MotionAimingModeOnlyPressed;
					}
					else {
						PrimaryGamepad.GamepadActionMode = MotionAimingMode; PrimaryGamepad.LastMotionAIMMode = MotionAimingMode;
					}

				} else
					PrimaryGamepad.GamepadActionMode = MotionDrivingMode;

				AppStatus.SkipPollCount = SkipPollTimeOut;
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

		KeyPress(VK_GAMEBAR, (PrimaryGamepad.PSOnlyCheckCount == 1 && PrimaryGamepad.PSOnlyPressed) || (PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_HOME), &PrimaryGamepad.ButtonsStates.PS, false);

		if (JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(PrimaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
			KeyPress(VK_VOLUME_DOWN2, PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_W, &PrimaryGamepad.ButtonsStates.VolumeDown, false);
			KeyPress(VK_VOLUME_UP2, PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE && PrimaryGamepad.InputState.buttons & JSMASK_E, &PrimaryGamepad.ButtonsStates.VolumeUp, false);
		}
		else {
			KeyPress(VK_VOLUME_DOWN2, PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_W, &PrimaryGamepad.ButtonsStates.VolumeDown, false);
			KeyPress(VK_VOLUME_UP2, PrimaryGamepad.InputState.buttons & JSMASK_PS && PrimaryGamepad.InputState.buttons & JSMASK_E, &PrimaryGamepad.ButtonsStates.VolumeUp, false);
		}

		// Screenshot / record key
		bool IsSharePressed = PrimaryGamepad.InputState.buttons & JSMASK_MIC || ((PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_S); // + DualShock 4 & Nintendo
		bool IsScreenshotPressed = false;
		bool IsRecordPressed = false;

		// Microphone (screenshots / record video) detect / Определение нажатия скриншота / записи видео
		if (AppStatus.ScreenshotMode != ScreenShotCustomKeyMode) {

			if (IsSharePressed && PrimaryGamepad.ShareHandled == false && PrimaryGamepad.ShareOnlyCheckCount == 0) {
				PrimaryGamepad.ShareHandled = true;
				PrimaryGamepad.ShareOnlyCheckCount = 20;
				PrimaryGamepad.ShareCheckUnpressed = false;
				// printf(" Start\n");
			}

			if (PrimaryGamepad.ShareOnlyCheckCount > 0) { // Checking start
				if (IsSharePressed == false) {
					PrimaryGamepad.ShareCheckUnpressed = true;
					PrimaryGamepad.ShareOnlyCheckCount = 1; // Skip timeout if button is released
				}

				if (PrimaryGamepad.ShareOnlyCheckCount == 1) {
					// Screenshot
					if (PrimaryGamepad.ShareCheckUnpressed) {
						IsScreenshotPressed = true;
						//printf(" Screenshot\n");

					// Record
					}
					else {
						IsRecordPressed = true;
						//printf(" Record\n");
						//GamepadOutState.MicLED = PrimaryGamepad.ShareIsRecording ? MIC_LED_PULSE : MIC_LED_OFF; // doesn't work via BT :(
						//GamepadSetState(PrimaryGamepad);
					}
				}

				PrimaryGamepad.ShareOnlyCheckCount--;
			}

			if (!IsSharePressed && PrimaryGamepad.ShareHandled && PrimaryGamepad.ShareOnlyCheckCount == 0)
				PrimaryGamepad.ShareHandled = false;
		}

		// Custom sens
		if (AppStatus.SkipPollCount == 0 && (PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_N) {
			PrimaryGamepad.Motion.CustomMulSens += 0.2f;
			if (PrimaryGamepad.Motion.CustomMulSens > 2.4f)
				PrimaryGamepad.Motion.CustomMulSens = 0.2f;
			AppStatus.SkipPollCount = SkipPollTimeOut;
		}
		if ((PrimaryGamepad.InputState.buttons & JSMASK_PS || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE) && PrimaryGamepad.InputState.buttons & JSMASK_RCLICK) PrimaryGamepad.Motion.CustomMulSens = 1.0f; //printf("%5.2f\n", CustomMulSens);

		// Gamepad modes
		// Motion racing  [O--]
		if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

			// Steering wheel
			if (!PrimaryGamepad.Motion.AircraftEnabled) {
				report.sThumbLX = (SHORT)(CalcMotionStick(MotionState.gravX, MotionState.gravZ, PrimaryGamepad.Motion.SteeringWheelAngle, PrimaryGamepad.Motion.OffsetAxisX) * 32767);
			}
			// Aircraft
			else {
				const float InputSize = sqrtf(velocityX * velocityX + velocityY * velocityY + velocityZ * velocityZ);
				float TightenedSensitivity = PrimaryGamepad.Motion.AircraftRollSens;
				if (InputSize < Tightening && Tightening > 0)
					TightenedSensitivity *= InputSize / Tightening;

				report.sThumbLX = std::clamp((int)(ClampFloat(-(velocityY * TightenedSensitivity * FrameTime * PrimaryGamepad.Motion.JoySensX * PrimaryGamepad.Motion.CustomMulSens), -1, 1) * 32767 + report.sThumbLX), -32767, 32767);
				report.sThumbLY = (SHORT)(CalcMotionStick(MotionState.gravY, MotionState.gravZ, PrimaryGamepad.Motion.AircraftPitchAngle, PrimaryGamepad.Motion.OffsetAxisY) * 32767) * PrimaryGamepad.Motion.AircraftPitchInverted;
			}
	
		}
		// Motion aiming  [--X}]
		else if (PrimaryGamepad.GamepadActionMode == MotionAimingMode || (
					(PrimaryGamepad.GamepadActionMode == MotionAimingModeOnlyPressed) && (	
						(AppStatus.AimingButton == JSMASK_ZL && DeadZoneAxis(PrimaryGamepad.InputState.lTrigger, PrimaryGamepad.Triggers.DeadZoneLeft) > 0) || // Classic L2 aiming, JSMASK_ZL is L2 - SonyNintendoKeyNameToJoyShockKeyCode()
						(PrimaryGamepad.InputState.buttons & AppStatus.AimingButton) // PS games with emulators (L1) & one-handed controllers like light guns and gaming accessibility
					)
				) ) { 

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
			
		}
		// [-_-] Touchpad sticks
		else if (PrimaryGamepad.GamepadActionMode == TouchpadSticksMode) {

			if (TouchState.t0Down) {
				if (FirstTouch.Touched == false) {
					FirstTouch.InitAxisX = TouchState.t0X;
					FirstTouch.InitAxisY = TouchState.t0Y;
					FirstTouch.Touched = true;
				}
				FirstTouch.AxisX = TouchState.t0X - FirstTouch.InitAxisX;
				FirstTouch.AxisY = TouchState.t0Y - FirstTouch.InitAxisY;

				if (FirstTouch.InitAxisX < 0.5) {
					report.sThumbLX = ClampFloat(FirstTouch.AxisX * PrimaryGamepad.TouchSticks.LeftX, -1, 1) * 32767;
					report.sThumbLY = ClampFloat(-FirstTouch.AxisY * PrimaryGamepad.TouchSticks.LeftY, -1, 1) * 32767;
					if (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
				}
				else {
					report.sThumbRX = ClampFloat((TouchState.t0X - FirstTouch.LastAxisX) * PrimaryGamepad.TouchSticks.RightX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t0Y - FirstTouch.LastAxisY) * PrimaryGamepad.TouchSticks.RightY * 200, -1, 1) * 32767;
					FirstTouch.LastAxisX = TouchState.t0X; FirstTouch.LastAxisY = TouchState.t0Y;
					if (PrimaryGamepad.InputState.buttons & JSMASK_TOUCHPAD_CLICK) report.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
				}
			}
			else {
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
				}
				else {
					report.sThumbRX = ClampFloat((TouchState.t1X - SecondTouch.LastAxisX) * PrimaryGamepad.TouchSticks.RightX * 200, -1, 1) * 32767;
					report.sThumbRY = ClampFloat(-(TouchState.t1Y - SecondTouch.LastAxisY) * PrimaryGamepad.TouchSticks.RightY * 200, -1, 1) * 32767;
					SecondTouch.LastAxisX = TouchState.t1X; SecondTouch.LastAxisY = TouchState.t1Y;
				}
			}
			else {
				SecondTouch.AxisX = 0;
				SecondTouch.AxisY = 0;
				SecondTouch.Touched = false;
			}
		}

		// Keyboard and mouse mode
		bool DontResetInputState = !( // Reset clicks when activating some actions / Сброс нажатий при активации некоторых действий
			PrimaryGamepad.InputState.buttons & JSMASK_PS ||
			IsSharePressed ||
			IsRecordPressed);

		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) {
			
			if (PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {

				float MotionAxisX = CalcMotionStick(MotionState.gravX, MotionState.gravZ, PrimaryGamepad.Motion.SteeringWheelAngle, PrimaryGamepad.Motion.OffsetAxisX);
				if (PrimaryGamepad.InputState.buttons & JSMASK_LEFT) PrimaryGamepad.InputState.buttons &= ~JSMASK_LEFT;
				if (PrimaryGamepad.InputState.buttons & JSMASK_RIGHT) PrimaryGamepad.InputState.buttons &= ~JSMASK_RIGHT;

				if (fabs(MotionAxisX) < PrimaryGamepad.KMEmu.SteeringWheelDeadZone) {
					PrimaryGamepad.KMEmu.MaxLeftAxisX = 0.0f;
					PrimaryGamepad.KMEmu.MaxRightAxisX = 0.0f;
				} else {

					// Left
					if (MotionAxisX < -PrimaryGamepad.KMEmu.SteeringWheelDeadZone) {
						// Update the maximum
						if (MotionAxisX < PrimaryGamepad.KMEmu.MaxLeftAxisX) PrimaryGamepad.KMEmu.MaxLeftAxisX = MotionAxisX;

						// Retention of at least 95% of the peak
						if (MotionAxisX <= PrimaryGamepad.KMEmu.MaxLeftAxisX * (1.0f - PrimaryGamepad.KMEmu.SteeringWheelReleaseThreshold))
							PrimaryGamepad.InputState.buttons |= JSMASK_LEFT;
					}

					// Right
					if (MotionAxisX > PrimaryGamepad.KMEmu.SteeringWheelDeadZone) {
						if (MotionAxisX > PrimaryGamepad.KMEmu.MaxRightAxisX) PrimaryGamepad.KMEmu.MaxRightAxisX = MotionAxisX;

						if (MotionAxisX >= PrimaryGamepad.KMEmu.MaxRightAxisX * (1.0f - PrimaryGamepad.KMEmu.SteeringWheelReleaseThreshold))
							PrimaryGamepad.InputState.buttons |= JSMASK_RIGHT;
					}
				}

			}

			KeyPress(PrimaryGamepad.ButtonsStates.LeftTrigger.KeyCode, DontResetInputState && PrimaryGamepad.InputState.lTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey, &PrimaryGamepad.ButtonsStates.LeftTrigger, true);
			KeyPress(PrimaryGamepad.ButtonsStates.RightTrigger.KeyCode, DontResetInputState && PrimaryGamepad.InputState.rTrigger > PrimaryGamepad.KMEmu.TriggerValuePressKey, &PrimaryGamepad.ButtonsStates.RightTrigger, true);

			KeyPress(PrimaryGamepad.ButtonsStates.LeftBumper.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_L, &PrimaryGamepad.ButtonsStates.LeftBumper, true);
			KeyPress(PrimaryGamepad.ButtonsStates.RightBumper.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_R, &PrimaryGamepad.ButtonsStates.RightBumper, true);

			KeyPress(PrimaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_SHARE || PrimaryGamepad.InputState.buttons & JSMASK_CAPTURE), &PrimaryGamepad.ButtonsStates.Back, true);
			KeyPress(PrimaryGamepad.ButtonsStates.Start.KeyCode, DontResetInputState && (PrimaryGamepad.InputState.buttons & JSMASK_OPTIONS || PrimaryGamepad.InputState.buttons & JSMASK_HOME), &PrimaryGamepad.ButtonsStates.Start, true);

			if (PrimaryGamepad.ButtonsStates.DPADAdvancedMode == false) { // Regular mode  ↑ → ↓ ←
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP, &PrimaryGamepad.ButtonsStates.DPADUp, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN, &PrimaryGamepad.ButtonsStates.DPADDown, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADRight, true);

			}
			else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖ for switching in retro games
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &PrimaryGamepad.ButtonsStates.DPADUp, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LEFT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &PrimaryGamepad.ButtonsStates.DPADLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT && !(PrimaryGamepad.InputState.buttons & JSMASK_UP) && !(PrimaryGamepad.InputState.buttons & JSMASK_DOWN), &PrimaryGamepad.ButtonsStates.DPADRight, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && !(PrimaryGamepad.InputState.buttons & JSMASK_LEFT) && !(PrimaryGamepad.InputState.buttons & JSMASK_RIGHT), &PrimaryGamepad.ButtonsStates.DPADDown, true);

				KeyPress(PrimaryGamepad.ButtonsStates.DPADUpLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADUpLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADUpRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_UP && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADUpRight, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDownLeft.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_LEFT, &PrimaryGamepad.ButtonsStates.DPADDownLeft, true);
				KeyPress(PrimaryGamepad.ButtonsStates.DPADDownRight.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_DOWN && PrimaryGamepad.InputState.buttons & JSMASK_RIGHT, &PrimaryGamepad.ButtonsStates.DPADDownRight, true);
			}

			KeyPress(PrimaryGamepad.ButtonsStates.Y.KeyCode, DontResetInputState &&  PrimaryGamepad.InputState.buttons & JSMASK_N, &PrimaryGamepad.ButtonsStates.Y, true);
			KeyPress(PrimaryGamepad.ButtonsStates.A.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_S, &PrimaryGamepad.ButtonsStates.A, true);
			KeyPress(PrimaryGamepad.ButtonsStates.X.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_W, &PrimaryGamepad.ButtonsStates.X, true);
			KeyPress(PrimaryGamepad.ButtonsStates.B.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_E, &PrimaryGamepad.ButtonsStates.B, true);

			KeyPress(PrimaryGamepad.ButtonsStates.LeftStick.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_LCLICK, &PrimaryGamepad.ButtonsStates.LeftStick, true);
			KeyPress(PrimaryGamepad.ButtonsStates.RightStick.KeyCode, DontResetInputState && PrimaryGamepad.InputState.buttons & JSMASK_RCLICK, &PrimaryGamepad.ButtonsStates.RightStick, true);

			KMStickMode(PrimaryGamepad, DontResetInputState, true, DeadZoneAxis(PrimaryGamepad.InputState.stickLX, PrimaryGamepad.Sticks.DeadZoneLeftX), DeadZoneAxis(PrimaryGamepad.InputState.stickLY, PrimaryGamepad.Sticks.DeadZoneLeftY), PrimaryGamepad.KMEmu.LeftStickMode);
			KMStickMode(PrimaryGamepad, DontResetInputState, false, DeadZoneAxis(PrimaryGamepad.InputState.stickRX, PrimaryGamepad.Sticks.DeadZoneRightX), DeadZoneAxis(PrimaryGamepad.InputState.stickRY, PrimaryGamepad.Sticks.DeadZoneRightY), PrimaryGamepad.KMEmu.RightStickMode);
		
			// Motion wheel
			PrimaryGamepad.ButtonsStates.WheelDefault.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelUp.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelDown.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelRight.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelUpLeft.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelUpRight.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelDownLeft.IsPressed = false;
			PrimaryGamepad.ButtonsStates.WheelDownRight.IsPressed = false;
			
			if (PrimaryGamepad.ButtonsStates.WheelActivationGamepadButton.KeyCode != 0 && PrimaryGamepad.InputState.buttons & PrimaryGamepad.ButtonsStates.WheelActivationGamepadButton.KeyCode && PrimaryGamepad.Motion.WheelCounter == 0) {
				//printf("start\n");
				PrimaryGamepad.Motion.WheelCounter = SkipPollTimeOut;
				PrimaryGamepad.Motion.WheelActive = true;
				PrimaryGamepad.Motion.WheelAccumX = 0.0f;
				PrimaryGamepad.Motion.WheelAccumY = 0.0f;
			}

			if ((PrimaryGamepad.InputState.buttons & PrimaryGamepad.ButtonsStates.WheelActivationGamepadButton.KeyCode) == false && PrimaryGamepad.Motion.WheelCounter > 0) {
				
				if (PrimaryGamepad.Motion.WheelCounter == 1) {

					//printf("%5.2f %5.2f\n", PrimaryGamepad.GyroWheelAccumX, PrimaryGamepad.GyroWheelAccumY);

					float MotionLength = sqrtf(PrimaryGamepad.Motion.WheelAccumX * PrimaryGamepad.Motion.WheelAccumX + PrimaryGamepad.Motion.WheelAccumY * PrimaryGamepad.Motion.WheelAccumY);

					//printf("%5.2f\n", len);

					if (MotionLength < PrimaryGamepad.Motion.MotionWheelButtonsDeadZone) {
						//printf("default\n");
						PrimaryGamepad.ButtonsStates.WheelDefault.IsPressed = true;
					
					} else {
						float MotionWheelAngle = atan2f(PrimaryGamepad.Motion.WheelAccumY, PrimaryGamepad.Motion.WheelAccumX) * 57.29578f;

						if (PrimaryGamepad.ButtonsStates.WheelAdvancedMode == false) { // Regular mode  ↑ → ↓ ←

							if (fabs(MotionWheelAngle) < 25.f) {
								//printf("up\n");
								PrimaryGamepad.ButtonsStates.WheelUp.IsPressed = true;
							}
							else if (MotionWheelAngle > 45.f && MotionWheelAngle < 135.f) {
								//printf("left\n");
								PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed = true;
							}
							else if (MotionWheelAngle < -45.f && MotionWheelAngle > -135.f) {
								//printf("right\n");
								PrimaryGamepad.ButtonsStates.WheelRight.IsPressed = true;
							}
							else if (MotionWheelAngle > 165.f || MotionWheelAngle < -165.f) {
								//printf("down\n");
								PrimaryGamepad.ButtonsStates.WheelDown.IsPressed = true;
							}

						} else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖
							if (MotionWheelAngle >= -22.5f && MotionWheelAngle < 22.5f) {
								//printf("up\n");
								PrimaryGamepad.ButtonsStates.WheelUp.IsPressed = true;
							} else if (MotionWheelAngle >= 22.5f && MotionWheelAngle < 67.5f) {
								//printf("up-left\n");
								PrimaryGamepad.ButtonsStates.WheelUpLeft.IsPressed = true;
							} else if (MotionWheelAngle >= 67.5f && MotionWheelAngle < 112.5f) {
								//printf("left\n");
								PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed = true;
							} else if (MotionWheelAngle >= 112.5f && MotionWheelAngle < 157.5f) {
								//printf("down-left\n");
								PrimaryGamepad.ButtonsStates.WheelDownLeft.IsPressed = true;
							} else if (MotionWheelAngle >= 157.5f || MotionWheelAngle < -157.5f) {
								//printf("down\n");
								PrimaryGamepad.ButtonsStates.WheelDown.IsPressed = true;
							} else if (MotionWheelAngle >= -157.5f && MotionWheelAngle < -112.5f) {
								//printf("down-right\n");
								PrimaryGamepad.ButtonsStates.WheelDownRight.IsPressed = true;
							} else if (MotionWheelAngle >= -112.5f && MotionWheelAngle < -67.5f) {
								//printf("right\n");
								PrimaryGamepad.ButtonsStates.WheelRight.IsPressed = true;
							} else if (MotionWheelAngle >= -67.5f && MotionWheelAngle < -22.5f) {
								//printf("up-right\n");
								PrimaryGamepad.ButtonsStates.WheelUpRight.IsPressed = true;
							}
						}
					
					}

					PrimaryGamepad.Motion.WheelActive = false;
				}

				PrimaryGamepad.Motion.WheelCounter--;
			}

			if (PrimaryGamepad.Motion.WheelActive) {
				PrimaryGamepad.Motion.WheelAccumX += velocityX * FrameTime;
				PrimaryGamepad.Motion.WheelAccumY += velocityY * FrameTime;
			}

			KeyPress(PrimaryGamepad.ButtonsStates.WheelDefault.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDefault.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDefault, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelUp.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelUp.IsPressed, &PrimaryGamepad.ButtonsStates.WheelUp, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelUpLeft.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelUpLeft.IsPressed, &PrimaryGamepad.ButtonsStates.WheelUpLeft, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelUpRight.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelUpRight.IsPressed, &PrimaryGamepad.ButtonsStates.WheelRight, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelDown.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDown.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDown, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelDownLeft.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDownLeft.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDownLeft, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelDownRight.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelDownRight.IsPressed, &PrimaryGamepad.ButtonsStates.WheelDownRight, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelLeft.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelLeft.IsPressed, &PrimaryGamepad.ButtonsStates.WheelLeft, true);
			KeyPress(PrimaryGamepad.ButtonsStates.WheelRight.KeyCode, DontResetInputState && PrimaryGamepad.ButtonsStates.WheelRight.IsPressed, &PrimaryGamepad.ButtonsStates.WheelRight, true);
		}

		// After releasing all the buttons you can click on screenshots, so it's here / После отпускания всех кнопок можно нажимать скриншоты, поэтому это здесь
		// Microphone (custom key)
		if (AppStatus.ScreenshotMode == ScreenShotCustomKeyMode)
			KeyPress(AppStatus.ScreenShotKey, IsSharePressed, &PrimaryGamepad.ButtonsStates.Screenshot, false);

		// Microphone (screenshots / record)
		else {
			KeyPress(AppStatus.ScreenShotKey, IsScreenshotPressed, &PrimaryGamepad.ButtonsStates.Screenshot, false);
			KeyPress(VK_GAMEBAR_RECORD, IsRecordPressed, &PrimaryGamepad.ButtonsStates.Record, false);
		}

		if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
			if (AppStatus.GamepadEmulationMode != EmuKeyboardAndMouse) {
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

				if (JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_DS || JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_DS4) {
					if (!(SecondaryGamepad.InputState.buttons & JSMASK_PS)) {
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_SHARE ? XINPUT_GAMEPAD_BACK : 0;
						report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_OPTIONS ? XINPUT_GAMEPAD_START : 0;
					}

					if (AppStatus.SkipPollCount == 0 && (SecondaryGamepad.InputState.buttons & JSMASK_PS && SecondaryGamepad.InputState.buttons & JSMASK_L))
					{
						if (SecondaryGamepad.OutState.LEDBrightness == SecondaryGamepad.DefaultLEDBrightness)
							SecondaryGamepad.OutState.LEDBrightness = 255;
						else
							SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.DefaultLEDBrightness;
						GamepadSetState(SecondaryGamepad);
						AppStatus.SkipPollCount = SkipPollTimeOut;
					}
				}
				else if (JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_PRO_CONTROLLER || JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_LEFT || JslGetControllerType(SecondaryGamepad.DeviceIndex) == JS_TYPE_JOYCON_RIGHT) {
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_MINUS ? XINPUT_GAMEPAD_BACK : 0;
					report2.wButtons |= SecondaryGamepad.InputState.buttons & JSMASK_PLUS ? XINPUT_GAMEPAD_START : 0;
				}

				if (SecondaryGamepad.RumbleSkipCounter > 0)
					SecondaryGamepad.RumbleSkipCounter--;
			}
			// Secondary gamepad keyboard & mouse
			else {
				KeyPress(SecondaryGamepad.ButtonsStates.LeftTrigger.KeyCode, DontResetInputState && SecondaryGamepad.InputState.lTrigger > SecondaryGamepad.KMEmu.TriggerValuePressKey, &SecondaryGamepad.ButtonsStates.LeftTrigger, true);
				KeyPress(SecondaryGamepad.ButtonsStates.RightTrigger.KeyCode, DontResetInputState && SecondaryGamepad.InputState.rTrigger > SecondaryGamepad.KMEmu.TriggerValuePressKey, &SecondaryGamepad.ButtonsStates.RightTrigger, true);

				KeyPress(SecondaryGamepad.ButtonsStates.LeftBumper.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_L, &SecondaryGamepad.ButtonsStates.LeftBumper, true);
				KeyPress(SecondaryGamepad.ButtonsStates.RightBumper.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_R, &SecondaryGamepad.ButtonsStates.RightBumper, true);

				KeyPress(SecondaryGamepad.ButtonsStates.Back.KeyCode, DontResetInputState && (SecondaryGamepad.InputState.buttons & JSMASK_SHARE || SecondaryGamepad.InputState.buttons & JSMASK_CAPTURE), &SecondaryGamepad.ButtonsStates.Back, true);
				KeyPress(SecondaryGamepad.ButtonsStates.Start.KeyCode, DontResetInputState && (SecondaryGamepad.InputState.buttons & JSMASK_OPTIONS || SecondaryGamepad.InputState.buttons & JSMASK_HOME), &SecondaryGamepad.ButtonsStates.Start, true);

				if (SecondaryGamepad.ButtonsStates.DPADAdvancedMode == false) { // Regular mode  ↑ → ↓ ←
					KeyPress(SecondaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP, &SecondaryGamepad.ButtonsStates.DPADUp, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN, &SecondaryGamepad.ButtonsStates.DPADDown, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_LEFT, &SecondaryGamepad.ButtonsStates.DPADLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT, &SecondaryGamepad.ButtonsStates.DPADRight, true);

				}
				else { // Advanced mode ↑ ↗ → ↘ ↓ ↙ ← ↖ for switching in retro games
					KeyPress(SecondaryGamepad.ButtonsStates.DPADUp.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP && !(SecondaryGamepad.InputState.buttons & JSMASK_LEFT) && !(SecondaryGamepad.InputState.buttons & JSMASK_RIGHT), &SecondaryGamepad.ButtonsStates.DPADUp, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_LEFT && !(SecondaryGamepad.InputState.buttons & JSMASK_UP) && !(SecondaryGamepad.InputState.buttons & JSMASK_DOWN), &SecondaryGamepad.ButtonsStates.DPADLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT && !(SecondaryGamepad.InputState.buttons & JSMASK_UP) && !(SecondaryGamepad.InputState.buttons & JSMASK_DOWN), &SecondaryGamepad.ButtonsStates.DPADRight, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDown.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN && !(SecondaryGamepad.InputState.buttons & JSMASK_LEFT) && !(SecondaryGamepad.InputState.buttons & JSMASK_RIGHT), &SecondaryGamepad.ButtonsStates.DPADDown, true);

					KeyPress(SecondaryGamepad.ButtonsStates.DPADUpLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP && SecondaryGamepad.InputState.buttons & JSMASK_LEFT, &SecondaryGamepad.ButtonsStates.DPADUpLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADUpRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_UP && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT, &SecondaryGamepad.ButtonsStates.DPADUpRight, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDownLeft.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN && SecondaryGamepad.InputState.buttons & JSMASK_LEFT, &SecondaryGamepad.ButtonsStates.DPADDownLeft, true);
					KeyPress(SecondaryGamepad.ButtonsStates.DPADDownRight.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_DOWN && SecondaryGamepad.InputState.buttons & JSMASK_RIGHT, &SecondaryGamepad.ButtonsStates.DPADDownRight, true);
				}

				KeyPress(SecondaryGamepad.ButtonsStates.Y.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_N, &SecondaryGamepad.ButtonsStates.Y, true);
				KeyPress(SecondaryGamepad.ButtonsStates.A.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_S, &SecondaryGamepad.ButtonsStates.A, true);
				KeyPress(SecondaryGamepad.ButtonsStates.X.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_W, &SecondaryGamepad.ButtonsStates.X, true);
				KeyPress(SecondaryGamepad.ButtonsStates.B.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_E, &SecondaryGamepad.ButtonsStates.B, true);

				KeyPress(SecondaryGamepad.ButtonsStates.LeftStick.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_LCLICK, &SecondaryGamepad.ButtonsStates.LeftStick, true);
				KeyPress(SecondaryGamepad.ButtonsStates.RightStick.KeyCode, DontResetInputState && SecondaryGamepad.InputState.buttons & JSMASK_RCLICK, &SecondaryGamepad.ButtonsStates.RightStick, true);

				KMStickMode(SecondaryGamepad, DontResetInputState, true, DeadZoneAxis(SecondaryGamepad.InputState.stickLX, SecondaryGamepad.Sticks.DeadZoneLeftX), DeadZoneAxis(SecondaryGamepad.InputState.stickLY, SecondaryGamepad.Sticks.DeadZoneLeftY), SecondaryGamepad.KMEmu.LeftStickMode);
				KMStickMode(SecondaryGamepad, DontResetInputState, false, DeadZoneAxis(SecondaryGamepad.InputState.stickRX, SecondaryGamepad.Sticks.DeadZoneRightX), DeadZoneAxis(SecondaryGamepad.InputState.stickRY, SecondaryGamepad.Sticks.DeadZoneRightY), SecondaryGamepad.KMEmu.RightStickMode);
			}
		}

		if (AppStatus.GamepadEmulationMode == EmuGamepadEnabled || (AppStatus.GamepadEmulationMode == EmuGamepadOnlyDriving && PrimaryGamepad.GamepadActionMode == MotionDrivingMode) || AppStatus.XboxGamepadReset) {
			if (AppStatus.XboxGamepadReset) { AppStatus.XboxGamepadReset = false; XUSB_REPORT_INIT(&report); }
			//if (AppStatus.XboxGamepadAttached)
				//ret = vigem_target_x360_update(client, x360, report);
		}

		if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse) { // Temporary hack(Vigem always, no removal)
			XUSB_REPORT_INIT(&report);
			report.sThumbLX = 1; // Maybe the crash is due to power saving? temporary test
			if (AppStatus.SecondaryGamepadEnabled) {
				XUSB_REPORT_INIT(&report2);
				report2.sThumbLX = 1; // Maybe the crash is due to power saving? temporary test
			}
		}
		ret = vigem_target_x360_update(client, x360, report);
		if (AppStatus.SecondaryGamepadEnabled)
			ret = vigem_target_x360_update(client2, x3602, report2);

		// Battery level display
		if (AppStatus.BackOutStateCounter > 0) {
			if (AppStatus.BackOutStateCounter == 1) {
				PrimaryGamepad.OutState.LEDColor = PrimaryGamepad.DefaultModeColor;
				PrimaryGamepad.OutState.PlayersCount = 0;
				if (AppStatus.ShowBatteryStatusOnLightBar) PrimaryGamepad.OutState.LEDBrightness = PrimaryGamepad.LastLEDBrightness;
				GamepadSetState(PrimaryGamepad);

				if (AppStatus.SecondaryGamepadEnabled && SecondaryGamepad.DeviceIndex != -1) {
					SecondaryGamepad.OutState.LEDColor = SecondaryGamepad.DefaultModeColor;
					SecondaryGamepad.OutState.PlayersCount = 0;
					if (AppStatus.ShowBatteryStatusOnLightBar) SecondaryGamepad.OutState.LEDBrightness = SecondaryGamepad.LastLEDBrightness;
					GamepadSetState(SecondaryGamepad);
				}

				AppStatus.ShowBatteryStatus = false;
				MainTextUpdate();
			}
			AppStatus.BackOutStateCounter--;
		}

		if (PrimaryGamepad.RumbleSkipCounter > 0)
			PrimaryGamepad.RumbleSkipCounter--;

		if (AppStatus.SkipPollCount > 0) AppStatus.SkipPollCount--;
		Sleep(AppStatus.SleepTimeOut);
	}

	// Reset keyboard motion driving
	if (AppStatus.GamepadEmulationMode == EmuKeyboardAndMouse && PrimaryGamepad.GamepadActionMode == MotionDrivingMode) {
		KeyPress(PrimaryGamepad.ButtonsStates.DPADLeft.KeyCode, false, &PrimaryGamepad.ButtonsStates.DPADLeft, true);
		KeyPress(PrimaryGamepad.ButtonsStates.DPADRight.KeyCode, false, &PrimaryGamepad.ButtonsStates.DPADRight, true);
	}

	// Reset adaptive triggers
	if (PrimaryGamepad.AdaptiveTriggersMode > 0) {
		PrimaryGamepad.AdaptiveTriggersOutputMode = 0;
		GamepadSetState(PrimaryGamepad);
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

	if (AppStatus.SecondaryGamepadEnabled) {
		vigem_target_x360_unregister_notification(x3602);
		vigem_target_remove(client2, x3602);
		vigem_target_free(x3602);

		vigem_disconnect(client2);
		vigem_free(client2);
	}
}
