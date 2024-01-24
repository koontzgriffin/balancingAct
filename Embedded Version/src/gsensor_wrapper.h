#include "ADXL345.h"
#include <stdio.h>

#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include "address_map_arm_brl4.h"

uint8_t devid;
int16_t mg_per_lsb = 4;

int gsensor_initialized = 0;

void gsensor_init(int fd){
	Map_Physical_Addrs();
	Pinmux_Config();
	I2C0_Init();

	ADXL345_REG_READ(0x00, &devid);

	if (devid == 0xE5){
		printf("Device ID verified\n");
		ADXL345_Init();
		printf("ADXL345 Initialized\n");
		gsensor_initialized = 1;
	}
}

float get_board_angle(){
	int16_t XYZ[3];
	if (ADXL345_IsDataReady()){
		ADXL345_XYZ_Read(XYZ);
		int x = XYZ[0]*mg_per_lsb;
		float angle = 1.57*((float)x)/1000;
		return angle;
	}
	printf("ERROR: could not get board angle\n");
	return 0;
}
