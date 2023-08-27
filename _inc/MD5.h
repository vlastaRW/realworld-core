
#pragma once

namespace MD5
{
	// F, G and H are basic MD5 functions: selection, majority, parity
	inline ULONG F(ULONG x, ULONG y, ULONG z) { return (((x) & (y)) | ((~x) & (z))); }
	inline ULONG G(ULONG x, ULONG y, ULONG z) { return (((x) & (z)) | ((y) & (~z))); }
	inline ULONG H(ULONG x, ULONG y, ULONG z) { return ((x) ^ (y) ^ (z)); }
	inline ULONG I(ULONG x, ULONG y, ULONG z) { return ((y) ^ ((x) | (~z))); }

	// ROTATE_LEFT rotates x left n bits
	inline ULONG ROTATE_LEFT(ULONG x, ULONG n) { return (((x) << (n)) | ((x) >> (32-(n)))); }

	// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4
	// Rotation is separate from addition to prevent recomputation
	inline void FF(ULONG& a, ULONG b, ULONG c, ULONG d, ULONG x, ULONG s, ULONG ac)
	{
		a += F ((b), (c), (d)) + (x) + (ULONG)(ac);
		a = ROTATE_LEFT ((a), (s));
		a += (b);
	}
	inline void GG(ULONG& a, ULONG b, ULONG c, ULONG d, ULONG x, ULONG s, ULONG ac)
	{
		a += G ((b), (c), (d)) + (x) + (ULONG)(ac);
		a = ROTATE_LEFT ((a), (s));
		a += (b);
	}
	inline void HH(ULONG& a, ULONG b, ULONG c, ULONG d, ULONG x, ULONG s, ULONG ac)
	{
		a += H ((b), (c), (d)) + (x) + (ULONG)(ac);
		a = ROTATE_LEFT ((a), (s));
		a += (b);
	}
	inline void II(ULONG& a, ULONG b, ULONG c, ULONG d, ULONG x, ULONG s, ULONG ac)
	{
		a += I ((b), (c), (d)) + (x) + (ULONG)(ac);
		a = ROTATE_LEFT ((a), (s));
		a += (b);
	}

