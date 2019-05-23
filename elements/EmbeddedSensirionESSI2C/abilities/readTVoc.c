    uint16_t tVoc;
    ATMO_ESSI2C_GetTVoc(&tVoc);
    ATMO_CreateValueInt(out, tVoc);
    return ATMO_Status_Success;
