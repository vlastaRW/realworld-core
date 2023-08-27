
#pragma once


struct CGammaTables
{
	WORD m_aGamma[256];
	WORD m_aInvGamma[256+4];

	CGammaTables()
	{
		// default sRGB
		float const fMul = (1<<16)-1;
		for (ULONG i = 0; i < 256; ++i)
		{
			float const f = i/255.0f;
			m_aGamma[i] = (f <= 0.04045f ? f/12.92f : powf((f+0.055f)/1.055f, 2.4f))*fMul+0.5f;
		}
		for (ULONG i = 0; i < 256; ++i)
		{
			float const f = i/255.0f;
			m_aInvGamma[i] = (f <= 0.0031308f ? f*12.92f : powf(f, 1.0f/2.4f)*1.055f-0.055f)*fMul+0.5f;
		}
		// it can happen due to rounding errors
		for (ULONG i = 0; i < 4; ++i)
			m_aInvGamma[i+256] = 255*256;
	}
	CGammaTables(float a_fGamma)
	{
		float const fMul = (1<<16)-1;
		for (ULONG i = 0; i < 256; ++i)
		{
			m_aGamma[i] = powf(i/255.0f, a_fGamma)*fMul+0.5f;
		}
		for (ULONG i = 0; i < 256; ++i)
		{
			m_aInvGamma[i] = powf(i/255.0f, 1.0f/a_fGamma)*fMul+0.5f;
		}
		// it can happen due to rounding errors
		for (ULONG i = 0; i < 4; ++i)
			m_aInvGamma[i+256] = 255*256;
	}

	inline BYTE InvGamma(ULONG const a_n) const
	{
		WORD const* const p = m_aInvGamma+(a_n>>8);
		return (*p+(((a_n&0xff)*(p[1]-*p)+0x80)>>8))>>8;
	}

	static BYTE ToSRGB(float f)
	{
		if (f < 0.0f)
			return 0;
		if (f > 1.0f)
			return 255;
		return static_cast<BYTE>((f <= 0.0031308f ? f*12.92f : powf(f, 1.0f/2.4f)*1.055f-0.055f)*255.0f+0.5f);
	}
	static float FromSRGB(BYTE b)
	{
		float const f = b/255.0f;
		return f <= 0.04045f ? f/12.92f : powf((f+0.055f)/1.055f, 2.4f);
	}
	static DWORD BlendSRGB(DWORD c0, DWORD c1, float w0)
	{
		float w1 = 1.0f-w0;

		float r0 = FromSRGB(c0&0xff);
		float r1 = FromSRGB(c1&0xff);
		float g0 = FromSRGB((c0>>8)&0xff);
		float g1 = FromSRGB((c1>>8)&0xff);
		float b0 = FromSRGB((c0>>16)&0xff);
		float b1 = FromSRGB((c1>>16)&0xff);

		DWORD r = ToSRGB(r0*w0+r1*w1);
		DWORD g = ToSRGB(g0*w0+g1*w1);
		DWORD b = ToSRGB(b0*w0+b1*w1);
		return r|(g<<8)|(b<<16)|(c0&0xff000000);
	}
	static DWORD BlendSRGBA(DWORD c0, DWORD c1, float w0)
	{
		float w1 = 1.0f-w0;

		float a0 = (c0>>24)/255.0f;
		float a1 = (c1>>24)/255.0f;
		float af = a0*w0+a1*w1;
		DWORD a = af*255.0f+0.5f;
		if (a == 0)
			return 0;
		float ia = 1.0f/af;

		float r0 = a0*FromSRGB(c0&0xff);
		float r1 = a1*FromSRGB(c1&0xff);
		float g0 = a0*FromSRGB((c0>>8)&0xff);
		float g1 = a1*FromSRGB((c1>>8)&0xff);
		float b0 = a0*FromSRGB((c0>>16)&0xff);
		float b1 = a1*FromSRGB((c1>>16)&0xff);
		DWORD r = ToSRGB(ia*(r0*w0+r1*w1));
		DWORD g = ToSRGB(ia*(g0*w0+g1*w1));
		DWORD b = ToSRGB(ia*(b0*w0+b1*w1));
		return r|(g<<8)|(b<<16)|(a<<24);
	}
};

struct CFloatSRGB
{
	ULONG m_aToSRGB[104];
	float m_aFromSRGB[256];

