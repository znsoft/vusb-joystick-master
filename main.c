/* Minimal V-USB joystick example. Runs on USBasp hardware.

Copyright (C) 2014 Shay Green
Licensed under GPL v2 or later. See License.txt. */
//#define USB_CFG_LONG_TRANSFERS	1
//#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH 42




#define USE_FORCEFEEDBACK 1
#define USE_YPOS 1
#define USE_XPOS 1
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#include "usbdrv/usbdrv.h"
#include "rotary.h"


/*  first ver work
PROGMEM const char usbHidReportDescriptor [] = {
	0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,     // USAGE (Game Pad)
	0xa1, 0x01,     // COLLECTION (Application)
	 0x09, 0x01,     //   USAGE (Pointer)
	 0xa1, 0x00,     //   COLLECTION (Physical)
	  0x09, 0x30,     //     USAGE (X)
	  0x09, 0x31,     //     USAGE (Y)               
	  0x15, 0x81,     //   LOGICAL_MINIMUM (-127)
	  0x25, 0x7f,     //   LOGICAL_MAXIMUM (127)
	  0x75, 0x08,     //   REPORT_SIZE (8)
	  0x95, 0x02,     //   REPORT_COUNT (2)
	  0x81, 0x02,     //   INPUT (Data,Var,Abs)
	 0xc0,           // END_COLLECTION 

	
	 0x05, 0x09,     // USAGE_PAGE (Button)
	 0x19, 0x01,     //   USAGE_MINIMUM (Button 1)
	 0x29, 0x08,     //   USAGE_MAXIMUM (Button 8)
	 0x15, 0x00,     //   LOGICAL_MINIMUM (0)
	 0x25, 0x01,     //   LOGICAL_MAXIMUM (1)
	 0x75, 0x01,     // REPORT_SIZE (1)
	 0x95, 0x08,     // REPORT_COUNT (8)
	 0x81, 0x02,     // INPUT (Data,Var,Abs)   // 16 bytes
	0xc0 ,           // END_COLLECTION	42 bytes */


// X/Y joystick w/ 8-bit readings (-127 to +127), 8 digital buttons 42 bytes+19  USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
#define ReportIndex 0
#define ReportNumber 1
#define Xindex ReportIndex+2
#define Yindex ReportIndex+1

