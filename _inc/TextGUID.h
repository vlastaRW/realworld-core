#pragma once

constexpr inline bool AddBits(BYTE* buf, int& bufUsed, int bufMax, BYTE addData, int addBits)
{
    if (bufUsed + addBits > bufMax)
        return false;
    BYTE* p = buf + (bufUsed >> 3);
    int subUsed = bufUsed & 7;
    *p |= addData << subUsed;
    if (addBits + subUsed > 8)
        p[1] = addData >> (8 - subUsed);
    bufUsed += addBits;
    return true;
}

constexpr inline bool GUIDFromText(char const* name, GUID* res)
{
    if (name == nullptr || *name == '\0')
        return false;
    BYTE buf[16] = { 0 };
    int bits = 0;
    while (*name)
    {
        if (*name == '.')
        {
            if (!AddBits(buf, bits, 16 * 8, 1, 6))
                return false; // name too long
        }
        else if (*name >= '0' && *name <= '9')
        {
            if (!AddBits(buf, bits, 16 * 8, *name - '0' + 2, 6))
                return false; // name too long
        }
        else if (*name >= 'A' && *name <= 'Z')
        {
            if (!AddBits(buf, bits, 16 * 8, *name - 'A' + 10 + 2, 6))
                return false; // name too long
        }
        else if (*name >= 'a' && *name <= 'z')
        {
            if (!AddBits(buf, bits, 16 * 8, *name - 'a' + 26 + 10 + 2, 6))
                return false; // name too long
        }
        else
            return false; // unsupported character
        ++name;
    }
    if (!AddBits(buf, bits, 16 * 8, 0, 6)) // add terminator
        return false; // name too long
    // repeat data in buffer so that we don't have trailing/leading zeros (bad for hashing?)
    BYTE* p = buf;
    while (bits < 16 * 8)
    {
        AddBits(buf, bits, 16 * 8, *p, min(8, 16 * 8 - bits));
        ++p;
    }
    *res = GUID{ buf[15] + 256 * buf[14] + 256 * 256 * buf[13] + 256UL * 256 * 256 * buf[12], unsigned short(buf[11] + 256 * buf[10]), unsigned short(buf[9] + 256 * buf[8]), {buf[7], buf[6], buf[5], buf[4], buf[3], buf[2], buf[1], buf[0]} };
    return true;
}

inline GUID GUIDTooLongOrHasInvalidChars()
{
    return GUID_NULL;
}

constexpr inline GUID GUIDFromText(char const* name)
{
    GUID g{ 0 };
    if (GUIDFromText(name, &g))
        return g;
    return GUIDTooLongOrHasInvalidChars();
}

constexpr inline BYTE GetBits(BYTE const* buf, int& bufUsed, int bufMax, int getBits)
{
    buf += bufUsed >> 3;
    int used = bufUsed & 7;
    BYTE r = (*buf) >> used;
    if (used + getBits > 8)
        r |= buf[1] << (8 - used);
    bufUsed += getBits;
    return r & ((1 << getBits) - 1);
}

//constexpr std::string TextFromGUID(GUID id)
//{
//    char buf[64]{ 0 };
//    BYTE src[16] =
//    {
//        id.Data4[7], id.Data4[6], id.Data4[5], id.Data4[4], id.Data4[3], id.Data4[2], id.Data4[1], id.Data4[0],
//        BYTE(id.Data3 >> 8), BYTE(id.Data3), BYTE(id.Data2 >> 8), BYTE(id.Data2), BYTE(id.Data1 >> 24), BYTE(id.Data1 >> 16), BYTE(id.Data1 >> 8), BYTE(id.Data1),
//    };
//    int read = 0;
//    char* end = buf;
//    while (read + 6 <= 16 * 8)
//    {
//        BYTE b = GetBits(src, read, 16 * 8, 6);
//        if (b == 0)
//            break;
//        else if (b == 1)
//            *end = '.';
//        else if (b < 12)
//            *end = b - 2 + '0';
//        else if (b < 12 + 26)
//            *end = b - 12 + 'A';
//        else if (b < 12 + 26 + 26)
//            *end = b - 12 - 26 + 'a';
//        ++end;
//    }
//    return std::string{ buf, end };
//}

