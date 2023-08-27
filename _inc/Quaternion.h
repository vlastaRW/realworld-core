
#pragma once

#ifndef __RWConceptSceneGraph_h__

#include <math.h>

struct TQuaternionf
{
	float fX;
	float fY;
	float fZ;
	float fW;
};

struct TMatrix4x4f
{
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
};

// Matrix rotation (using quaternions)
const TQuaternionf TQUATERNIONF_IDENTITY = {0.0f, 0.0f, 0.0f, 1.0f};

class CQuaternionf
{
public:
	CQuaternionf() : m_t(TQUATERNIONF_IDENTITY) {}
	CQuaternionf(TQuaternionf const& a_t) : m_t(a_t) {}
	CQuaternionf(float const a_fAxisX, float const a_fAxisY, float const a_fAxisZ, float const a_fAngle)
	{
		float const f = sinf(0.5f*a_fAngle)/sqrtf(a_fAxisX*a_fAxisX + a_fAxisY*a_fAxisY + a_fAxisZ*a_fAxisZ);
		m_t.fW = cosf(0.5f*a_fAngle);
		m_t.fX = a_fAxisX*f;
		m_t.fY = a_fAxisY*f;
		m_t.fZ = a_fAxisZ*f;
	}
	CQuaternionf(float const* a_fAxis, float const a_fAngle)
	{
		float f = sinf(0.5f*a_fAngle)/sqrtf(a_fAxis[0]*a_fAxis[0] + a_fAxis[1]*a_fAxis[1] + a_fAxis[2]*a_fAxis[2]);
		m_t.fW = cosf(0.5f*a_fAngle);
		m_t.fX = a_fAxis[0]*f;
		m_t.fY = a_fAxis[1]*f;
		m_t.fZ = a_fAxis[2]*f;
	}
	CQuaternionf(CQuaternionf const& a_c) : m_t(a_c.m_t) {}
	explicit CQuaternionf(TMatrix4x4f a_tRotation)
	{
		// remove scale factor from matrix
		float fTmp = 1.0f/sqrtf(a_tRotation._11*a_tRotation._11 + a_tRotation._12*a_tRotation._12 + a_tRotation._13*a_tRotation._13);
		a_tRotation._11 *= fTmp; a_tRotation._12 *= fTmp; a_tRotation._13 *= fTmp;
		fTmp = 1.0f/sqrtf(a_tRotation._21*a_tRotation._21 + a_tRotation._22*a_tRotation._22 + a_tRotation._23*a_tRotation._23);
		a_tRotation._21 *= fTmp; a_tRotation._22 *= fTmp; a_tRotation._23 *= fTmp;
		fTmp = 1.0f/sqrtf(a_tRotation._31*a_tRotation._31 + a_tRotation._32*a_tRotation._32 + a_tRotation._33*a_tRotation._33);
		a_tRotation._31 *= fTmp; a_tRotation._32 *= fTmp; a_tRotation._33 *= fTmp;

		float fT = a_tRotation._11 + a_tRotation._22 + a_tRotation._33 + 1.0f;
		if (fT > 1e-4f)
		{
			float const fSqrT = sqrtf(fT);
			float const fS = 0.5f / fSqrT;
			m_t.fX = (a_tRotation._32 - a_tRotation._23) * fS;
			m_t.fY = (a_tRotation._13 - a_tRotation._31) * fS;
			m_t.fZ = (a_tRotation._21 - a_tRotation._12) * fS;
			m_t.fW = 0.5f * fSqrT;
		}
		else
		{
			if (a_tRotation._11 > a_tRotation._22 && a_tRotation._11 > a_tRotation._33)
			{
				float const fSqrT = sqrtf(1.0f + a_tRotation._11 - a_tRotation._22 - a_tRotation._33);
				float const fS = 0.5f / fSqrT;
				m_t.fX = 0.5f * fSqrT;
				m_t.fY = (a_tRotation._12 + a_tRotation._21) * fS;
				m_t.fZ = (a_tRotation._13 + a_tRotation._31) * fS;
				m_t.fW = (a_tRotation._23 - a_tRotation._32) * fS;
			}
			else if (a_tRotation._22 > a_tRotation._33)
			{
				float const fSqrT = sqrtf(1.0f + a_tRotation._22 - a_tRotation._11 - a_tRotation._33);
				float const fS = 0.5f / fSqrT;
				m_t.fX = (a_tRotation._12 + a_tRotation._21) * fS;
				m_t.fY = 0.5f * fSqrT;
				m_t.fZ = (a_tRotation._23 + a_tRotation._32) * fS;
				m_t.fW = (a_tRotation._13 - a_tRotation._31) * fS;
			}
			else
			{
				float const fSqrT = sqrtf(1.0f + a_tRotation._33 - a_tRotation._11 - a_tRotation._22);
				float const fS = 0.5f / fSqrT;
				m_t.fX = (a_tRotation._13 + a_tRotation._31) * fS;
				m_t.fY = (a_tRotation._23 + a_tRotation._32) * fS;
				m_t.fZ = 0.5f * fSqrT;
				m_t.fW = (a_tRotation._12 - a_tRotation._21) * fS;
			}
		}
	}
	explicit CQuaternionf(TMatrix3x3f a_tRotation)
	{
		// remove scale factor from matrix
		float fTmp = 1.0f/sqrtf(a_tRotation._11*a_tRotation._11 + a_tRotation._12*a_tRotation._12);
		a_tRotation._11 *= fTmp; a_tRotation._12 *= fTmp;
		fTmp = 1.0f/sqrtf(a_tRotation._21*a_tRotation._21 + a_tRotation._22*a_tRotation._22);
		a_tRotation._21 *= fTmp; a_tRotation._22 *= fTmp;

		float fT = a_tRotation._11 + a_tRotation._22 + 2.0f;
		if (fT > 1e-4f)
		{
			float const fSqrT = sqrtf(fT);
			float const fS = 0.5f / fSqrT;
			m_t.fX = 0.0f;
			m_t.fY = 0.0f;
			m_t.fZ = (a_tRotation._21 - a_tRotation._12) * fS;
			m_t.fW = 0.5f * fSqrT;
		}
		else
		{
			float const fSqrT = sqrtf(2.0f - a_tRotation._11 - a_tRotation._22);
			float const fS = 0.5f / fSqrT;
			m_t.fX = 0.0f;
			m_t.fY = 0.0f;
			m_t.fZ = 0.5f * fSqrT;
			m_t.fW = (a_tRotation._12 - a_tRotation._21) * fS;
		}
	}