PROGMEM const char usbHidReportDescriptor [] = {

	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x15, 0x00,        // Logical Minimum (0)
0x09, 0x04,        // Usage (Joystick)
0xA1, 0x01,        // Collection (Application)

0x85, 0x01,        //   Report ID (1)
0x05, 0x02,        //   Usage Page (Sim Ctrls)
0x09, 0xBB,        //   Usage (Throttle)  //  0
0x15, 0x81,        //   Logical Minimum (-127)
0x25, 0x7F,        //   Logical Maximum (127)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)  // 0
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x01,        //   Usage (Pointer)
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x30,        //     Usage (X)      //1
0x09, 0x31,        //     Usage (Y)      //2
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection



0x09, 0x39,        //   Usage (Hat switch)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x03,        //   Logical Maximum (3)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x0E, 0x01,  //   Physical Maximum (270)
0x66, 0x14, 0x00,  //   Unit (System: English Rotation, Length: Centimeter)
0x75, 0x04,        //   Report Size (4)
0x95, 0x01,        //   Report Count (1) //3
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x45, 0x00,        //   Physical Maximum (0)
0x66, 0x00, 0x00,  //   Unit (None)


0x05, 0x09,        //   Usage Page (Button)
0x19, 0x01,        //   Usage Minimum (0x01)
0x29, 0x04,        //   Usage Maximum (0x04)
0x25, 0x01,        //   Logical Maximum (1)
0x95, 0x04,        //   Report Count (4)
0x75, 0x01,        //   Report Size (1)   //4
0x35, 0x00,        //   Physical Minimum (0)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

0x05, 0x0F,        //   Usage Page (PID Page)
0x09, 0x21,        //   Usage (0x21)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x22,        //     Usage (0x22)
0x25, 0x7F,        //     Logical Maximum (127)
0x75, 0x07,        //     Report Size (7)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x25,        //     Usage (0x25)
0xA1, 0x02,        //     Collection (Logical)
0x09, 0x26,        //       Usage (0x26)
0x09, 0x27,        //       Usage (0x27)
0x09, 0x30,        //       Usage (0x30)
0x09, 0x31,        //       Usage (0x31)
0x09, 0x32,        //       Usage (0x32)
0x09, 0x33,        //       Usage (0x33)
0x09, 0x34,        //       Usage (0x34)
0x09, 0x40,        //       Usage (0x40)
0x09, 0x41,        //       Usage (0x41)
0x09, 0x42,        //       Usage (0x42)
0x15, 0x01,        //       Logical Minimum (1)
0x25, 0x0A,        //       Logical Maximum (10)
0x75, 0x08,        //       Report Size (8)
0x91, 0x00,        //       Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //     End Collection
0x09, 0x50,        //     Usage (0x50)
0x09, 0x54,        //     Usage (0x54)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0x10, 0x27,  //     Logical Maximum (10000)
0x46, 0x10, 0x27,  //     Physical Maximum (10000)
0x75, 0x10,        //     Report Size (16)
0x66, 0x03, 0x10,  //     Unit (System: English Linear, Time: Seconds)
0x55, 0x0D,        //     Unit Exponent (-3)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x55, 0x0A,        //     Unit Exponent (-6)
0x09, 0x51,        //     Usage (0x51)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x45, 0x00,        //     Physical Maximum (0)
0x55, 0x00,        //     Unit Exponent (0)
0x65, 0x00,        //     Unit (None)
0x09, 0x52,        //     Usage (0x52)
0x09, 0x53,        //     Usage (0x53)
0x25, 0x7F,        //     Logical Maximum (127)
0x75, 0x08,        //     Report Size (8)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x55,        //     Usage (0x55)
0xA1, 0x02,        //     Collection (Logical)
0x05, 0x01,        //       Usage Page (Generic Desktop Ctrls)
0x09, 0x01,        //       Usage (Pointer)
0xA1, 0x00,        //       Collection (Physical)
0x09, 0x30,        //         Usage (X)
0x09, 0x31,        //         Usage (Y)
0x25, 0x01,        //         Logical Maximum (1)
0x75, 0x01,        //         Report Size (1)
0x95, 0x02,        //         Report Count (2)
0x91, 0x02,        //         Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //       End Collection
0xC0,              //     End Collection
0x95, 0x06,        //     Report Count (6)
0x91, 0x03,        //     Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x05, 0x0F,        //     Usage Page (PID Page)
0x09, 0x57,        //     Usage (0x57)
0xA1, 0x02,        //     Collection (Logical)
0x05, 0x01,        //       Usage Page (Generic Desktop Ctrls)
0x09, 0x01,        //       Usage (Pointer)
0xA1, 0x00,        //       Collection (Physical)
0x09, 0x30,        //         Usage (X)
0x09, 0x31,        //         Usage (Y)
0x15, 0x00,        //         Logical Minimum (0)
0x26, 0xFF, 0x00,  //         Logical Maximum (255)
0x46, 0x68, 0x01,  //         Physical Maximum (360)
0x66, 0x14, 0x00,  //         Unit (System: English Rotation, Length: Centimeter)
0x75, 0x08,        //         Report Size (8)
0x95, 0x02,        //         Report Count (2)
0x91, 0x02,        //         Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x65, 0x00,        //         Unit (None)
0x45, 0x00,        //         Physical Maximum (0)
0xC0,              //       End Collection
0xC0,              //     End Collection
0x05, 0x0F,        //     Usage Page (PID Page)
0x09, 0x58,        //     Usage (0x58)
0xA1, 0x02,        //     Collection (Logical)
0x0B, 0x01, 0x00, 0x0A, 0x00,  //       Usage (0x0A0001)
0x0B, 0x02, 0x00, 0x0A, 0x00,  //       Usage (0x0A0002)
0x26, 0xFD, 0x7F,  //       Logical Maximum (32765)
0x75, 0x10,        //       Report Size (16)
0x95, 0x02,        //       Report Count (2)
0x91, 0x02,        //       Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //     End Collection
0xC0,              //   End Collection
0x09, 0x5A,        //   Usage (0x5A)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x02,        //     Report ID (2)
0x09, 0x23,        //     Usage (0x23)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x75, 0x0F,        //     Report Size (15)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x5B,        //     Usage (0x5B)
0x09, 0x5D,        //     Usage (0x5D)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x5C,        //     Usage (0x5C)
0x09, 0x5E,        //     Usage (0x5E)
0x26, 0x10, 0x27,  //     Logical Maximum (10000)
0x46, 0x10, 0x27,  //     Physical Maximum (10000)
0x66, 0x03, 0x10,  //     Unit (System: English Linear, Time: Seconds)
0x55, 0x0D,        //     Unit Exponent (-3)
0x75, 0x10,        //     Report Size (16)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x45, 0x00,        //     Physical Maximum (0)
0x65, 0x00,        //     Unit (None)
0x55, 0x00,        //     Unit Exponent (0)
0xC0,              //   End Collection
0x09, 0x5F,        //   Usage (0x5F)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x03,        //     Report ID (3)
0x09, 0x23,        //     Usage (0x23)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x75, 0x0F,        //     Report Size (15)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x60,        //     Usage (0x60)
0x09, 0x61,        //     Usage (0x61)
0x09, 0x62,        //     Usage (0x62)
0x09, 0x63,        //     Usage (0x63)
0x09, 0x64,        //     Usage (0x64)
0x09, 0x65,        //     Usage (0x65)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x06,        //     Report Count (6)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x6E,        //   Usage (0x6E)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x04,        //     Report ID (4)
0x09, 0x23,        //     Usage (0x23)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x75, 0x0F,        //     Report Size (15)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x70,        //     Usage (0x70)
0x09, 0x6F,        //     Usage (0x6F)
0x09, 0x71,        //     Usage (0x71)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x03,        //     Report Count (3)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x72,        //     Usage (0x72)
0x26, 0x10, 0x27,  //     Logical Maximum (10000)
0x46, 0x10, 0x27,  //     Physical Maximum (10000)
0x66, 0x03, 0x10,  //     Unit (System: English Linear, Time: Seconds)
0x55, 0x0D,        //     Unit Exponent (-3)
0x75, 0x10,        //     Report Size (16)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x45, 0x00,        //     Physical Maximum (0)
0x65, 0x00,        //     Unit (None)
0x55, 0x00,        //     Unit Exponent (0)
0xC0,              //   End Collection
0x09, 0x73,        //   Usage (0x73)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x05,        //     Report ID (5)
0x09, 0x23,        //     Usage (0x23)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x75, 0x0F,        //     Report Size (15)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x70,        //     Usage (0x70)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x74,        //   Usage (0x74)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x06,        //     Report ID (6)
0x09, 0x23,        //     Usage (0x23)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x75, 0x0F,        //     Report Size (15)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x75,        //     Usage (0x75)
0x09, 0x76,        //     Usage (0x76)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x02,        //     Report Count (2)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x68,        //   Usage (0x68)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x07,        //     Report ID (7)
0x09, 0x23,        //     Usage (0x23)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x75, 0x0F,        //     Report Size (15)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x0B, 0x3B, 0x00, 0x01, 0x00,  //     Usage (0x01003B)
0x26, 0x00, 0x01,  //     Logical Maximum (256)
0x75, 0x09,        //     Report Size (9)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x69,        //     Usage (0x69)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x96, 0x00, 0x01,  //     Report Count (256)
0x92, 0x02, 0x01,  //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
0xC0,              //   End Collection
0x09, 0x66,        //   Usage (0x66)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x08,        //     Report ID (8)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x01,        //     Usage (Pointer)
0xA1, 0x02,        //     Collection (Logical)
0x09, 0x30,        //       Usage (X)
0x09, 0x31,        //       Usage (Y)
0x15, 0x81,        //       Logical Minimum (-127)
0x25, 0x7F,        //       Logical Maximum (127)
0x75, 0x08,        //       Report Size (8)
0x95, 0x02,        //       Report Count (2)
0x91, 0x02,        //       Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //     End Collection
0xC0,              //   End Collection
0x05, 0x0F,        //   Usage Page (PID Page)
0x09, 0x6B,        //   Usage (0x6B)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x09,        //     Report ID (9)
0x09, 0x23,        //     Usage (0x23)
0x09, 0x6C,        //     Usage (0x6C)
0x09, 0x6D,        //     Usage (0x6D)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x95, 0x03,        //     Report Count (3)
0x75, 0x10,        //     Report Size (16)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x77,        //   Usage (0x77)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x0A,        //     Report ID (10)
0x09, 0x22,        //     Usage (0x22)
0x25, 0x7F,        //     Logical Maximum (127)
0x75, 0x07,        //     Report Size (7)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x78,        //     Usage (0x78)
0xA1, 0x02,        //     Collection (Logical)
0x09, 0x79,        //       Usage (0x79)
0x09, 0x7A,        //       Usage (0x7A)
0x09, 0x7B,        //       Usage (0x7B)
0x15, 0x01,        //       Logical Minimum (1)
0x25, 0x03,        //       Logical Maximum (3)
0x75, 0x08,        //       Report Size (8)
0x91, 0x00,        //       Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //     End Collection
0x09, 0x7C,        //     Usage (0x7C)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x7F,        //   Usage (0x7F)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x01,        //     Report ID (1)
0x09, 0x80,        //     Usage (0x80)
0x09, 0x81,        //     Usage (0x81)
0x09, 0x82,        //     Usage (0x82)
0x26, 0xFD, 0x7F,  //     Logical Maximum (32765)
0x95, 0x03,        //     Report Count (3)
0x75, 0x10,        //     Report Size (16)
0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0xA8,        //     Usage (0xA8)
0xA1, 0x02,        //     Collection (Logical)
0x09, 0x21,        //       Usage (0x21)
0x09, 0x5A,        //       Usage (0x5A)
0x09, 0x5F,        //       Usage (0x5F)
0x09, 0x6E,        //       Usage (0x6E)
0x09, 0x73,        //       Usage (0x73)
0x09, 0x74,        //       Usage (0x74)
0x09, 0x6B,        //       Usage (0x6B)
0x26, 0xFF, 0x00,  //       Logical Maximum (255)
0x75, 0x08,        //       Report Size (8)
0x95, 0x07,        //       Report Count (7)
0xB1, 0x02,        //       Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //     End Collection
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x07,        //     Report Size (7)
0x95, 0x01,        //     Report Count (1)
0xB1, 0x03,        //     Feature (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x67,        //     Usage (0x67)
0x75, 0x01,        //     Report Size (1)
0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x09, 0x92,        //   Usage (0x92)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x02,        //     Report ID (2)
0x09, 0x22,        //     Usage (0x22)
0x25, 0x7F,        //     Logical Maximum (127)
0x75, 0x07,        //     Report Size (7)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x24,        //     Usage (0x24)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x01,        //     Report Count (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x94,        //     Usage (0x94)
0x09, 0xA0,        //     Usage (0xA0)
0x09, 0xA4,        //     Usage (0xA4)
0x09, 0xA6,        //     Usage (0xA6)
0x75, 0x01,        //     Report Size (1)
0x95, 0x04,        //     Report Count (4)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0x09, 0x95,        //   Usage (0x95)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x0B,        //     Report ID (11)
0x09, 0x96,        //     Usage (0x96)
0xA1, 0x02,        //     Collection (Logical)
0x09, 0x97,        //       Usage (0x97)
0x09, 0x98,        //       Usage (0x98)
0x09, 0x99,        //       Usage (0x99)
0x09, 0x9A,        //       Usage (0x9A)
0x09, 0x9B,        //       Usage (0x9B)
0x09, 0x9C,        //       Usage (0x9C)
0x15, 0x01,        //       Logical Minimum (1)
0x25, 0x06,        //       Logical Maximum (6)
0x75, 0x01,        //       Report Size (1)
0x95, 0x08,        //       Report Count (8)
0x91, 0x02,        //       Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //     End Collection
0xC0,              //   End Collection
0x09, 0x85,        //   Usage (0x85)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x0C,        //     Report ID (12)
0x09, 0x86,        //     Usage (0x86)
0x09, 0x87,        //     Usage (0x87)
0x09, 0x88,        //     Usage (0x88)
0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
0x75, 0x10,        //     Report Size (16)
0x95, 0x03,        //     Report Count (3)
0x92, 0x02, 0x01,  //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
0xC0,              //   End Collection
0x09, 0x7D,        //   Usage (0x7D)
0xA1, 0x02,        //   Collection (Logical)
0x85, 0x02,        //     Report ID (2)
0x09, 0x7E,        //     Usage (0x7E)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0xC0,              // End Collection
// 862 bytes

};




