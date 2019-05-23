    float humidity;
    ATMO_ESSI2C_GetHumidity(&humidity);
    ATMO_CreateValueFloat(out, humidity);
    return ATMO_Status_Success;
