/*

  ******************************************************************************
  * @file 			( фаил ):   LM75.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):	author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
  ******************************************************************************
  
*/

/* Includes ----------------------------------------------------------*/
#include "LM75.h"


/*
	******************************************************************************
	* @brief	 ( описание ):  функция (вспомогательная ) для чтения температуры ( LM75 9 bit ) из указанного регистра
	* @param	( параметры ):	регистр
	* @return  ( возвращает ):	температуру

	******************************************************************************
*/
float LM75_Temperature_9Bit_ReadReg( uint8_t reg ){
	
	// отправляем команду на чтение температуры-------
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, &reg, 1, 500 );
	//------------------------------------------------
	
	//-- для LM75B LM75C ( разрешение 9 бит )--------------------------------------------------
	// считываем 2 байта информации ( сама температыра хранится в 9 битах )
	// данные расположены таким образом:
	// биты D0 – D6: не используются. биты D7 – D15: данные о температуре. 
	// температура измеряется кротно 0,5°C ( младший бит при 1 = 0,5°C при 0 = 0,0°C  )
	// Один младший значащий бит = 0,5°C. Формат дополнения до двух.
	// сами данные о температуре это 8 бит типа INT ( так как есть и отрицательная )
	//-----------------------------------------------------------------------------------------
	
	uint16_t value;
	HAL_I2C_Master_Receive( &MY_I2C, LM75_ADDRESS, (uint8_t*)&value, 2, 500 );
	
	value = ((( value >> 8 ) | ( value << 8 )) >> 7) & 0x01FF;
	
	//если число отрицательное то 8 бит будет 1
	if( value & 0x0100 ){
		value = (0x01FE ^ value) + 2;	// инвертируем биты и +1 ( тем самым получаем из отрицательного числа положительное )	
		return (float)( value * (-0.5f) );
	}
	else{
		return (float)( value * 0.5f );
	}

}
//-----------------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  функция (вспомогательная ) для чтения температуры ( LM75A 11 bit ) из указанного регистра
	* @param	( параметры ):	регистр
	* @return  ( возвращает ):	температуру

	******************************************************************************
*/
float LM75_Temperature_11Bit_ReadReg( uint8_t reg ){
	
	// отправляем команду на чтение температуры-------
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, &reg, 1, 500 );
	//------------------------------------------------

	//-- для LM75A( разрешение 11 бит. но можно и 9 бит как в LM75B LM75C )--------------------
	// считываем 2 байта информации ( сама температыра хранится в 11 битах )
	// данные расположены таким образом:
	// биты D0 – D4: не используются. биты D5 – D15: данные о температуре. 
	// температура измеряется кротно 0,125°C ( старший бит при 1 = +°C при 0 = -°C  )
	// сами данные о температуре это 11 бит типа INT ( так как есть и отрицательная )
	//-----------------------------------------------------------------------------------------
	
	uint16_t value;
	HAL_I2C_Master_Receive( &MY_I2C, LM75_ADDRESS, (uint8_t*)&value, 2, 500 );
	
	value = ((( value >> 8 ) | ( value << 8 )) >> 5 ) & 0x07FF;

	// проверяем если 10 бит 1 ( значит у нас температура - )
	if(value & 0x04FF){
		value = (0x07FF ^ value) + 1;	// инвертируем биты и +1 ( тем самым получаем из отрицательного числа положительное )
		return  (float)(value * ( -0.125f ) );
	}
	// иначе ( значит у нас температура + )
	else{
		return  (float)(value * 0.125f );
	}
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  функция (вспомогательная ) для записи температуры в указаный регистр ( кротно 0,5°C )
	* @param	( параметры ):	регистр, температура
	* @return  ( возвращает ):	

	******************************************************************************
*/
static void LM75_TemperatureWriteReg( uint8_t reg, float Temperature ){
	
	// формируем 2 байта информации ( сама температыра хранится в 9 битах )
	// данные расположены таким образом:
	// биты D0 – D6: не используются. биты D7 – D15: данные о температуре. 
	// температура измеряется кротно 0,5°C ( младший бит при 1 = 0,5°C при 0 = 0,0°C  )
	// Один младший значащий бит = 0,5°C. Формат дополнения до двух.
	// сами данные о температуре это 8 бит типа INT ( так как есть и отрицательная )
	
	int8_t value[3];
	
	value[0] = reg;
	value[1] = Temperature;

	// проверяем если у нас температура + 0.5 
	if( ( ABS(Temperature) - ABS(value[1]) ) >= 0.5f ){
		value[2] = 0x80;
	}
	// иначе без десятков
	else{
		value[2] = 0x00;
	}
	
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, (uint8_t*)value, 3, 500 );
	
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  функция (вспомогательная ) для чтения конфигурации настройки датчика
	* @param	( параметры ):	
	* @return  ( возвращает ):	конфиг

	******************************************************************************
