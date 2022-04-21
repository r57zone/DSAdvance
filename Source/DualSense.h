#pragma once

#include "MahonyAHRS.h"

#define SONY_VENDOR 0x054C
#define SONY_DS5_USB 0x0CE6

#define SONY_DUALSENSE					28

#define TOUCHPAD_WIDTH					1920
#define TOUCHPAD_HEIGHT					943

#define NEX_CONTROLLER_WIRED            0
#define NEX_CONTROLLER_WIRELESS         1
#define NEX_BATTERY_NONE                0
#define NEX_BATTERY_LOW                 1
#define NEX_BATTERY_FULL                5

#define DS_MAX_COUNT					4

#define ERROR_DEVICE_NOT_CONNECTED      1
#define ERROR_SUCCESS                   0

typedef struct _DS5_BUTTONS
{
	bool							DPAD_UP;
	bool							DPAD_DOWN;
	bool							DPAD_LEFT;
	bool							DPAD_RIGHT;

	bool							OPTIONS;
	bool							SHARE;

	bool							TOUCHPAD;
	bool							PS;
	bool							MIC;

	bool							LS;
	bool							RS;
	bool							L1;
	bool							R1;

	bool							L2;
	bool							R2;

	bool							A;
	bool							B;
	bool							X;
	bool							Y;
} DS5_BUTTONS;

typedef struct _DS5_TOUCH
{
	bool							FirstDown;
	WORD							FirstX;
	WORD							FirstY;
	bool							SecondDown;
	WORD							SecondX;
	WORD							SecondY;
} DS5_TOUCH;

typedef struct _DS5_MOTION
{
	float								AccelX;
	float								AccelY;
	float								AccelZ;
	float								GyroX;
	float								GyroY;
	float								GyroZ;
} DS5_MOTION;

typedef struct _DS_INPUT_STATE
{
	DS5_BUTTONS							Buttons;
	BYTE								L2;
	BYTE								R2;
	SHORT								AxisLX;
	SHORT								AxisLY;
	SHORT								AxisRX;
	SHORT								AxisRY;
	DS5_TOUCH							Touch;
	DS5_MOTION							Motion;
} DS_INPUT_STATE, *PDS_INPUT_STATE;

typedef struct _DS_OUTPUT_STATE
{
	WORD								LeftMotorSpeed;
	WORD								RightMotorSpeed;
	BYTE								LEDBrightness;
	BYTE								LEDRed;
	BYTE								LEDGreen;
	BYTE								LEDBlue;
} DS_OUTPUT_STATE, *PDS_OUTPUT_STATE;

typedef struct _DS_CONTROLLER_INFO
{
	WORD								ControllerType;
	BYTE								ConnectType;
	BYTE								BatteryLevel;
	bool								SupportRotation;
} DS_CONTROLLER_INFO, *PDS_CONTROLLER_INFO;


typedef DWORD(__stdcall *_NEXInputGetState)(__in DWORD dwUserIndex, __out DS_INPUT_STATE *pInputState);
typedef DWORD(__stdcall *_NEXInputSetState)(__in DWORD dwUserIndex, __in DS_OUTPUT_STATE *pOutputState);
typedef DWORD(__stdcall *_NEXInputGetInfo)(__in DWORD dwUserIndex, __out DS_CONTROLLER_INFO *pControllerInfo);
typedef DWORD(__stdcall *_NEXInputPowerOff)(__in DWORD dwUserIndex);