//#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH sizeof(usbHidReportDescriptor)

#define reportLen 5
// Report format: Thrott, X, Y, hat pos, buttons (up to 8)
static uint8_t report [reportLen]; // current
static uint8_t report_out [reportLen]; // last sent over USB

static long Xpos = 0;
static long Ypos = 0;
static int divider= 2;
static int multiplier= 1;
volatile int8_t adc; 
#define YPIN1 PC5

#define BUTTON0 (_BV(0))
#define BUTTON1 (_BV(1))
#define BUTTON2 (_BV(2))
#define BUTTON3 (_BV(3))
#define BUTTON4 (_BV(4))
#define BUTTON5 (_BV(5))
#define BUTTON6 (_BV(6))
#define BUTTON7 (_BV(7))


//motor pin l298n
#define MOTORPWMPIN ( _BV(6))
#define MOTORPIN1 ( _BV(5))
#define MOTORPIN2 ( _BV(7))
//motor controll l298n
#define PWM(x) OCR2A=x 
#define MOTOROFF PORTD &= ~MOTORPWMPIN 
#define MOTORON  PORTD |=  MOTORPWMPIN 
#define MOTORCCW PORTD &= ~MOTORPIN1;PORTD |= MOTORPIN2 
#define MOTORCW  PORTD &= ~MOTORPIN2;PORTD |= MOTORPIN1 