	inline void Transform(ULONG *buf, ULONG *in)
	{
		ULONG a = buf[0], b = buf[1], c = buf[2], d = buf[3];

		static ULONG const S11 = 7;
		static ULONG const S12 = 12;
		static ULONG const S13 = 17;
		static ULONG const S14 = 22;
		FF ( a, b, c, d, in[ 0], S11, 3614090360); /* 1 */
		FF ( d, a, b, c, in[ 1], S12, 3905402710); /* 2 */
		FF ( c, d, a, b, in[ 2], S13,  606105819); /* 3 */
		FF ( b, c, d, a, in[ 3], S14, 3250441966); /* 4 */
		FF ( a, b, c, d, in[ 4], S11, 4118548399); /* 5 */
		FF ( d, a, b, c, in[ 5], S12, 1200080426); /* 6 */
		FF ( c, d, a, b, in[ 6], S13, 2821735955); /* 7 */
		FF ( b, c, d, a, in[ 7], S14, 4249261313); /* 8 */
		FF ( a, b, c, d, in[ 8], S11, 1770035416); /* 9 */
		FF ( d, a, b, c, in[ 9], S12, 2336552879); /* 10 */
		FF ( c, d, a, b, in[10], S13, 4294925233); /* 11 */
		FF ( b, c, d, a, in[11], S14, 2304563134); /* 12 */
		FF ( a, b, c, d, in[12], S11, 1804603682); /* 13 */
		FF ( d, a, b, c, in[13], S12, 4254626195); /* 14 */
		FF ( c, d, a, b, in[14], S13, 2792965006); /* 15 */
		FF ( b, c, d, a, in[15], S14, 1236535329); /* 16 */

		static ULONG const S21 = 5;
		static ULONG const S22 = 9;
		static ULONG const S23 = 14;
		static ULONG const S24 = 20;
		GG ( a, b, c, d, in[ 1], S21, 4129170786); /* 17 */
		GG ( d, a, b, c, in[ 6], S22, 3225465664); /* 18 */
		GG ( c, d, a, b, in[11], S23,  643717713); /* 19 */
		GG ( b, c, d, a, in[ 0], S24, 3921069994); /* 20 */
		GG ( a, b, c, d, in[ 5], S21, 3593408605); /* 21 */
		GG ( d, a, b, c, in[10], S22,   38016083); /* 22 */
		GG ( c, d, a, b, in[15], S23, 3634488961); /* 23 */
		GG ( b, c, d, a, in[ 4], S24, 3889429448); /* 24 */
		GG ( a, b, c, d, in[ 9], S21,  568446438); /* 25 */
		GG ( d, a, b, c, in[14], S22, 3275163606); /* 26 */
		GG ( c, d, a, b, in[ 3], S23, 4107603335); /* 27 */
		GG ( b, c, d, a, in[ 8], S24, 1163531501); /* 28 */
		GG ( a, b, c, d, in[13], S21, 2850285829); /* 29 */
		GG ( d, a, b, c, in[ 2], S22, 4243563512); /* 30 */
		GG ( c, d, a, b, in[ 7], S23, 1735328473); /* 31 */
		GG ( b, c, d, a, in[12], S24, 2368359562); /* 32 */

		static ULONG const S31 = 4;
		static ULONG const S32 = 11;
		static ULONG const S33 = 16;
		static ULONG const S34 = 23;
		HH ( a, b, c, d, in[ 5], S31, 4294588738); /* 33 */
		HH ( d, a, b, c, in[ 8], S32, 2272392833); /* 34 */
		HH ( c, d, a, b, in[11], S33, 1839030562); /* 35 */
		HH ( b, c, d, a, in[14], S34, 4259657740); /* 36 */
		HH ( a, b, c, d, in[ 1], S31, 2763975236); /* 37 */
		HH ( d, a, b, c, in[ 4], S32, 1272893353); /* 38 */
		HH ( c, d, a, b, in[ 7], S33, 4139469664); /* 39 */
		HH ( b, c, d, a, in[10], S34, 3200236656); /* 40 */
		HH ( a, b, c, d, in[13], S31,  681279174); /* 41 */
		HH ( d, a, b, c, in[ 0], S32, 3936430074); /* 42 */
		HH ( c, d, a, b, in[ 3], S33, 3572445317); /* 43 */
		HH ( b, c, d, a, in[ 6], S34,   76029189); /* 44 */
		HH ( a, b, c, d, in[ 9], S31, 3654602809); /* 45 */
		HH ( d, a, b, c, in[12], S32, 3873151461); /* 46 */
		HH ( c, d, a, b, in[15], S33,  530742520); /* 47 */
		HH ( b, c, d, a, in[ 2], S34, 3299628645); /* 48 */

		static ULONG const S41 = 6;
		static ULONG const S42 = 10;
		static ULONG const S43 = 15;
		static ULONG const S44 = 21;
		II ( a, b, c, d, in[ 0], S41, 4096336452); /* 49 */
		II ( d, a, b, c, in[ 7], S42, 1126891415); /* 50 */
		II ( c, d, a, b, in[14], S43, 2878612391); /* 51 */
		II ( b, c, d, a, in[ 5], S44, 4237533241); /* 52 */
		II ( a, b, c, d, in[12], S41, 1700485571); /* 53 */
		II ( d, a, b, c, in[ 3], S42, 2399980690); /* 54 */
		II ( c, d, a, b, in[10], S43, 4293915773); /* 55 */
		II ( b, c, d, a, in[ 1], S44, 2240044497); /* 56 */
		II ( a, b, c, d, in[ 8], S41, 1873313359); /* 57 */
		II ( d, a, b, c, in[15], S42, 4264355552); /* 58 */
		II ( c, d, a, b, in[ 6], S43, 2734768916); /* 59 */
		II ( b, c, d, a, in[13], S44, 1309151649); /* 60 */
		II ( a, b, c, d, in[ 4], S41, 4149444226); /* 61 */
		II ( d, a, b, c, in[11], S42, 3174756917); /* 62 */
		II ( c, d, a, b, in[ 2], S43,  718787259); /* 63 */
		II ( b, c, d, a, in[ 9], S44, 3951481745); /* 64 */

		buf[0] += a;
		buf[1] += b;
		buf[2] += c;
		buf[3] += d;
	}