	BYTE ToSRGB(float const f) const
	{
		ULONG const MAXV_BITS = 0x3f7fffff; // 1.0 - f32::EPSILON
		ULONG const MINV_BITS = 0x39000000; // 2^(-13)
		float const minv = *reinterpret_cast<float const*>(&MINV_BITS);
		float const maxv = *reinterpret_cast<float const*>(&MAXV_BITS);
		// written like this to handle nans.
		float input = f;
		if (!(input > minv))
			input = minv;

		if (input > maxv)
			input = maxv;

		ULONG fu = *reinterpret_cast<ULONG*>(&input);

		size_t const i = (fu - MINV_BITS) >> 20;

		ULONG const entry = m_aToSRGB[i];

		// bottom 16 bits are bias, top 9 are scale.
		ULONG const bias = (entry >> 16) << 9;
		ULONG const scale = entry & 0xffff;

		// lerp to the next highest mantissa bits.
		ULONG const t = (fu >> 12) & 0xff;
		ULONG const res = (bias + scale * t) >> 16;
		return static_cast<BYTE>(res);
	}
	BYTE ToSRGBUnchecked(float const f) const
	{
		ULONG const MAXV_BITS = 0x3f7fffff; // 1.0 - f32::EPSILON
		ULONG const MINV_BITS = 0x39000000; // 2^(-13)

		float input = f;
		ULONG fu = *reinterpret_cast<ULONG*>(&input);

		size_t const i = (fu - MINV_BITS) >> 20;

		ULONG const entry = m_aToSRGB[i];

		// bottom 16 bits are bias, top 9 are scale.
		ULONG const bias = (entry >> 16) << 9;
		ULONG const scale = entry & 0xffff;

		// lerp to the next highest mantissa bits.
		ULONG const t = (fu >> 12) & 0xff;
		ULONG const res = (bias + scale * t) >> 16;
		return static_cast<BYTE>(res);
	}