//Програма инициализации ШИМ
static void init_pwm (void)
{

DDRD |= MOTORPWMPIN;
    // PD6 is now an output

    PWM(0x00);			//Начальная Мощность нулевая


    TCCR2A |= (1 << COM2A1);
    // set none-inverting mode

    TCCR2A |= (1 << WGM21) | (1 << WGM20);
    // set fast PWM Mode

    TCCR2B |= (1 << CS21);
    // set prescaler to 8 and starts PWM

  
  MOTOROFF;
 // TCCR1A=(1<<COM1A1)|(1<<WGM10); //На выводе OC1A единица, когда OCR1A==TCNT1, восьмибитный ШИМ
 // TCCR1B=(1<<CS10);		 //Делитель= /1

}

static void init_motor(void){
	init_pwm ();

	DDRD  |= MOTORPIN1;
	DDRD  |= MOTORPIN2;
	MOTORCCW;
}



void startadc(uchar adctouse)
{

	ADMUX	&=	0xf0;
    ADMUX |= adctouse;         // use #1 ADC
    ADMUX |= _BV(REFS0);    // use AVcc as the reference
    ADMUX &= ~_BV( ADLAR);   // clear for 10 bit resolution
    ADCSRA |= _BV( ADPS2) | _BV( ADPS1) | _BV( ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= _BV( ADEN);    // Enable the ADC
    ADCSRA |= _BV(ADIE);

	ADCSRA |= _BV(ADSC); // starts conversion
}

ISR(ADC_vect) //подпрограмма обработки прерывания от АЦП
{
	int ADCval;
	ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval; 
	adc=ADCval>>2; //считываем значение с АЦП и преобразуем его в значение от 0 до 8
	ADCSRA |= _BV(ADSC); // starts conversion
}

static void init_joy( void )
{

	

}


int _adc(uchar adctouse)
{
    int ADCval;

    ADMUX = adctouse;         // use #1 ADC
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish 


    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    return ADCval;
}


//#define ReportIndex 0
//#define ReportNumber 1
//#define Xindex ReportIndex+2
//#define Yindex ReportIndex+1

static void read_joy( void )
{
	report [ReportIndex] = ReportNumber;
	//report [1] = 0;
	//report [2] = 0;
	//calcEncode();
	
	int8_t dx = encode_read1();
	//report [0] = dx;
	if(dx!=0){
		Xpos +=(int)dx;
		int Xp = (multiplier * Xpos) / divider;

		if(Xp<127&&Xp>-127){
		report [Xindex] = (int8_t)Xp;
		#ifdef USE_FORCEFEEDBACK
		PWM(0x00);MOTOROFF;
		#endif
		}
		#ifdef USE_FORCEFEEDBACK
		if(Xp<-127){MOTORON;MOTORCW;PWM(-(Xp+127)*48);}
		if(Xp>127){MOTORON;MOTORCCW;PWM((Xp-127)*48);}
		#endif
			}
	
	#ifdef USE_YPOS
	Ypos = 127 - adc;

	report [Yindex] = (int8_t)(Ypos);
	#endif
	
	// Buttons

	//if ( ! (PINB & 0x01) ) report [2] |= 0x04;
	//if ( ! (PINB & 0x04) ) report [2] |= 0x08;

	
	
	
	/*
	 if(encode_readKey()==1){
	
	 	divider = 1+(divider&3);
	 	multiplier = divider-1;
	 	if(multiplier<1)multiplier=1;
    
    }
	 */

}





static uchar    idleRate;           /* in 4 ms units */
char _awaitReport;

usbMsgLen_t  usbFunctionSetup( uint8_t data [8] )
{

	usbRequest_t const* rq = (usbRequest_t const*) data;

	if ( (rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS )
		return 0;
	
	switch ( rq->bRequest )
	{
	case USBRQ_HID_GET_REPORT: // HID joystick only has to handle this
		usbMsgPtr = (usbMsgPtr_t) report_out;
		return sizeof report_out;
	
	case USBRQ_HID_SET_REPORT: // LEDs on joystick?
	
	 _awaitReport = 1;
		return USB_NO_MSG;
	default:
		return 0;
	}
	
	

	
}


uchar   usbFunctionWrite(uchar *data, uchar len)
{
   if (!_awaitReport || len < 1)
      return 1;
   if(data[0]<127){MOTORON;MOTORCCW;PWM((data[0]));} ;
   if(data[0]>127){MOTORON;MOTORCW;PWM(127-(data[0]));} ;
   _awaitReport = 0;
    return 1;
}


int main( void )
{
	encode_init();
	startadc(YPIN1);
	usbInit();
	sei();
	
	init_joy();
	#ifdef USE_FORCEFEEDBACK
	init_motor();
	#endif
	for ( ;; )
	{
		usbPoll();
		
		// Don't bother reading joy if previous changes haven't gone out yet.
		// Forces delay after changes which serves to debounce controller as well.
		if ( usbInterruptIsReady() )
		{
			read_joy();

			
			// Don't send update unless joystick changed
			if ( memcmp( report_out, report, sizeof report ) )
			{
				memcpy( report_out, report, sizeof report );
				usbSetInterrupt( report_out, sizeof report_out );
				//toggle_led();
			}
		}
	}
	
	return 0;
}