typedef struct
{
	unsigned char Unknown;
	unsigned char AxisLX;              /* 0 */
	unsigned char AxisLY;              /* 1 */
	unsigned char AxisRX;             /* 2 */
	unsigned char AxisRY;             /* 3 */
	unsigned char L2;                /* 4 */
	unsigned char R2;               /* 5 */
	unsigned char Counter;                    /* 6 */
	unsigned char ButtonsAndHat[3];         /* 7 */
	unsigned char Zero;                       /* 10 */
	unsigned char rgucPacketSequence[4];        /* 11 - 32 bit little endian */
	unsigned char rgucGyroX[2];                 /* 15 */
	unsigned char rgucGyroY[2];                 /* 17 */
	unsigned char rgucGyroZ[2];                 /* 19 */
	unsigned char rgucAccelX[2];                /* 21 */
	unsigned char rgucAccelY[2];                /* 23 */
	unsigned char rgucAccelZ[2];                /* 25 */
	unsigned char rgucTimer1[4];                /* 27 - 32 bit little endian */
	unsigned char ucBatteryTemp;                /* 31 */
	unsigned char ucTouchpadCounter1;           /* 32 - high bit clear + counter */
	unsigned char rgucTouchpadData1[3];         /* 33 - X/Y, 12 bits per axis */
	unsigned char ucTouchpadCounter2;           /* 36 - high bit clear + counter */
	unsigned char rgucTouchpadData2[3];         /* 37 - X/Y, 12 bits per axis */
	unsigned char rgucUnknown1[8];              /* 40 */
	unsigned char rgucTimer2[4];                /* 48 - 32 bit little endian */
	unsigned char ucBatteryLevel;               /* 52 */
	unsigned char ucConnectState;               /* 53 - 0x08 = USB, 0x01 = headphone */

	/* There's more unknown data at the end, and a 32-bit CRC on Bluetooth */
} DS5_USB_PACKET;


//DISCHARGING = 0x00, /*!< Battery is discharging */
//CHARGING = 0x01, /*!< Battery is charging */
//CHARGED = 0x02, /*!< Battery is charged */
//VOLTAGE_ERROR = 0x0A, /*!< Voltage error */
//TEMP_ERROR = 0x0B, /*!< Temperature error */
//CHARGING_ERROR = 0x0F  /*!< Unknown error */

