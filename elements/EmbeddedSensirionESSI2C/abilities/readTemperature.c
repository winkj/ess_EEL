    int32_t tempC;
    ATMO_ESSI2C_GetTemperature(&tempC);
    ATMO_CreateValueInt(out, tempC);
    return ATMO_Status_Success;