*/
static uint8_t LM75_ReadConfig(void){
	
	// регистр LM75_CONFIGURATION имеет 8 бит:
	// D0: Shutdown: Если установлено значение 1, LM75 переходит в режим отключения с низким энергопотреблением.
	// D1: Режим компаратора / прерывания: 0 - режим компаратора ( по умолчанию ), 1 - режим прерывания.
	// D2: О.С. Полярность: 0 - активный низкий уровень ( по умолчанию ), 1 - активный высокий уровень. является выходом с открытым стоком при любых условиях.
	// D3 – D4: Очередь отказов: количество отказов, которые необходимо обнаружить перед установкой O.S. выход, чтобы избежать ложного отключения из-за
	// к шуму. Неисправности обнаруживаются в конце преобразования. См. Указанное время преобразования температуры в
	//	D3 – D4 = 0x00 -> 1 ( по умолчанию )
	//	D3 – D4 = 0x01 -> 2 
	//	D3 – D4 = 0x02 -> 4 
	//	D3 – D4 = 0x03 -> 6 
	// D5 D6 D7 не используются ( заполняем 0 )
	
	// отправляем команду ----------------------------
	uint8_t reg = LM75_CONFIGURATION;
	HAL_I2C_Master_Transmit( &MY_I2C, LM75_ADDRESS, &reg, 1, 500 );
	//------------------------------------------------
	
	uint8_t conf;
	HAL_I2C_Master_Receive( &MY_I2C, LM75_ADDRESS, &conf, 1, 500 );
	
	return conf;
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  функция включения и выключения спящего режима датчика
	* @param	( параметры ):	передаваемый параметр ( 1 - включить спящий режим, 0- выключить )
								по умолчанию выключен спящий режим
	* @return  ( возвращает ):	

	******************************************************************************
*/
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
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  функция включения Режим компаратора / прерывания: 0 - режим компаратора ( по умолчанию ), 1 - режим прерывания.
	* @param	( параметры ):	передаваемый параметр ( 1 - режим прерывания, 0- режим компаратора ( по умолчанию ) )
								по умолчанию режим компаратора
	* @return  ( возвращает ):	

	******************************************************************************
*/
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
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  функция установки Полярности: 0 - активный низкий уровень ( по умолчанию ), 1 - активный высокий уровень. 
	* @param	( параметры ):	передаваемый параметр ( 1 - активный высокий уровень, 0- активный низкий уровень ( по умолчанию ) )
								по умолчанию активный низкий уровень
	* @return  ( возвращает ):	

	******************************************************************************
*/
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
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ): функция установки Очереди отказов: количество отказов, которые необходимо обнаружить перед установкой O.S. выход, чтобы избежать ложного отключения из-за шума
								сколько раз должен сработать переход через границу чтобы включилась ножка OS 
								D3 – D4 = 0x00 -> 1 ( по умолчанию )
								D3 – D4 = 0x01 -> 2 
								D3 – D4 = 0x02 -> 4 
								D3 – D4 = 0x03 -> 6 
	* @param	( параметры ):	передаваемые параметры 1, 2, 4, 6. 1 ( по умолчанию )
	* @return  ( возвращает ):	

	******************************************************************************
*/
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
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  Функция чтения температуры возвращает температуру кратную 0,5°C
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
float LM75_TemperatureRead(void){
	
	#ifdef LM75_11BIT
		return LM75_Temperature_11Bit_ReadReg( LM75_TEMPERATURE );
	#endif

	#ifdef LM75_9BIT
		return LM75_Temperature_9Bit_ReadReg( LM75_TEMPERATURE );
	#endif	
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ): функция чтение Температуры THYST (температура выключения пина OS) по умолчанию равна 75°C 
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
float LM75_THYST_Read(void){
	// используем только 9 битдаже если у нас LM75A 11Bit
	return LM75_Temperature_9Bit_ReadReg( LM75_THYST );
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ): функция чтение Температуры TOS (температура включения пина OS) по умолчанию равна 80°C 
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
float LM75_TOS_Read(void){
	// используем только 9 битдаже если у нас LM75A 11Bit
	return LM75_Temperature_9Bit_ReadReg( LM75_TOS );
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ): функция записи Температуры THYST (температура выключения пина OS) ( кратную 0,5°C ) по умолчанию равна 75°C 
								запись данных после сброса микросхемы сбрасывается и становится по умолчанию
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void LM75_THYST_Write(float TemperatureTHYST){
	
	LM75_TemperatureWriteReg( LM75_THYST, TemperatureTHYST );
}
//-----------------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ): функция записи Температуры TOS (температура включения пина OS) ( кратную 0,5°C ) по умолчанию равна 80°C 
								запись данных после сброса микросхемы сбрасывается и становится по умолчанию
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void LM75_TOS_Write(float TemperatureTOS){
	
	LM75_TemperatureWriteReg( LM75_TOS, TemperatureTOS );
}
//-----------------------------------------------------------------------------------------


/************************ (C) COPYRIGHT GKP *****END OF FILE****/