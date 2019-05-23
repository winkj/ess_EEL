    float tempC;
    ATMO_ESSI2C_GetTemperature(&tempC);
    ATMO_CreateValueFloat(out, tempC);
    return ATMO_Status_Success;
