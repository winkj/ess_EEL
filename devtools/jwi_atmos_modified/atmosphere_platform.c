#include "atmosphere_platform.h"
#include "../gpio/gpio_arduino.h"
#include "../i2c/i2c_arduino.h"
// #include "../spi/spi_arduino.h"
#include "../adc/adc_arduino.h"
#include "../block/block_arduino.h"
// #include "../cloud/cloud_ble.h"
// #include "../cloud/cloud_uart.h"
#include "../uart/uart_arduino.h"
#include "../interval/interval_arduino.h"
// #include "../filesystem/filesystem_crastfs.h"
#include "../block/block_arduino.h"
#include "memdebug.h"

static void ATMO_ARDUINO_GPIO_ISR_2(){_ATMO_ARDUINO_GPIO_ISR(2);}
static void ATMO_ARDUINO_GPIO_ISR_3(){_ATMO_ARDUINO_GPIO_ISR(3);}
static void ATMO_ARDUINO_GPIO_ISR_18(){_ATMO_ARDUINO_GPIO_ISR(18);}
static void ATMO_ARDUINO_GPIO_ISR_19(){_ATMO_ARDUINO_GPIO_ISR(19);}
static void ATMO_ARDUINO_GPIO_ISR_20(){_ATMO_ARDUINO_GPIO_ISR(20);}
static void ATMO_ARDUINO_GPIO_ISR_21(){_ATMO_ARDUINO_GPIO_ISR(21);}

static ATMO_ARDUINO_I2C_ISRHandler_t _MEGA_GPIO_ISR_Handles[] = {
	NULL,
	NULL,
	ATMO_ARDUINO_GPIO_ISR_2,
	ATMO_ARDUINO_GPIO_ISR_3,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ATMO_ARDUINO_GPIO_ISR_18,
	ATMO_ARDUINO_GPIO_ISR_19,
	ATMO_ARDUINO_GPIO_ISR_20,
	ATMO_ARDUINO_GPIO_ISR_21,
};

#define ATMO_MEGA_I2C_MAX_CHUNK_LEN (0)

static bool ATMO_MEGA_ADC_IsAnalogPinDefaultValidator(unsigned int pin)
{
	return (pin >= A0 && pin <= A15);
}

static void uptimePrinter(void *data)
{
	ATMO_PLATFORM_DebugPrint("Uptime: %d\r\n", millis() / 1000);
}

void ATMO_PLATFORM_Init() {
	ATMO_DriverInstanceHandle_t instanceNumber = 0;
	ATMO_DriverInstanceHandle_t blockInstanceNumber = 0;
	ATMO_DriverInstanceHandle_t fsInstanceNumber = 0;

	ATMO_ARDUINO_ADC_AddDriverInstance(&instanceNumber, ATMO_MEGA_ADC_IsAnalogPinDefaultValidator, 5.0, 1024);
	ATMO_ADC_Init(instanceNumber);

	ATMO_ARDUINO_INTERVAL_AddDriverInstance(&instanceNumber);
	ATMO_INTERVAL_Init(instanceNumber);

	ATMO_ARDUINO_GPIO_AddDriverInstance(&instanceNumber, _MEGA_GPIO_ISR_Handles, 22);
	ATMO_GPIO_Init(instanceNumber);

	ATMO_ARDUINO_I2C_AddDriverInstance(&instanceNumber, ATMO_MEGA_I2C_MAX_CHUNK_LEN);
	ATMO_I2C_Init(instanceNumber);

	// ATMO_ARDUINO_SPI_AddDriverInstance(&instanceNumber);
	// ATMO_SPI_Init(instanceNumber);

	ATMO_ARDUINO_BLOCK_AddDriverInstance(&blockInstanceNumber, 256);
	ATMO_BLOCK_Init(blockInstanceNumber);

	//ATMO_PLATFORM_DebugPrint("Initializing FS\r\n");
	//ATMO_CRASTFS_FILESYSTEM_AddDriverInstance( &fsInstanceNumber );
	//ATMO_FILESYSTEM_Init(fsInstanceNumber, blockInstanceNumber);
	//ATMO_PLATFORM_DebugPrint("FS Initialized\r\n");

	//ATMO_FILESYSTEM_Config_t fsConfig;
	//fsConfig.numRetries = 1;
	//fsConfig.retryDelayMs = 10;
	//ATMO_FILESYSTEM_SetConfiguration(fsInstanceNumber, &fsConfig);

	//ATMO_PLATFORM_DebugPrint("Init cloud FS");
	//ATMO_CLOUD_InitFilesystemData(0);
	//ATMO_PLATFORM_DebugPrint("Init cloud FS done");

	ATMO_ARDUINO_UART_AddDriverInstance(&instanceNumber);
	ATMO_UART_Init(instanceNumber);

	//ATMO_CLOUD_UART_AddDriverInstance(&instanceNumber, 0);
	//ATMO_CLOUD_UART_Init(instanceNumber);
}

void ATMO_PLATFORM_PostInit() {
}

void *ATMO_Malloc(uint32_t numBytes)
{
	// ATMO_PLATFORM_DebugPrint("Free Memory: %d Requesting %d\r\n", getFreeMemory(), numBytes);
	void *data = malloc(numBytes);

	if(data == NULL)
	{
		ATMO_PLATFORM_DebugPrint("MALLOC FAIL\r\n");
	}
	return data;
}

void ATMO_Lock()
{

}

void ATMO_Unlock()
{

}

void *ATMO_Calloc(size_t num, size_t size)
{
	return calloc(num, size);
}

void ATMO_Free(void *data)
{
	free(data);
}

void ATMO_PLATFORM_DelayMilliseconds(uint32_t milliseconds)
{
	delay(milliseconds);
}

uint64_t ATMO_PLATFORM_UptimeMs()
{
	return (uint64_t)millis();
}