	struct SMD5 { BYTE val[16]; };
	inline void Compute(void const* a_pData, ULONG a_nLength, SMD5* a_pResult)
	{
		ULONG bits[2];                /* number of _bits_ handled mod 2^64 */
		ULONG buf[4];                                    /* scratch buffer */
		unsigned char in[64];                              /* input buffer */

		bits[0] = bits[1] = (ULONG)0;
		buf[0] = (ULONG)0x67452301;
		buf[1] = (ULONG)0xefcdab89;
		buf[2] = (ULONG)0x98badcfe;
		buf[3] = (ULONG)0x10325476;

		BYTE const* inBuf = reinterpret_cast<BYTE const*>(a_pData);
		ULONG loc[16];
		unsigned int i, ii;

		/* compute number of bytes mod 64 */
		int mdi = (int)((bits[0] >> 3) & 0x3F);

		/* update number of bits */
		if ((bits[0] + (a_nLength << 3)) < bits[0])
			bits[1]++;
		bits[0] += (a_nLength << 3);
		bits[1] += (a_nLength >> 29);

		while (a_nLength--)
		{
			in[mdi++] = *inBuf++;

			if (mdi == 0x40)
			{
				for (i = 0, ii = 0; i < 16; i++, ii += 4)
					loc[i] = (((ULONG)in[ii+3]) << 24) | (((ULONG)in[ii+2]) << 16) | (((ULONG)in[ii+1]) << 8) | ((ULONG)in[ii]);
				Transform(buf, loc);
				mdi = 0;
			}
		}

		ULONG fin[16];

		/* save number of bits */
		fin[14] = bits[0];
		fin[15] = bits[1];

		/* compute number of bytes mod 64 */
		mdi = (int)((bits[0] >> 3) & 0x3F);

		// pad out to 56 mod 64
		static BYTE const PADDING[64] = {
			0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		a_nLength = (mdi < 56) ? (56 - mdi) : (120 - mdi);
		inBuf = PADDING;
		mdi = (int)((bits[0] >> 3) & 0x3F);
		if ((bits[0] + (a_nLength << 3)) < bits[0])
			bits[1]++;
		bits[0] += (a_nLength << 3);
		bits[1] += (a_nLength >> 29);

		while (a_nLength--)
		{
			in[mdi++] = *inBuf++;

			if (mdi == 0x40)
			{
				for (i = 0, ii = 0; i < 16; i++, ii += 4)
					loc[i] = (((ULONG)in[ii+3]) << 24) | (((ULONG)in[ii+2]) << 16) | (((ULONG)in[ii+1]) << 8) | ((ULONG)in[ii]);
				Transform(buf, loc);
				mdi = 0;
			}
		}

		// append length in bits and transform
		for (i = 0, ii = 0; i < 14; i++, ii += 4)
			fin[i] = (((ULONG)in[ii+3]) << 24) | (((ULONG)in[ii+2]) << 16) | (((ULONG)in[ii+1]) << 8) | ((ULONG)in[ii]);
		Transform(buf, fin);

		for (i = 0, ii = 0; i < 4; i++, ii += 4)
		{
			a_pResult->val[ii] = (unsigned char)(buf[i] & 0xFF);
			a_pResult->val[ii+1] = (unsigned char)((buf[i] >> 8) & 0xFF);
			a_pResult->val[ii+2] = (unsigned char)((buf[i] >> 16) & 0xFF);
			a_pResult->val[ii+3] = (unsigned char)((buf[i] >> 24) & 0xFF);
		}

	}
}