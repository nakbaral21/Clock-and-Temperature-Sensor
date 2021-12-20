#ifndef _LM75_H
#define _LM75_H

#ifdef __cplusplus
extern "C" {
	#endif


	#include "main.h"

	#define LM75_9BIT
		
	#define MY_I2C			hi2c1

	#define	LM75_TEMPERATURE		0x00
	#define LM75_CONFIGURATION		0x01
	#define LM75_THYST				0x02
	#define LM75_TOS				0x03



	#define ABS(x)   ((x) > 0 ? (x) : -(x))


	#ifdef MY_I2C
		extern I2C_HandleTypeDef MY_I2C;
	#endif


	float LM75_Temperature_9Bit_ReadReg( uint8_t reg );

	float LM75_Temperature_11Bit_ReadReg( uint8_t reg );

	static void LM75_TemperatureWriteReg( uint8_t reg, float Temperature );

	static uint8_t LM75_ReadConfig(void);

	void LM75_SleepMode(uint8_t mode);

	void LM75_ComparatorOrInterruptMode(uint8_t mode);

	void LM75_LevelOsMode(uint8_t mode);

	void LM75_FaultQueueMode(uint8_t mode);

	float LM75_TemperatureRead(void);

	float LM75_THYST_Read(void);

	float LM75_TOS_Read(void);

	void LM75_THYST_Write(float TemperatureTHYST);

	void LM75_TOS_Write(float TemperatureTOS);

	#ifdef __cplusplus
}
#endif

#endif
