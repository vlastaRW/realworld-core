
#pragma once

#include <StringOutput.h>


template<typename TBinaryOutput>
class CStringOutputUTF8 : public CStringOutput
{
public:
	CStringOutputUTF8(TBinaryOutput& a_cBinaryOutput) :
		m_cBinaryOutput(a_cBinaryOutput)
	{
	}

	void Write(wchar_t const* a_pData, size_t a_nSize)
	{
		for (size_t i = 0; i != a_nSize; i++)
		{
			if (a_pData[i] & 0xff80)
			{
				if (a_pData[i] & 0xf800)
				{
					// 0x0800-0xffff - 3 bytes
					BYTE const b[3] =
					{
						static_cast<BYTE const>(0xe0 | ((a_pData[i]>>12)&0x0f)),
						static_cast<BYTE const>(0x80 | ((a_pData[i]>>6)&0x3f)),
						static_cast<BYTE const>(0x80 | (a_pData[i]&0x3f))
					};
					m_cBinaryOutput.Write(b, 3);
				}
				else
				{
					// 0x0080-0x07ff - 2 bytes
					BYTE const b[2] =
					{
						static_cast<BYTE const>(0xc0 | ((a_pData[i]>>6)&0x1f)),
						static_cast<BYTE const>(0x80 | (a_pData[i]&0x3f))
					};
					m_cBinaryOutput.Write(b, 2);
				}
			}
			else
			{
				// 0x0000-0x007f - 1 byte
				BYTE const b = static_cast<BYTE const>(a_pData[i]);
				m_cBinaryOutput.Write(&b, 1);
			}
		}
	}

private:
	TBinaryOutput& m_cBinaryOutput;
};