	CFloatSRGB()
	{
		static ULONG const TO_SRGB8_TABLE[104] =
		{
			0x0073000d, 0x007a000d, 0x0080000d, 0x0087000d, 0x008d000d, 0x0094000d, 0x009a000d, 0x00a1000d,
			0x00a7001a, 0x00b4001a, 0x00c1001a, 0x00ce001a, 0x00da001a, 0x00e7001a, 0x00f4001a, 0x0101001a,
			0x010e0033, 0x01280033, 0x01410033, 0x015b0033, 0x01750033, 0x018f0033, 0x01a80033, 0x01c20033,
			0x01dc0067, 0x020f0067, 0x02430067, 0x02760067, 0x02aa0067, 0x02dd0067, 0x03110067, 0x03440067,
			0x037800ce, 0x03df00ce, 0x044600ce, 0x04ad00ce, 0x051400ce, 0x057b00c5, 0x05dd00bc, 0x063b00b5,
			0x06970158, 0x07420142, 0x07e30130, 0x087b0120, 0x090b0112, 0x09940106, 0x0a1700fc, 0x0a9500f2,
			0x0b0f01cb, 0x0bf401ae, 0x0ccb0195, 0x0d950180, 0x0e56016e, 0x0f0d015e, 0x0fbc0150, 0x10630143,
			0x11070264, 0x1238023e, 0x1357021d, 0x14660201, 0x156601e9, 0x165a01d3, 0x174401c0, 0x182401af,
			0x18fe0331, 0x1a9602fe, 0x1c1502d2, 0x1d7e02ad, 0x1ed4028d, 0x201a0270, 0x21520256, 0x227d0240,
			0x239f0443, 0x25c003fe, 0x27bf03c4, 0x29a10392, 0x2b6a0367, 0x2d1d0341, 0x2ebe031f, 0x304d0300,
			0x31d105b0, 0x34a80555, 0x37520507, 0x39d504c5, 0x3c37048b, 0x3e7c0458, 0x40a8042a, 0x42bd0401,
			0x44c20798, 0x488e071e, 0x4c1c06b6, 0x4f76065d, 0x52a50610, 0x55ac05cc, 0x5892058f, 0x5b590559,
			0x5e0c0a23, 0x631c0980, 0x67db08f6, 0x6c55087f, 0x70940818, 0x74a007bd, 0x787d076c, 0x7c330723,
		};
		memcpy(m_aToSRGB, TO_SRGB8_TABLE, sizeof TO_SRGB8_TABLE);
		static float SRGB8_TO_FLOAT[256] =
		{
			0.0f, 0.000303527f, 0.000607054f, 0.00091058103f, 0.001214108f, 0.001517635f, 0.0018211621f, 0.002124689f,
			0.002428216f, 0.002731743f, 0.00303527f, 0.0033465356f, 0.003676507f, 0.004024717f, 0.004391442f,
			0.0047769533f, 0.005181517f, 0.0056053917f, 0.0060488326f, 0.006512091f, 0.00699541f, 0.0074990317f,
			0.008023192f, 0.008568125f, 0.009134057f, 0.009721218f, 0.010329823f, 0.010960094f, 0.011612245f,
			0.012286487f, 0.012983031f, 0.013702081f, 0.014443844f, 0.015208514f, 0.015996292f, 0.016807375f,
			0.017641952f, 0.018500218f, 0.019382361f, 0.020288562f, 0.02121901f, 0.022173883f, 0.023153365f,
			0.02415763f, 0.025186857f, 0.026241222f, 0.027320892f, 0.028426038f, 0.029556843f, 0.03071345f, 0.03189604f,
			0.033104774f, 0.03433981f, 0.035601325f, 0.036889452f, 0.038204376f, 0.039546248f, 0.04091521f, 0.042311423f,
			0.043735042f, 0.045186214f, 0.046665095f, 0.048171833f, 0.049706575f, 0.051269468f, 0.052860655f, 0.05448028f,
			0.056128494f, 0.057805434f, 0.05951124f, 0.06124607f, 0.06301003f, 0.06480328f, 0.06662595f, 0.06847818f,
			0.07036011f, 0.07227186f, 0.07421358f, 0.07618539f, 0.07818743f, 0.08021983f, 0.082282715f, 0.084376216f,
			0.086500466f, 0.088655606f, 0.09084173f, 0.09305898f, 0.095307484f, 0.09758736f, 0.09989874f, 0.10224175f,
			0.10461649f, 0.10702311f, 0.10946172f, 0.111932434f, 0.11443538f, 0.116970696f, 0.11953845f, 0.12213881f,
			0.12477186f, 0.12743773f, 0.13013652f, 0.13286836f, 0.13563336f, 0.13843165f, 0.14126332f, 0.1441285f,
			0.1470273f, 0.14995982f, 0.15292618f, 0.1559265f, 0.15896086f, 0.16202943f, 0.16513224f, 0.16826946f,
			0.17144115f, 0.17464745f, 0.17788847f, 0.1811643f, 0.18447503f, 0.1878208f, 0.19120172f, 0.19461787f,
			0.19806935f, 0.2015563f, 0.20507877f, 0.2086369f, 0.21223079f, 0.21586053f, 0.21952623f, 0.22322798f,
			0.22696589f, 0.23074007f, 0.23455065f, 0.23839766f, 0.2422812f, 0.2462014f, 0.25015837f, 0.25415218f,
			0.2581829f, 0.26225072f, 0.26635566f, 0.27049786f, 0.27467737f, 0.27889434f, 0.2831488f, 0.2874409f,
			0.2917707f, 0.29613832f, 0.30054384f, 0.30498737f, 0.30946895f, 0.31398875f, 0.31854683f, 0.32314324f,
			0.32777813f, 0.33245158f, 0.33716366f, 0.34191445f, 0.3467041f, 0.3515327f, 0.35640025f, 0.36130688f,
			0.3662527f, 0.37123778f, 0.37626222f, 0.3813261f, 0.38642952f, 0.39157256f, 0.3967553f, 0.40197787f,
			0.4072403f, 0.4125427f, 0.41788515f, 0.42326775f, 0.42869055f, 0.4341537f, 0.43965724f, 0.44520125f,
			0.45078585f, 0.45641106f, 0.46207705f, 0.46778384f, 0.47353154f, 0.47932023f, 0.48514998f, 0.4910209f,
			0.49693304f, 0.5028866f, 0.50888145f, 0.5149178f, 0.5209957f, 0.52711535f, 0.5332766f, 0.5394797f,
			0.5457247f, 0.5520116f, 0.5583406f, 0.5647117f, 0.57112503f, 0.57758063f, 0.5840786f, 0.590619f, 0.597202f,
			0.60382754f, 0.61049575f, 0.61720675f, 0.62396055f, 0.63075733f, 0.637597f, 0.6444799f, 0.6514058f,
			0.65837497f, 0.66538745f, 0.67244333f, 0.6795426f, 0.68668544f, 0.69387203f, 0.70110214f, 0.70837605f,
			0.7156938f, 0.72305536f, 0.730461f, 0.7379107f, 0.7454045f, 0.75294244f, 0.76052475f, 0.7681514f, 0.77582246f,
			0.78353804f, 0.79129815f, 0.79910296f, 0.8069525f, 0.8148468f, 0.822786f, 0.8307701f, 0.83879924f, 0.84687346f,
			0.8549928f, 0.8631574f, 0.87136734f, 0.8796226f, 0.8879232f, 0.89626956f, 0.90466136f, 0.913099f, 0.92158204f,
			0.93011117f, 0.9386859f, 0.9473069f, 0.9559735f, 0.9646866f, 0.9734455f, 0.98225087f, 0.9911022f, 1.0f
		};
		memcpy(m_aFromSRGB, SRGB8_TO_FLOAT, sizeof SRGB8_TO_FLOAT);
	}
};

MIDL_INTERFACE("28CB1317-2A27-4DB3-AFDC-E7C30F256E38")
IGammaTableCache : public IUnknown
{
public:
    virtual CGammaTables const* STDMETHODCALLTYPE GetSRGBTable() = 0;
	virtual CFloatSRGB const* STDMETHODCALLTYPE GetFloatSRGB() = 0;
};

class DECLSPEC_UUID("0DB564F7-62B2-48B5-88FD-FFDF224F1AC7")
GammaTableCache;