DS_INPUT_STATE DS5ToNexInputState(DS5_USB_PACKET *DS5State) {
	DS_INPUT_STATE NexInputState;

	memset(&NexInputState, 0, sizeof(DS_INPUT_STATE));

	NexInputState.AxisLX = (DS5State->AxisLX + ((USHRT_MAX / 2) + 1)) * 257;
	NexInputState.AxisLY = (-(DS5State->AxisLY + ((USHRT_MAX / 2) + 1)) * 257);
	NexInputState.AxisRX = ((DS5State->AxisRX + ((USHRT_MAX / 2) + 1)) * 257);
	NexInputState.AxisRY = (-(DS5State->AxisRY + ((USHRT_MAX / 2) + 1)) * 257);
	if (NexInputState.AxisLX >= -128 && NexInputState.AxisLX <= 128) NexInputState.AxisLX = 0;
	if (NexInputState.AxisLY >= -128 && NexInputState.AxisLY <= 128) NexInputState.AxisLY = 0;
	if (NexInputState.AxisRX >= -128 && NexInputState.AxisRX <= 128) NexInputState.AxisRX = 0;
	if (NexInputState.AxisRY >= -128 && NexInputState.AxisRY <= 128) NexInputState.AxisRY = 0;

	NexInputState.Touch.FirstX = (DS5State->rgucTouchpadData1[0] | (DS5State->rgucTouchpadData1[1] & 0x0F) << 8); // / 1920.0f;
	NexInputState.Touch.FirstY = ((DS5State->rgucTouchpadData1[1] & 0xF0) >> 4 | DS5State->rgucTouchpadData1[2] << 4); // / 943.0f;

	NexInputState.Touch.SecondX = (DS5State->rgucTouchpadData2[0] | (DS5State->rgucTouchpadData2[1] & 0x0F) << 8); // / 1920.0f;
	NexInputState.Touch.SecondY = ((DS5State->rgucTouchpadData2[1] & 0xF0) >> 4 | DS5State->rgucTouchpadData2[2] << 4); // / 943.0f;

	NexInputState.Touch.FirstDown = (DS5State->ucTouchpadCounter1 & 0x80) == 0;
	NexInputState.Touch.SecondDown = (DS5State->ucTouchpadCounter2 & 0x80) == 0;

	NexInputState.L2 = DS5State->L2;
	NexInputState.R2 = DS5State->R2;

	NexInputState.Buttons.L1 |= DS5State->ButtonsAndHat[1] & 0x0001;
	NexInputState.Buttons.R1 |= DS5State->ButtonsAndHat[1] & 0x0002;

	NexInputState.Buttons.TOUCHPAD |= DS5State->ButtonsAndHat[2] & 0x0002;
	NexInputState.Buttons.PS |= DS5State->ButtonsAndHat[2] & 0x0001;
	NexInputState.Buttons.MIC |= DS5State->ButtonsAndHat[2] & 0x0004;

	NexInputState.Buttons.SHARE |= DS5State->ButtonsAndHat[1] & 0x0010;
	NexInputState.Buttons.OPTIONS |= DS5State->ButtonsAndHat[1] & 0x0020;
	
	NexInputState.Buttons.L2 |= DS5State->ButtonsAndHat[1] & 0x0004;
	NexInputState.Buttons.R2 |= DS5State->ButtonsAndHat[1] & 0x0008;

	NexInputState.Buttons.Y |= DS5State->ButtonsAndHat[0] & (1 << 7);
	NexInputState.Buttons.X |= DS5State->ButtonsAndHat[0] & (1 << 4);
	NexInputState.Buttons.B |= DS5State->ButtonsAndHat[0] & (1 << 6);
	NexInputState.Buttons.A |= DS5State->ButtonsAndHat[0] & (1 << 5);

	NexInputState.Buttons.LS |= DS5State->ButtonsAndHat[1] & 0x0040;
	NexInputState.Buttons.RS |= DS5State->ButtonsAndHat[1] & 0x0080;

	//int16_t gyroSampleX = uint16_to_int16(DS5State->rgucGyroX[0] | (DS5State->rgucGyroX[1] << 8) & 0xFF00);
	//int16_t gyroSampleY = uint16_to_int16(DS5State->rgucGyroY[0] | (DS5State->rgucGyroY[1] << 8) & 0xFF00);
	//int16_t gyroSampleZ = uint16_to_int16(DS5State->rgucGyroZ[0] | (DS5State->rgucGyroZ[1] << 8) & 0xFF00);
	//int16_t accelSampleX = uint16_to_int16(DS5State->rgucAccelX[0] | (DS5State->rgucAccelX[1] << 8) & 0xFF00);
	//int16_t accelSampleY = uint16_to_int16(DS5State->rgucAccelY[0] | (DS5State->rgucAccelY[1] << 8) & 0xFF00);
	//int16_t accelSampleZ = uint16_to_int16(DS5State->rgucAccelZ[0] | (DS5State->rgucAccelZ[1] << 8) & 0xFF00);

	int16_t gyroSampleX = DS5State->rgucGyroX[0] | (DS5State->rgucGyroX[1] << 8) & 0xFF00;
	int16_t gyroSampleY = DS5State->rgucGyroY[0] | (DS5State->rgucGyroY[1] << 8) & 0xFF00;
	int16_t gyroSampleZ = DS5State->rgucGyroZ[0] | (DS5State->rgucGyroZ[1] << 8) & 0xFF00;
	int16_t accelSampleX = DS5State->rgucAccelX[0] | (DS5State->rgucAccelX[1] << 8) & 0xFF00;
	int16_t accelSampleY = DS5State->rgucAccelY[0] | (DS5State->rgucAccelY[1] << 8) & 0xFF00;
	int16_t accelSampleZ = DS5State->rgucAccelZ[0] | (DS5State->rgucAccelZ[1] << 8) & 0xFF00;

	//uint16_t ACCELEROMETER_RESOLUTION = 8192;
	//int16_t GYROSCOPE_RESOLUTION = 1024;

	/* JSM
	
	NexInputState.Motion.GyroX = (float)(gyroSampleX) * (2000.0 / 32767.0);
	NexInputState.Motion.GyroY = (float)(gyroSampleY) * (2000.0 / 32767.0);
	NexInputState.Motion.GyroZ = (float)(gyroSampleZ) * (2000.0 / 32767.0);

	NexInputState.Motion.AccelX = (float)(accelSampleX) / 8192.0;
	NexInputState.Motion.AccelY = (float)(accelSampleY) / 8192.0;
	NexInputState.Motion.AccelZ = (float)(accelSampleZ) / 8192.0;*/

	NexInputState.Motion.GyroX = (float)(gyroSampleX);// / 1024 * 2;
	NexInputState.Motion.GyroY = (float)(gyroSampleY);// / 1024 * 2;
	NexInputState.Motion.GyroZ = (float)(gyroSampleZ);// / 1024 * 2;

	NexInputState.Motion.AccelX = (float)(accelSampleX) / 8192.0;
	NexInputState.Motion.AccelY = (float)(accelSampleY) / 8192.0;
	NexInputState.Motion.AccelZ = (float)(accelSampleZ) / 8192.0;

	DS5State->ButtonsAndHat[0] &= 0xF;
	NexInputState.Buttons.DPAD_UP |= (DS5State->ButtonsAndHat[0] == 0x0007 || DS5State->ButtonsAndHat[0] == 0x0000 || DS5State->ButtonsAndHat[0] == 0x0001);
	NexInputState.Buttons.DPAD_DOWN |= (DS5State->ButtonsAndHat[0] == 0x0005 || DS5State->ButtonsAndHat[0] == 0x0004 || DS5State->ButtonsAndHat[0] == 0x0003);
	
	NexInputState.Buttons.DPAD_LEFT |= (DS5State->ButtonsAndHat[0] == 0x0006 || DS5State->ButtonsAndHat[0] == 0x0007 || DS5State->ButtonsAndHat[0] == 0x0005);
	NexInputState.Buttons.DPAD_RIGHT |= (DS5State->ButtonsAndHat[0] == 0x0002 || DS5State->ButtonsAndHat[0] == 0x0001 || DS5State->ButtonsAndHat[0] == 0x0003);

	//printf("%d %d %d\n", DS5State->ButtonsAndHat[0], DS5State->ButtonsAndHat[1], DS5State->ButtonsAndHat[2]);

	return NexInputState;
}

