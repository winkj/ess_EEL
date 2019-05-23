#include "essi2c.h"

#define ESS_SHT_ADDR (0x70)
#define ESS_SHT_CMD_LENGTH (2)
#define ESS_SHT_DATA_LENGTH (6)

typedef struct {
    ATMO_ESSI2C_Config_t config;
    bool configured;
} ATMO_ESSI2C_Priv_Config;

static ATMO_ESSI2C_Priv_Config _ATMO_ESSI2C_config;

static ATMO_I2C_Peripheral_t _ATMO_ESSI2C_i2cConfig = {
    .operatingMode = ATMO_I2C_OperatingMode_Master,
    .baudRate = ATMO_I2C_BaudRate_Standard_Mode
};

ATMO_ESSI2C_Status_t ATMO_ESSI2C_Init(ATMO_ESSI2C_Config_t *config)
{
    if (config) { // Did the user supply a configuration?
        ATMO_ESSI2C_SetConfiguration(config);
    } else {
        _ATMO_ESSI2C_config.configured = false;
    }
    return ATMO_ESSI2C_Status_Success;
}

ATMO_ESSI2C_Status_t ATMO_ESSI2C_SetConfiguration(const ATMO_ESSI2C_Config_t *config)
{
    if (config == NULL) {
        return ATMO_ESSI2C_Status_Fail;
    }

    if (ATMO_I2C_SetConfiguration(config->i2cDriverInstance, &_ATMO_ESSI2C_i2cConfig) != ATMO_I2C_Status_Success) {
        return ATMO_ESSI2C_Status_Fail;
    }
    memcpy( &_ATMO_ESSI2C_config.config, config, sizeof(ATMO_ESSI2C_Config_t) );
    _ATMO_ESSI2C_config.configured = true;

    return ATMO_ESSI2C_Status_Success;
}

ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetConfiguration(ATMO_ESSI2C_Config_t *config)
{
    if (config == NULL || !_ATMO_ESSI2C_config.configured) {
        return ATMO_ESSI2C_Status_Fail;
    }

    memcpy(config, &_ATMO_ESSI2C_config.config, sizeof(ATMO_ESSI2C_Config_t));
    return ATMO_ESSI2C_Status_Success;
}

ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetTemperature(int32_t *temperatureCelsius)
{
    if (!_ATMO_ESSI2C_config.configured) {
        return ATMO_ESSI2C_Status_Fail;
    }

    uint8_t data[ESS_SHT_DATA_LENGTH] = { 0 };
    uint8_t cmd[ESS_SHT_CMD_LENGTH] = { 0x78, 0x66 };
    ATMO_I2C_Status_t readStatus = ATMO_I2C_MasterRead(_ATMO_ESSI2C_config.config.i2cDriverInstance, ESS_SHT_ADDR,
                                                       cmd, ESS_SHT_CMD_LENGTH, data, ESS_SHT_DATA_LENGTH, 0);
    if (readStatus != ATMO_I2C_Status_Success) {
        return ATMO_ESSI2C_Status_Fail;
    }

    //  mTemperature = mA + mB * (val / mC);
    uint16_t rawTemp = (data[1] << 8) | data[0];
    *temperatureCelsius = (uint32_t)(-45 + (175.0 * rawTemp / 65535.0));

    return ATMO_ESSI2C_Status_Success;
}