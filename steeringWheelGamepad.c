#include "gamepad.h"




Gamepad steeringWheelGamepad = {
	.num_reports			=	1,
	.reportDescriptorSize	=	sizeof(sega_usbHidReportDescriptor),
	.init					=	Init,
	.update					=	Update,
	.changed				=	Changed,
	.buildReport			=	BuildReport
};


Gamepad *GetGamepad(void){
	

	return &steeringWheelGamepad;

}