unsigned char *DS5ToNexOutputState(DS_OUTPUT_STATE NexOutputState) {
	unsigned char outputReport[48];
	memset(outputReport, 0, 48);

	outputReport[0] = 0x02; // report type

	outputReport[1] = 0xff; // flags determining what changes this packet will perform
		// 0x01 ??? used by PS Remote Play on startup
		// 0x02 ???
		// 0x04 ??? used by PS Remote Play on startup
		// 0x08 ??? used by PS Remote Play on startup
		// 0x10 modification of audio volume
		// 0x20 toggling of internal speaker while headset is connected 
		// 0x40 modification of microphone volume
		// 0x80 toggling of internal mic or external speaker while headset is connected 
	outputReport[2] = 0x15; // further flags determining what changes this packet will perform
		// 0x01 toggling microphone LED
		// 0x02 toggling audio/mic mute
		// 0x04 toggling LED strips on the sides of the touchpad
		// 0x08 will actively turn all LEDs off? Convenience flag? (if so, third parties might not support it properly)
		// 0x10 toggling white player indicator LEDs below touchpad
		// 0x20 ???
		// 0x40 ??? used by PS Remote Play on startup and exit
		// 0x80 ???

	// main motors		
	outputReport[3] = NexOutputState.LeftMotorSpeed / 257; //0 .. 65535; // left low freq motor 0-255
	outputReport[4] = NexOutputState.RightMotorSpeed / 257; // right high freq motor 0-255

	// audio settings requiring volume control flags
	outputReport[5] = 0xff; // audio volume of connected headphones (maxes out at about 0x7f)
	outputReport[6] = 0xff; // volume of internal speaker (0-255) (ties in with index 38?!?)
	outputReport[7] = 0xff; // internal microphone volume (not at all linear; 0-255, appears to max out at about 64; 0 is not fully muted, use audio mute flag instead!)
	outputReport[8] = 0x0c; // internal device enablement override flags (0xc default by ps remote play)
		// 0x01 = enable internal microphone (in addition to a connected headset) 
		// 0x04 = ??? set by default via PS Remote Play
		// 0x08 = ??? set by default via PS Remote Play 
		// 0x10 = disable attached headphones (only if 0x20 to enable internal speakers is provided as well)
		// 0x20 = enable audio on internal speaker (in addition to a connected headset)

	// audio related LEDs requiring according LED toggle flags
	outputReport[9] = 0; // microphone LED (1 = on, 2 = pulsating / neither does affect the mic)

	// audio settings requiring mute toggling flags
	outputReport[10] = 0x00; // 0x10 microphone mute, 0x40 audio mute

	// left trigger motor  
	outputReport[11] = 0; // right trigger motor mode (0 = no resistance, 1 = continuous resistance, 2 = section resistance / PS Remote Play defaults this to 5; bit 4 only disables the motor?)
	outputReport[12] = 0; // right trigger start of resistance section (0-255; 0 = released state; 0xb0 roughly matches trigger value 0xff)
	outputReport[13] = 0; // right trigger
		// (mode1) amount of force exerted; 0-255
		// (mode2) end of resistance section (>= begin of resistance section is enforced); 0xff makes it behave like mode1
	outputReport[14] = 0; // right trigger force exerted in range (mode2), 0-255

	outputReport[22] = 0;// left trigger motor mode (0 = no resistance, 1 = continuous resistance, 2 = section resistance / PS Remote Play defaults this to 5; bit 4 only disables the motor?)
	outputReport[23] = 0; // left trigger start of resistance section 0-255 (0 = released state; 0xb0 roughly matches trigger value 0xff)
	outputReport[24] = 0; // left trigger
		// (mode1) amount of force exerted; 0-255
		// (mode2) end of resistance section (>= begin of resistance section is enforced); 0xff makes it behave like mode1
	outputReport[25] = 0; // left trigger: (mode2) amount of force exerted within range; 0-255

	outputReport[38] = 0x07; // volume of internal speaker (0-7; ties in with index 6)

	// Uninterruptable Pulse LED setting (requires LED setting flag)
	outputReport[39] = 0; // 2 = blue LED pulse (together with index 42)
	outputReport[42] = 0; // pulse option
	/*1 = slowly(2s) fade to blue(scheduled to when the regular LED settings are active)
		2 = slowly(2s) fade out(scheduled after fade - in completion) with eventual switch back to configured LED color; only a fade - out can cancel the pulse(neither index 2, 0x08, nor turning this off will cancel it!)
		*/
		// Regular LED settings (requires LED setting flag)
	outputReport[44] = 0; // 5 white player indicator LEDs below the touchpad (bitmask 00-1f from left to right with 0x04 being the center LED)
	
	//outputReport[45] = 144; // Red value of light bars left and right from touchpad
	//outputReport[46] = 144; // Green value of light bars left and right from touchpad
	//outputReport[47] = 144; // Blue value of light bars left and right from touchpad

	outputReport[45] = NexOutputState.LEDRed;
	outputReport[46] = NexOutputState.LEDGreen;
	outputReport[47] = NexOutputState.LEDBlue;

	return outputReport;
}