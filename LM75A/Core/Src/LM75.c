
#include "LM75.h"

#define LM75_ADDRESS	((0x48 | 0x00) << 1 )

float LM75_Temperature_9Bit_ReadReg( uint8_t reg ){
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, &reg, 1, 500 );
	
	
	uint16_t value;
	HAL_I2C_Master_Receive( &MY_I2C, LM75_ADDRESS, (uint8_t*)&value, 2, 500 );
	
	value = ((( value >> 8 ) | ( value << 8 )) >> 7) & 0x01FF;
	

	if( value & 0x0100 ){
		value = (0x01FE ^ value) + 2;
		return (float)( value * (-0.5f) );
	}
	else{
		return (float)( value * 0.5f );
	}

}

float LM75_Temperature_11Bit_ReadReg( uint8_t reg ){
	

	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, &reg, 1, 500 );
	

	uint16_t value;
	HAL_I2C_Master_Receive( &MY_I2C, LM75_ADDRESS, (uint8_t*)&value, 2, 500 );
	
	value = ((( value >> 8 ) | ( value << 8 )) >> 5 ) & 0x07FF;


	if(value & 0x04FF){
		value = (0x07FF ^ value) + 1;
		return  (float)(value * ( -0.125f ) );
	}

	else{
		return  (float)(value * 0.125f );
	}
}

static void LM75_TemperatureWriteReg( uint8_t reg, float Temperature ){
	
	int8_t value[3];
	
	value[0] = reg;
	value[1] = Temperature;

	if( ( ABS(Temperature) - ABS(value[1]) ) >= 0.5f ){
		value[2] = 0x80;
	}

	else{
		value[2] = 0x00;
	}
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, (uint8_t*)value, 3, 500 );
	
}

static uint8_t LM75_ReadConfig(void){
	
	
	uint8_t reg = LM75_CONFIGURATION;
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, &reg, 1, 500 );
	
	uint8_t conf;
	HAL_I2C_Master_Receive( &MY_I2C, LM75_ADDRESS, &conf, 1, 500 );
	
	return conf;
}

void LM75_SleepMode(uint8_t mode){
	
	uint8_t buff[2];
	buff[0] = LM75_CONFIGURATION;
	buff[1] = LM75_ReadConfig();
	
	if( mode ){
		buff[1] = buff[1] | 0x01;
	}
	else{
		buff[1] = buff[1] & 0xFE;
	}
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, buff, 2, 500 );
}

void LM75_ComparatorOrInterruptMode(uint8_t mode){
	
	uint8_t buff[2];
	buff[0] = LM75_CONFIGURATION;
	buff[1] = LM75_ReadConfig();
	
	if( mode ){
		buff[1] = buff[1] | 0x02;
	}
	else{
		buff[1] = buff[1] & 0xFD;
	}
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, buff, 2, 500 );
}

void LM75_LevelOsMode(uint8_t mode){
	
	uint8_t buff[2];
	buff[0] = LM75_CONFIGURATION;
	buff[1] = LM75_ReadConfig();
	
	if( mode ){
		buff[1] = buff[1] | 0x04;
	}
	else{
		buff[1] = buff[1] & 0xFB;
	}
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, buff, 2, 500 );
}

void LM75_FaultQueueMode(uint8_t mode){
	
	uint8_t buff[2];
	buff[0] = LM75_CONFIGURATION;
	buff[1] = LM75_ReadConfig();
	
	if( mode == 1 ){
		buff[1] = buff[1] & 0xE7;
	}
	else if( mode == 2 ){
		buff[1] = (buff[1] & 0xE7) | 0x08;
	}
	else if( mode == 4 ){
		buff[1] = (buff[1] & 0xE7) | 0x10;
	}
	else if( mode == 6 ){
		buff[1] = (buff[1] & 0xE7) | 0x18;
	}
	else{
		buff[1] = buff[1] & 0xE7;
	}
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, buff, 2, 500 );
}

float LM75_TemperatureRead(void){
	
	#ifdef LM75_11BIT
		return LM75_Temperature_11Bit_ReadReg( LM75_TEMPERATURE );
	#endif

	#ifdef LM75_9BIT
		return LM75_Temperature_9Bit_ReadReg( LM75_TEMPERATURE );
	#endif	
}

float LM75_THYST_Read(void){

	return LM75_Temperature_9Bit_ReadReg( LM75_THYST );
}

float LM75_TOS_Read(void){

	return LM75_Temperature_9Bit_ReadReg( LM75_TOS );
}

void LM75_THYST_Write(float TemperatureTHYST){
	
	LM75_TemperatureWriteReg( LM75_THYST, TemperatureTHYST );
}

void LM75_TOS_Write(float TemperatureTOS){
	
	LM75_TemperatureWriteReg( LM75_TOS, TemperatureTOS );
}

