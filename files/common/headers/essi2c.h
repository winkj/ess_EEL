#ifndef _ATMO_ESSI2C_H_
#define _ATMO_ESSI2C_H_

#include "../app_src/atmosphere_platform.h"
#include "../i2c/i2c.h"

typedef enum {
    ATMO_ESSI2C_Status_Success              = 0x00u,  // Common - Operation was successful
    ATMO_ESSI2C_Status_Fail                 = 0x01u,  // Common - Operation failed
    ATMO_ESSI2C_Status_Initialized          = 0x02u,  // Common - Peripheral already initialized
    ATMO_ESSI2C_Status_Invalid              = 0x03u,  // Common - Invalid operation or result
    ATMO_ESSI2C_Status_NotSupported         = 0x04u,  // Common - Feature not supported by platform
} ATMO_ESSI2C_Status_t;

typedef struct {
    ATMO_DriverInstanceHandle_t i2cDriverInstance;
} ATMO_ESSI2C_Config_t;

/**
 * Initialize ESSI2C Driver
 *
 * @param[in] config - Device configuration (optional)
 */
ATMO_ESSI2C_Status_t ATMO_ESSI2C_Init(ATMO_ESSI2C_Config_t *config);

/**
 * Set basic device configuration
 *
 * @param[in] config
 */
ATMO_ESSI2C_Status_t ATMO_ESSI2C_SetConfiguration(const ATMO_ESSI2C_Config_t *config);

/**
 * Get device configuration
 *
 * @param[out] config
 */
ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetConfiguration(ATMO_ESSI2C_Config_t *config);

/**
 * Get Temperature in degrees celsius
 *
 * @param[out] temperature
 */
ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetTemperature(int32_t *temperatureCelsius);

#endif
