    uint16_t co2eq;
    ATMO_ESSI2C_GetCo2eq(&co2eq);
    ATMO_CreateValueInt(out, co2eq);
    return ATMO_Status_Success;