	template<int t_nRow, int t_nCol>
	friend float MatrixItemf(CQuaternionf const& a_tQuat);

	CQuaternionf operator +(CQuaternionf const& a_rhs) const
	{
		TQuaternionf const t1 = {m_t.fX*a_rhs.m_t.fW, m_t.fY*a_rhs.m_t.fW, m_t.fZ*a_rhs.m_t.fW};
		TQuaternionf const t2 = {a_rhs.m_t.fX*m_t.fW, a_rhs.m_t.fY*m_t.fW, a_rhs.m_t.fZ*m_t.fW};
		TQuaternionf const t3 = {
			a_rhs.m_t.fY*m_t.fZ-a_rhs.m_t.fZ*m_t.fY + t1.fX + t2.fX,
			a_rhs.m_t.fZ*m_t.fX-a_rhs.m_t.fX*m_t.fZ + t1.fY + t2.fY,
			a_rhs.m_t.fX*m_t.fY-a_rhs.m_t.fY*m_t.fX + t1.fZ + t2.fZ,
			a_rhs.m_t.fW * m_t.fW -
			a_rhs.m_t.fX * m_t.fX -
			a_rhs.m_t.fY * m_t.fY -
			a_rhs.m_t.fZ * m_t.fZ};

		return t3;
	}

