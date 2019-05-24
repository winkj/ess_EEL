// This is an adaption of:
//   https://github.com/Sensirion/arduino-ess/blob/master/sensirion_ess.cpp
//
// Based on IR Thermoclick example
//
// Author: Johannes Winkelmann, jw@smts.ch
//

#include "essi2c.h"

#define ESS_SHT_ADDR (0x70)
#define ESS_SGP_ADDR (0x58)
#define ESS_SHT_CMD_LENGTH (2)
#define ESS_SHT_DATA_LENGTH (6)

#define ESS_SHT_UPDATE_THRESHOLD_MS 800
#define ESS_SGP_UPDATE_THRESHOLD_MS 800

#define ESS_SHT_MEASUREMENT_DELAY 15
#define ESS_SGP_MEASUREMENT_DELAY 50

static uint64_t sLastSHTUpdateMs = 0;
static uint64_t sLastSGPUpdateMs = 0;

static uint16_t sCo2eqCache = 0;
static uint16_t sTVocCache = 0;

static float sHumidityCache = 0;
static float sTemperatureCache = 0;

typedef struct {
    ATMO_ESSI2C_Config_t config;
    bool configured;
} ATMO_ESSI2C_Priv_Config;

static ATMO_ESSI2C_Priv_Config _ATMO_ESSI2C_config;

static ATMO_I2C_Peripheral_t _ATMO_ESSI2C_i2cConfig = {
    .operatingMode = ATMO_I2C_OperatingMode_Master,
    .baudRate = ATMO_I2C_BaudRate_Standard_Mode
};

ATMO_ESSI2C_Status_t ATMO_ESSI2C_InitSGP_Internal();
ATMO_ESSI2C_Status_t ATMO_ESSI2C_updateSHT_Internal();
ATMO_ESSI2C_Status_t ATMO_ESSI2C_updateSGP_Internal();


