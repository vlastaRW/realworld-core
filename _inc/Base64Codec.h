
#pragma once

extern __declspec(selectany) char const s_tBase64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
extern __declspec(selectany) char const s_tBase64Decode[] =
{
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

template<typename TIterator, class TOutString>
inline void Base64Encode(TIterator a_begin, TIterator a_end, TOutString& a_tOut)
{
    TOutString::size_type i;
    char c;
	a_tOut.clear();
    TOutString::size_type  len = a_end-a_begin;
    TOutString& ret = a_tOut;

	ret.reserve(len * 2);

    for (i = 0; i < len; ++i)
    {
        c = (*(a_begin+i) >> 2) & 0x3f;
        ret.append(1, s_tBase64Encode[c]);
        c = (*(a_begin+i) << 4) & 0x3f;
        if (++i < len)
            c |= (*(a_begin+i) >> 4) & 0x0f;

        ret.append(1, s_tBase64Encode[c]);
        if (i < len)
        {
            c = (*(a_begin+i) << 2) & 0x3f;
            if (++i < len)
                c |= (*(a_begin+i) >> 6) & 0x03;

            ret.append(1, s_tBase64Encode[c]);
        }
        else
        {
            ++i;
            ret.append(1, '='); // fillchar
        }

        if (i < len)
        {
            c = *(a_begin+i) & 0x3f;
            ret.append(1, s_tBase64Encode[c]);
        }
        else
        {
            ret.append(1, '='); // fillchar
        }
    }
}

inline void Base64Encode(BYTE const* a_begin, BYTE const* a_end, char* a_out)
{
    ULONG i;
    char c;
    ULONG len = a_end-a_begin;

    for (i = 0; i < len; ++i)
    {
        c = (*(a_begin+i) >> 2) & 0x3f;
        *a_out++ = s_tBase64Encode[c];
        c = (*(a_begin+i) << 4) & 0x3f;
        if (++i < len)
            c |= (*(a_begin+i) >> 4) & 0x0f;

        *a_out++ = s_tBase64Encode[c];
        if (i < len)
        {
            c = (*(a_begin+i) << 2) & 0x3f;
            if (++i < len)
                c |= (*(a_begin+i) >> 6) & 0x03;

            *a_out++ = s_tBase64Encode[c];
        }
        else
        {
            ++i;
            *a_out++ = '='; // fillchar
        }

        if (i < len)
        {
            c = *(a_begin+i) & 0x3f;
            *a_out++ = s_tBase64Encode[c];
        }
        else
        {
            *a_out++ = '='; // fillchar
        }
    }
}


template<typename TIterator, class TOutString>
inline void Base64Decode(TIterator a_begin, TIterator a_end, TOutString& a_tOut)

{
    size_t  i;
    char               c;
    char               c1;
    size_t  len = a_end-a_begin;
    TOutString& ret = a_tOut;

	ret.reserve(len);

    for (i = 0; i < len; ++i)
    {
        c = (char) s_tBase64Decode[static_cast<unsigned char>(*(a_begin+i))];
        ++i;
        c1 = (char) s_tBase64Decode[static_cast<unsigned char>(*(a_begin+i))];
        c = (c << 2) | ((c1 >> 4) & 0x3);
        ret.append(1, c);
        if (++i < len)
        {
            c = *(a_begin+i);
            if ('=' == c) // fillchar
                break;

            c = (char) s_tBase64Decode[static_cast<unsigned char>(*(a_begin+i))];
            c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
            ret.append(1, c1);
        }

        if (++i < len)
        {
            c1 = *(a_begin+i);
            if ('=' == c1) // fillchar
                break;

            c1 = (char) s_tBase64Decode[(unsigned char)*(a_begin+i)];
            c = ((c << 6) & 0xc0) | c1;
            ret.append(1, c);
        }
    }
}