	CQuaternionf const& operator +=(CQuaternionf const& a_rhs)
	{
		TQuaternionf const t1 = {m_t.fX*a_rhs.m_t.fW, m_t.fY*a_rhs.m_t.fW, m_t.fZ*a_rhs.m_t.fW};
		TQuaternionf const t2 = {a_rhs.m_t.fX*m_t.fW, a_rhs.m_t.fY*m_t.fW, a_rhs.m_t.fZ*m_t.fW};
		TQuaternionf const t3 = {
			a_rhs.m_t.fY*m_t.fZ-a_rhs.m_t.fZ*m_t.fY + t1.fX + t2.fX,
			a_rhs.m_t.fZ*m_t.fX-a_rhs.m_t.fX*m_t.fZ + t1.fY + t2.fY,
			a_rhs.m_t.fX*m_t.fY-a_rhs.m_t.fY*m_t.fX + t1.fZ + t2.fZ,
			a_rhs.m_t.fW * m_t.fW -
			a_rhs.m_t.fX * m_t.fX -
			a_rhs.m_t.fY * m_t.fY -
			a_rhs.m_t.fZ * m_t.fZ};
		m_t = t3;
		return *this;
	}
	CQuaternionf operator ~() const
	{
		TQuaternionf t = {m_t.fX, m_t.fY, m_t.fZ, -m_t.fW};
		return t;
	}

	operator TQuaternionf const&() const
	{
		return m_t;
	}
	operator TQuaternionf const*() const
	{
		return &m_t;
	}

	TQuaternionf* operator&()
	{
		return &m_t;
	}

	void Normalize()
	{
		float const f = 1.0f/sqrtf(m_t.fX*m_t.fX + m_t.fY*m_t.fY + m_t.fZ*m_t.fZ + m_t.fW*m_t.fW);
		m_t.fX *= f;
		m_t.fY *= f;
		m_t.fZ *= f;
		m_t.fW *= f;
	}

private:
	TQuaternionf m_t;
};

template<int t_nRow, int t_nCol>
inline float MatrixItemf(CQuaternionf const& a_tQuat)
{
	// TODO: static assert (t_nRow >= 0 && t_nRow < 4 && t_nCol >= 0 && t_nCol < 4)
	switch (t_nRow)
	{
	case 0: switch (t_nCol) {
		case 0: return 1.0f - 2.0f * (a_tQuat.m_t.fY*a_tQuat.m_t.fY + a_tQuat.m_t.fZ*a_tQuat.m_t.fZ);
		case 1: return 2.0f * (a_tQuat.m_t.fX*a_tQuat.m_t.fY - a_tQuat.m_t.fZ*a_tQuat.m_t.fW);
		case 2: return 2.0f * (a_tQuat.m_t.fZ*a_tQuat.m_t.fX + a_tQuat.m_t.fY*a_tQuat.m_t.fW);
		case 3: return 0.0f; }
	case 1: switch (t_nCol) {
		case 0: return 2.0f * (a_tQuat.m_t.fX*a_tQuat.m_t.fY + a_tQuat.m_t.fZ*a_tQuat.m_t.fW);
		case 1: return 1.0f - 2.0f * (a_tQuat.m_t.fZ*a_tQuat.m_t.fZ + a_tQuat.m_t.fX*a_tQuat.m_t.fX);
		case 2: return 2.0f * (a_tQuat.m_t.fY*a_tQuat.m_t.fZ - a_tQuat.m_t.fX*a_tQuat.m_t.fW);
		case 3: return 0.0f; }
	case 2: switch (t_nCol) {
		case 0: return 2.0f * (a_tQuat.m_t.fZ*a_tQuat.m_t.fX - a_tQuat.m_t.fY*a_tQuat.m_t.fW);
		case 1: return 2.0f * (a_tQuat.m_t.fY*a_tQuat.m_t.fZ + a_tQuat.m_t.fX*a_tQuat.m_t.fW);
		case 2: return 1.0f - 2.0f * (a_tQuat.m_t.fY*a_tQuat.m_t.fY + a_tQuat.m_t.fX*a_tQuat.m_t.fX);
		case 3: return 0.0f; }
	case 3: switch (t_nCol) {
		case 0: return 0.0f;
		case 1: return 0.0f;
		case 2: return 0.0f;
		case 3: return 1.0f; }
	}
	return 0.0f; // just to prevent warning C4715: not all paths return value..
}