ATMO_ESSI2C_Status_t ATMO_ESSI2C_Init(ATMO_ESSI2C_Config_t *config)
{
    if (config) { // Did the user supply a configuration?
        ATMO_ESSI2C_SetConfiguration(config);
    } else {
        _ATMO_ESSI2C_config.configured = false;
    }

    if (ATMO_ESSI2C_InitSGP_Internal() != ATMO_ESSI2C_Status_Success) {
      return ATMO_ESSI2C_Status_Fail;
    }

    // initial calls to update functions to set lastUpdate variables
    ATMO_ESSI2C_updateSHT_Internal();
    ATMO_ESSI2C_updateSGP_Internal();

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

ATMO_ESSI2C_Status_t ATMO_ESSI2C_ReadData_Internal(uint8_t addr, uint8_t* cmd, uint16_t cmdLength,
          uint8_t* data, uint16_t dataLength, uint16_t measurementDelay)
{
  ATMO_I2C_Status_t readStatus;
  readStatus = ATMO_I2C_MasterWrite(_ATMO_ESSI2C_config.config.i2cDriverInstance,
                                    addr, cmd, cmdLength,
                                    NULL, 0,
                                    0);
  if (readStatus != ATMO_I2C_Status_Success) {
      return ATMO_ESSI2C_Status_Fail;
  }

  ATMO_DelayMillisecondsNonBlock(measurementDelay);

  readStatus = ATMO_I2C_MasterRead(_ATMO_ESSI2C_config.config.i2cDriverInstance,
                                  addr, NULL, 0, data, dataLength, 0);
  if (readStatus != ATMO_I2C_Status_Success) {
      return ATMO_ESSI2C_Status_Fail;
  }

  return ATMO_ESSI2C_Status_Success;
}

uint8_t ATMO_ESSI2C_CheckCrc_Internal(const uint8_t* data, uint8_t len)
{
    // adapted from SHT21 sample code from http://www.sensirion.com/en/products/humidity-temperature/download-center/

    uint8_t crc = 0xff;
    uint8_t byteCtr;
    for (byteCtr = 0; byteCtr < len; ++byteCtr) {
        crc ^= (data[byteCtr]);
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

// - SHT
ATMO_ESSI2C_Status_t ATMO_ESSI2C_updateSHT_Internal()
{
  uint64_t now = ATMO_PLATFORM_UptimeMs();
  if ((now - sLastSHTUpdateMs) < ESS_SHT_UPDATE_THRESHOLD_MS) {
    // reuse cached values
    return ATMO_ESSI2C_Status_Success;
  }
  sLastSHTUpdateMs = now;

  if (!_ATMO_ESSI2C_config.configured) {
      return ATMO_ESSI2C_Status_Fail;
  }

  uint8_t data[ESS_SHT_DATA_LENGTH] = { 0 };
  uint8_t cmd[ESS_SHT_CMD_LENGTH] = { 0x78, 0x66 };
  if (ATMO_ESSI2C_ReadData_Internal(ESS_SHT_ADDR, cmd,
                                    ESS_SHT_CMD_LENGTH, data,
                                    ESS_SHT_DATA_LENGTH,
                                    ESS_SHT_MEASUREMENT_DELAY) != ATMO_ESSI2C_Status_Success) {
    return ATMO_ESSI2C_Status_Fail;
  }

  if (ATMO_ESSI2C_CheckCrc_Internal(data+0, 2) != data[2] ||
      ATMO_ESSI2C_CheckCrc_Internal(data+3, 2) != data[5]) {
    return ATMO_ESSI2C_Status_Fail;
  }

  //  mTemperature = mA + mB * (val / mC);
  uint16_t rawTemp = (data[0] << 8) | data[1];
  sTemperatureCache = (float)(-45 + (175.0 * rawTemp / 65535.0));

  //  mHumidity = mX * (val / mY);
  uint16_t rawHumidity = (data[3] << 8) | data[4];
  sHumidityCache = (float)(100.0 * rawHumidity / 65535.0);

  return ATMO_ESSI2C_Status_Success;
}

ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetTemperature(float *temperatureCelsius)
{
  if (ATMO_ESSI2C_updateSHT_Internal() != ATMO_ESSI2C_Status_Success) {
    return ATMO_ESSI2C_Status_Fail;
  }
  *temperatureCelsius = sTemperatureCache;

  return ATMO_ESSI2C_Status_Success;
}


ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetHumidity(float *humidity)
{
  if (ATMO_ESSI2C_updateSHT_Internal() != ATMO_ESSI2C_Status_Success) {
    return ATMO_ESSI2C_Status_Fail;
  }
  *humidity = sHumidityCache;

  return ATMO_ESSI2C_Status_Success;
}




// - SGP
ATMO_ESSI2C_Status_t ATMO_ESSI2C_InitSGP_Internal()
{
  uint8_t cmd[ESS_SHT_CMD_LENGTH] = { 0x20, 0x03 }; // init iaq
  ATMO_I2C_Status_t readStatus;
  readStatus = ATMO_I2C_MasterWrite(_ATMO_ESSI2C_config.config.i2cDriverInstance,
                                    ESS_SGP_ADDR, cmd, ESS_SHT_CMD_LENGTH,
                                    NULL, 0,
                                    0);
  if (readStatus != ATMO_I2C_Status_Success) {
      return ATMO_ESSI2C_Status_Fail;
  }

  return ATMO_ESSI2C_Status_Success;
}


ATMO_ESSI2C_Status_t ATMO_ESSI2C_updateSGP_Internal()
{
  uint64_t now = ATMO_PLATFORM_UptimeMs();
  if ((now - sLastSGPUpdateMs) < ESS_SGP_UPDATE_THRESHOLD_MS) {
    // reuse cached values
    return ATMO_ESSI2C_Status_Success;
  }
  sLastSGPUpdateMs = now;

  if (!_ATMO_ESSI2C_config.configured) {
      return ATMO_ESSI2C_Status_Fail;
  }

  uint8_t data[ESS_SHT_DATA_LENGTH] = { 0 };
  uint8_t cmd[ESS_SHT_CMD_LENGTH] = { 0x20, 0x08 };
  if (ATMO_ESSI2C_ReadData_Internal(ESS_SGP_ADDR, cmd,
                                    ESS_SHT_CMD_LENGTH, data,
                                    ESS_SHT_DATA_LENGTH,
                                    ESS_SGP_MEASUREMENT_DELAY) != ATMO_ESSI2C_Status_Success) {
    return ATMO_ESSI2C_Status_Fail;
  }

  if (ATMO_ESSI2C_CheckCrc_Internal(data+0, 2) != data[2] ||
      ATMO_ESSI2C_CheckCrc_Internal(data+3, 2) != data[5]) {
    return ATMO_ESSI2C_Status_Fail;
  }
  sTVocCache = (uint16_t)(data[3] << 8) | data[4];
  sCo2eqCache = (uint16_t)(data[0] << 8) | data[1];

  return ATMO_ESSI2C_Status_Success;
}

ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetTVoc(uint16_t *tVoc)
{
  if (ATMO_ESSI2C_updateSGP_Internal() != ATMO_ESSI2C_Status_Success) {
    return ATMO_ESSI2C_Status_Fail;
  }
  *tVoc = sTVocCache;
  return ATMO_ESSI2C_Status_Success;
}

ATMO_ESSI2C_Status_t ATMO_ESSI2C_GetCo2eq(uint16_t *co2eq)
{
  if (ATMO_ESSI2C_updateSGP_Internal() != ATMO_ESSI2C_Status_Success) {
    return ATMO_ESSI2C_Status_Fail;
  }
  *co2eq = sCo2eqCache;
  return ATMO_ESSI2C_Status_Success;
}