inline CQuaternionf TrackballRotation(float const a_fX1, float const a_fY1, float const a_fX2, float const a_fY2)
{
    if (a_fX1 == a_fX2 && a_fY1 == a_fY2)
	{
		return TQUATERNIONF_IDENTITY;
    }

    float const fD1 = a_fX1*a_fX1 + a_fY1*a_fY1;
	float const fZ1 = fD1 < 0.5f ? sqrtf(1-fD1) : (0.5f/sqrtf(fD1));

	float const fD2 = a_fX2*a_fX2 + a_fY2*a_fY2;
	float const fZ2 = fD2 < 0.5f ? sqrtf(1-fD2) : (0.5f/sqrtf(fD2));

	float const tAxis[3] = {a_fY2*fZ1-fZ2*a_fY1, fZ2*a_fX1-a_fX2*fZ1, a_fX2*a_fY1-a_fY2*a_fX1};

	float fT = 0.5f*sqrtf((a_fX1-a_fX2)*(a_fX1-a_fX2) + (a_fY1-a_fY2)*(a_fY1-a_fY2) + (fZ1-fZ2)*(fZ1-fZ2));
    if (fT > 1.0f) fT = 1.0f;
    if (fT < -1.0f) fT = -1.0f;

	return CQuaternionf(tAxis, 2.0f * asinf(fT));
}

template<class TMatrix>
inline void MatrixAssign(TMatrix4x4f* a_pOut, TMatrix const& a_tRightOp)
{
	a_pOut->_11 = MatrixItemf<0, 0>(a_tRightOp);
	a_pOut->_12 = MatrixItemf<0, 1>(a_tRightOp);
	a_pOut->_13 = MatrixItemf<0, 2>(a_tRightOp);
	a_pOut->_14 = MatrixItemf<0, 3>(a_tRightOp);
	a_pOut->_21 = MatrixItemf<1, 0>(a_tRightOp);
	a_pOut->_22 = MatrixItemf<1, 1>(a_tRightOp);
	a_pOut->_23 = MatrixItemf<1, 2>(a_tRightOp);
	a_pOut->_24 = MatrixItemf<1, 3>(a_tRightOp);
	a_pOut->_31 = MatrixItemf<2, 0>(a_tRightOp);
	a_pOut->_32 = MatrixItemf<2, 1>(a_tRightOp);
	a_pOut->_33 = MatrixItemf<2, 2>(a_tRightOp);
	a_pOut->_34 = MatrixItemf<2, 3>(a_tRightOp);
	a_pOut->_41 = MatrixItemf<3, 0>(a_tRightOp);
	a_pOut->_42 = MatrixItemf<3, 1>(a_tRightOp);
	a_pOut->_43 = MatrixItemf<3, 2>(a_tRightOp);
	a_pOut->_44 = MatrixItemf<3, 3>(a_tRightOp);
}

struct TVector3f { union { struct { float x, y, z; }; float v[3]; }; };

// item accessor for TMatrix4x4f - compatibility with "Matrix" is defined by this accessor, not by TMatrix4x4f
template<int t_nRow, int t_nCol>
float const& MatrixItemf(TMatrix4x4f const& a_tMatrix)
{
	return reinterpret_cast<float const*>(&a_tMatrix)[(t_nRow << 2) + t_nCol];
}

template<class TMatrix>
TVector3f TransformVector3(TMatrix const& a_tMatrix, TVector3f const& a_tVector)
{
	float const fW = 1.0f / (MatrixItemf<0, 3>(a_tMatrix) * a_tVector.x + MatrixItemf<1, 3>(a_tMatrix) * a_tVector.y + MatrixItemf<2, 3>(a_tMatrix) * a_tVector.z + MatrixItemf<3, 3>(a_tMatrix));
	TVector3f t =
	{
		fW * (MatrixItemf<0, 0>(a_tMatrix) * a_tVector.x + MatrixItemf<1, 0>(a_tMatrix) * a_tVector.y + MatrixItemf<2, 0>(a_tMatrix) * a_tVector.z + MatrixItemf<3, 0>(a_tMatrix)),
		fW * (MatrixItemf<0, 1>(a_tMatrix) * a_tVector.x + MatrixItemf<1, 1>(a_tMatrix) * a_tVector.y + MatrixItemf<2, 1>(a_tMatrix) * a_tVector.z + MatrixItemf<3, 1>(a_tMatrix)),
		fW * (MatrixItemf<0, 2>(a_tMatrix) * a_tVector.x + MatrixItemf<1, 2>(a_tMatrix) * a_tVector.y + MatrixItemf<2, 2>(a_tMatrix) * a_tVector.z + MatrixItemf<3, 2>(a_tMatrix))
	};
	return t;
}

#endif
