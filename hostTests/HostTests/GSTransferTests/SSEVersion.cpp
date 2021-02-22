// The implementation used in Play.

typedef unsigned int uint32;
typedef unsigned char uint8;

enum PAGESIZE
{
	PAGESIZE = 8192,
};

enum BLOCKSIZE
{
	BLOCKSIZE = 256,
};

enum COLUMNSIZE
{
	COLUMNSIZE = 64,
};

struct STORAGEPSMT8x
{
	enum PAGEWIDTH
	{
		PAGEWIDTH = 128
	};
	enum PAGEHEIGHT
	{
		PAGEHEIGHT = 64
	};
	enum BLOCKWIDTH
	{
		BLOCKWIDTH = 16
	};
	enum BLOCKHEIGHT
	{
		BLOCKHEIGHT = 16
	};
	enum COLUMNWIDTH
	{
		COLUMNWIDTH = 16
	};
	enum COLUMNHEIGHT
	{
		COLUMNHEIGHT = 4
	};

	static const int m_nBlockSwizzleTable[4][8];
	static const int m_nColumnWordTable[2][2][8];

	typedef unsigned char Unit;
};

struct STORAGEPSMT4x
{
	enum PAGEWIDTH
	{
		PAGEWIDTH = 128
	};
	enum PAGEHEIGHT
	{
		PAGEHEIGHT = 128
	};
	enum BLOCKWIDTH
	{
		BLOCKWIDTH = 32
	};
	enum BLOCKHEIGHT
	{
		BLOCKHEIGHT = 16
	};
	enum COLUMNWIDTH
	{
		COLUMNWIDTH = 32
	};
	enum COLUMNHEIGHT
	{
		COLUMNHEIGHT = 4
	};

	static const int m_nBlockSwizzleTable[8][4];
	static const int m_nColumnWordTable[2][2][8];

	typedef uint8 Unit;
};

enum RAMSIZE
{
	RAMSIZE = 0x00400000,
};

template <typename Storage>
class CPixelIndexor
{
public:
	CPixelIndexor(uint8* pMemory, uint32 nPointer, uint32 nWidth)
	{
		m_nPointer = nPointer;
		m_nWidth = nWidth;
		m_pMemory = pMemory;
		BuildPageOffsetTable();
	}

	typename Storage::Unit GetPixel(unsigned int nX, unsigned int nY)
	{
		return *GetPixelAddress(nX, nY);
	}

	void SetPixel(unsigned int nX, unsigned int nY, typename Storage::Unit nPixel)
	{
		*GetPixelAddress(nX, nY) = nPixel;
	}

	typename Storage::Unit* GetPixelAddress(unsigned int nX, unsigned int nY)
	{
		uint32 pageNum = (nX / Storage::PAGEWIDTH) + (nY / Storage::PAGEHEIGHT) * (m_nWidth * 64) / Storage::PAGEWIDTH;

		nX %= Storage::PAGEWIDTH;
		nY %= Storage::PAGEHEIGHT;

		uint32 pageOffset = m_pageOffsets[nY][nX];
		auto pixelAddr = m_pMemory + ((m_nPointer + (pageNum * PAGESIZE) + pageOffset) & (RAMSIZE - 1));
		return reinterpret_cast<typename Storage::Unit*>(pixelAddr);
	}

	static uint32* GetPageOffsets()
	{
		BuildPageOffsetTable();
		return reinterpret_cast<uint32*>(m_pageOffsets);
	}

private:
	static void BuildPageOffsetTable()
	{
		if (m_pageOffsetsInitialized) return;

		// not used in this case as specialisation wins below
	}

	uint32 GetColumnAddress(unsigned int& nX, unsigned int& nY)
	{
		uint32 nPageNum = (nX / Storage::PAGEWIDTH) + (nY / Storage::PAGEHEIGHT) * (m_nWidth * 64) / Storage::PAGEWIDTH;

		nX %= Storage::PAGEWIDTH;
		nY %= Storage::PAGEHEIGHT;

		uint32 nBlockNum = Storage::m_nBlockSwizzleTable[nY / Storage::BLOCKHEIGHT][nX / Storage::BLOCKWIDTH];

		nX %= Storage::BLOCKWIDTH;
		nY %= Storage::BLOCKHEIGHT;

		uint32 nColumnNum = (nY / Storage::COLUMNHEIGHT);

		nY %= Storage::COLUMNHEIGHT;

		return (m_nPointer + (nPageNum * PAGESIZE) + (nBlockNum * BLOCKSIZE) + (nColumnNum * COLUMNSIZE)) & (RAMSIZE - 1);
	}

	uint32 m_nPointer;
	uint32 m_nWidth;
	uint8* m_pMemory;

	static bool m_pageOffsetsInitialized;
	static uint32 m_pageOffsets[Storage::PAGEHEIGHT][Storage::PAGEWIDTH];
};

template <>
inline uint8 CPixelIndexor<STORAGEPSMT4x>::GetPixel(unsigned int nX, unsigned int nY)
{
	typedef STORAGEPSMT4x Storage;

	uint32 nAddress;
	unsigned int nColumnNum, nSubTable, nShiftAmount;

	nColumnNum = (nY / Storage::COLUMNHEIGHT) & 0x01;
	nAddress = GetColumnAddress(nX, nY);

	nShiftAmount = (nX & 0x18);
	nShiftAmount += (nY & 0x02) << 1;
	nSubTable = (nY & 0x02) >> 1;
	nSubTable ^= (nColumnNum);

	nX &= 0x07;
	nY &= 0x01;

	return (uint8)(((uint32*)&m_pMemory[nAddress])[Storage::m_nColumnWordTable[nSubTable][nY][nX]] >> nShiftAmount) & 0x0F;
}

template <>
inline void CPixelIndexor<STORAGEPSMT4x>::SetPixel(unsigned int nX, unsigned int nY, uint8 nPixel)
{
	typedef STORAGEPSMT4x Storage;

	uint32 nAddress;
	unsigned int nColumnNum, nSubTable, nShiftAmount;

	nColumnNum = (nY / Storage::COLUMNHEIGHT) & 0x01;
	nAddress = GetColumnAddress(nX, nY);

	nShiftAmount = (nX & 0x18);
	nShiftAmount += (nY & 0x02) << 1;
	nSubTable = (nY & 0x02) >> 1;
	nSubTable ^= (nColumnNum);

	nX &= 0x07;
	nY &= 0x01;

	uint32* pPixel = &(((uint32*)&m_pMemory[nAddress])[Storage::m_nColumnWordTable[nSubTable][nY][nX]]);

	(*pPixel) &= ~(0xF << nShiftAmount);
	(*pPixel) |= (nPixel << nShiftAmount);
}

template <>
inline void CPixelIndexor<STORAGEPSMT4x>::BuildPageOffsetTable()
{
	if (m_pageOffsetsInitialized) return;

	typedef STORAGEPSMT4x Storage;

	for (uint32 y = 0; y < Storage::PAGEHEIGHT; y++)
	{
		for (uint32 x = 0; x < Storage::PAGEWIDTH; x++)
		{
			uint32 workX = x;
			uint32 workY = y;

			uint32 blockNum = Storage::m_nBlockSwizzleTable[workY / Storage::BLOCKHEIGHT][workX / Storage::BLOCKWIDTH];

			workX %= Storage::BLOCKWIDTH;
			workY %= Storage::BLOCKHEIGHT;

			uint32 columnNum = (workY / Storage::COLUMNHEIGHT);

			workY %= Storage::COLUMNHEIGHT;

			uint32 shiftAmount = (workX & 0x18);
			shiftAmount += (workY & 0x02) << 1;
			uint32 nibble = shiftAmount / 4;

			uint32 subTable = (workY & 0x02) >> 1;
			subTable ^= (columnNum & 0x01);

			workX &= 0x07;
			workY &= 0x01;

			uint32 offset = ((columnNum * COLUMNSIZE) + (blockNum * BLOCKSIZE) + (Storage::m_nColumnWordTable[subTable][workY][workX] * 4)) * 2 + nibble;
			m_pageOffsets[y][x] = offset;
		}
	}
	m_pageOffsetsInitialized = true;
}

template <>
inline void CPixelIndexor<STORAGEPSMT8x>::BuildPageOffsetTable()
{
	if (m_pageOffsetsInitialized) return;

	typedef STORAGEPSMT8x Storage;

	for (uint32 y = 0; y < Storage::PAGEHEIGHT; y++)
	{
		for (uint32 x = 0; x < Storage::PAGEWIDTH; x++)
		{
			uint32 workX = x;
			uint32 workY = y;

			uint32 blockNum = Storage::m_nBlockSwizzleTable[workY / Storage::BLOCKHEIGHT][workX / Storage::BLOCKWIDTH];

			workX %= Storage::BLOCKWIDTH;
			workY %= Storage::BLOCKHEIGHT;

			uint32 columnNum = (workY / Storage::COLUMNHEIGHT);

			workY %= Storage::COLUMNHEIGHT;

			uint32 table = (workY & 0x02) >> 1;
			uint32 byte = (workX & 0x08) >> 2;
			byte += (workY & 0x02) >> 1;
			table ^= ((y / Storage::COLUMNHEIGHT) & 1);

			workX &= 0x7;
			workY &= 0x1;

			uint32 offset = (blockNum * BLOCKSIZE) + (columnNum * COLUMNSIZE) + (Storage::m_nColumnWordTable[table][workY][workX] * 4) + byte;
			m_pageOffsets[y][x] = offset;
		}
	}
	m_pageOffsetsInitialized = true;
}

typedef CPixelIndexor<STORAGEPSMT8x> CPixelIndexorPSMT8;
typedef CPixelIndexor<STORAGEPSMT4x> CPixelIndexorPSMT4;

template <typename Storage>
bool CPixelIndexor<Storage>::m_pageOffsetsInitialized = false;

template <typename Storage>
uint32 CPixelIndexor<Storage>::m_pageOffsets[Storage::PAGEHEIGHT][Storage::PAGEWIDTH];

static
uint8* m_pCvtBuffer;

const int STORAGEPSMT4x::m_nBlockSwizzleTable[8][4] =
{
	{	0,	2,	8,	10,	},
	{	1,	3,	9,	11,	},
	{	4,	6,	12,	14,	},
	{	5,	7,	13,	15,	},
	{	16,	18,	24,	26,	},
	{	17,	19,	25,	27,	},
	{	20,	22,	28,	30,	},
	{	21,	23,	29,	31,	}
};

const int STORAGEPSMT4x::m_nColumnWordTable[2][2][8] =
{
	{
		{	0,	1,	4,	5,	8,	9,	12,	13,	},
		{	2,	3,	6,	7,	10,	11,	14,	15,	},
	},
	{
		{	8,	9,	12,	13,	0,	1,	4,	5,	},
		{	10,	11,	14,	15,	2,	3,	6,	7,	},
	},
};


const int STORAGEPSMT8x::m_nBlockSwizzleTable[4][8] =
{
	{	0,	1,	4,	5,	16,	17,	20,	21	},
	{	2,	3,	6,	7,	18,	19,	22,	23	},
	{	8,	9,	12,	13,	24,	25,	28,	29	},
	{	10,	11,	14,	15,	26,	27,	30,	31	},
};

const int STORAGEPSMT8x::m_nColumnWordTable[2][2][8] =
{
	{
		{	0,	1,	4,	5,	8,	9,	12,	13,	},
		{	2,	3,	6,	7,	10,	11,	14,	15,	},
	},
	{
		{	8,	9,	12,	13,	0,	1,	4,	5,	},
		{	10,	11,	14,	15,	2,	3,	6,	7,	},
	},
};

#ifdef __arm__

#include <arm_neon.h>

inline
void convertColumn8(uint8* dest, const int destStride, uint8* src, int colNum)
{
	// This sucks in the entire column and de-interleaves it
	uint8x16x4_t data = vld4q_u8(src);

	// https://developer.arm.com/documentation/den0018/a/NEON-Intrinsics-Reference/Intrinsics-type-conversion/VCOMBINE

	// VCOMBINE joins two 64-bit vectors into a single 128-bit vector. 
	// The lower half of the output vector contains the elements of the first input vector.

	uint16x8_t row0 = vcombine_u16(vmovn_u32(vreinterpretq_u32_u8(data.val[0])), vmovn_u32(vreinterpretq_u32_u8(data.val[2])));
	uint16x8_t revr0 = vrev32q_u16(vreinterpretq_u16_u8(data.val[0]));
	uint16x8_t revr2 = vrev32q_u16(vreinterpretq_u16_u8(data.val[2]));
	uint16x8_t row1 = vcombine_u16(vmovn_u32(vreinterpretq_u32_u16(revr0)), vmovn_u32(vreinterpretq_u32_u16(revr2)));

	uint16x8_t row2 = vcombine_u16(vmovn_u32(vreinterpretq_u32_u8(data.val[1])), vmovn_u32(vreinterpretq_u32_u8(data.val[3])));
	uint16x8_t revr1 = vrev32q_u16(vreinterpretq_u16_u8(data.val[1]));
	uint16x8_t revr3 = vrev32q_u16(vreinterpretq_u16_u8(data.val[3]));
	uint16x8_t row3 = vcombine_u16(vmovn_u32(vreinterpretq_u32_u16(revr1)), vmovn_u32(vreinterpretq_u32_u16(revr3)));

	if ((colNum & 1) == 0){
		row2 = vreinterpretq_u16_u32(vrev64q_u32(vreinterpretq_u32_u16(row2)));
		row3 = vreinterpretq_u16_u32(vrev64q_u32(vreinterpretq_u32_u16(row3)));
	}
	else {
		row0 = vreinterpretq_u16_u32(vrev64q_u32(vreinterpretq_u32_u16(row0)));
		row1 = vreinterpretq_u16_u32(vrev64q_u32(vreinterpretq_u32_u16(row1)));
	}

	vst1q_u8(dest, vreinterpretq_u8_u16(row0));
	vst1q_u8(dest+destStride, vreinterpretq_u8_u16(row1));
	vst1q_u8(dest+2*destStride, vreinterpretq_u8_u16(row2));
	vst1q_u8(dest+3*destStride, vreinterpretq_u8_u16(row3));
}
#else

#include <xmmintrin.h>
#include <emmintrin.h>

inline
void convertColumn8(uint8* dest, const int destStride, uint8* src, int colNum)
{
	__m128i* mSrc = (__m128i*)src;

	__m128i a = mSrc[0];
	__m128i b = mSrc[1];
	__m128i c = mSrc[2];
	__m128i d = mSrc[3];

	__m128i* mdest = (__m128i*)dest;

	__m128i temp_a = a;
	__m128i temp_c = c;

	a = _mm_unpacklo_epi8(temp_a, b);
	c = _mm_unpackhi_epi8(temp_a, b);
	b = _mm_unpacklo_epi8(temp_c, d);
	d = _mm_unpackhi_epi8(temp_c, d);

	temp_a = a;
	temp_c = c;

	a = _mm_unpacklo_epi16(temp_a, b);
	c = _mm_unpackhi_epi16(temp_a, b);
	b = _mm_unpacklo_epi16(temp_c, d);
	d = _mm_unpackhi_epi16(temp_c, d);

	temp_a = a;
	__m128i temp_b = b;

	a = _mm_unpacklo_epi8(temp_a, c);
	b = _mm_unpackhi_epi8(temp_a, c);
	c = _mm_unpacklo_epi8(temp_b, d);
	d = _mm_unpackhi_epi8(temp_b, d);

	temp_a = a;
	temp_c = c;

	a = _mm_unpacklo_epi64(temp_a, b);
	c = _mm_unpackhi_epi64(temp_a, b);
	b = _mm_unpacklo_epi64(temp_c, d);
	d = _mm_unpackhi_epi64(temp_c, d);

	if ((colNum & 1) == 0){
		c = _mm_shuffle_epi32(c, _MM_SHUFFLE(2, 3, 0, 1));
		d = _mm_shuffle_epi32(d, _MM_SHUFFLE(2, 3, 0, 1));
	}
	else {
		a = _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1));
		b = _mm_shuffle_epi32(b, _MM_SHUFFLE(2, 3, 0, 1));
	}

	int mStride = destStride / 16;

	mdest[0] = a;
	mdest[mStride] = b;
	mdest[mStride*2] = c;
	mdest[mStride*3] = d;
}

#endif


void TexUpdater_PSMT8(uint8* pCvtBuffer, uint8* pRam, unsigned int bufPtr, unsigned int bufWidth, unsigned int texX, unsigned int texY, unsigned int texWidth, unsigned int texHeight)
{
	// Assume transfer is aligned to block boundaries (16 x 16 pixels)
	CPixelIndexorPSMT8 indexor(pRam, bufPtr, bufWidth);

	uint8* dst = pCvtBuffer;
	for (unsigned int y = 0; y < texHeight; y += 16)
	{
		for (unsigned int x = 0; x < texWidth; x += 16)
		{
			uint8* colDst = dst;
			uint8* src = indexor.GetPixelAddress(texX + x, texY + y);

			// process an entire 16x16 block.
			// A column (64 bytes) is 16x4 pixels and they stack vertically in a block
			
			// we could read an entire column in 4 xmm registers or ARM NEON registers.
			int colNum = 0;
			for (unsigned int coly = 0; coly < 16; coly += 4) {
				convertColumn8(colDst + x, texWidth, src, colNum++);
				src += 64;
				colDst += texWidth * 4;
			}
		}

		dst += texWidth * 16;
	}
}


void TexUpdater_PSMT4(uint8* pCvtBuffer, uint8* pRam, unsigned int bufPtr, unsigned int bufWidth, unsigned int texX, unsigned int texY, unsigned int texWidth, unsigned int texHeight)
{
	CPixelIndexorPSMT4 indexor(pRam, bufPtr, bufWidth);

	uint8* dst = pCvtBuffer;
	for (unsigned int y = 0; y < texHeight; y++)
	{
		for (unsigned int x = 0; x < texWidth; x++)
		{
			uint8 pixel = indexor.GetPixel(texX + x, texY + y);
			dst[x] = pixel;
		}

		dst += texWidth;
	}

	//glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY, texWidth, texHeight, GL_RED, GL_UNSIGNED_BYTE, m_pCvtBuffer);
}


enum CVTBUFFERSIZE
{
	CVTBUFFERSIZE = 0x800000,
};

void runSSEVersionPSMT8(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH)
{
	for (int i = 0; i < loops; ++i) {
		TexUpdater_PSMT8(pCvtBuffer, pRAM, 0, 1024/64, 0, 0, texW, texH);
	}
}

void runSSEVersionPSMT4(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH)
{
	for (int i = 0; i < loops; ++i) {
		TexUpdater_PSMT4(pCvtBuffer, pRAM, 0, 1024 / 64, 0, 0, texW, texH);
	}
}

/*
* Notes on GS Local memory
* 
Page 8k
Block 256 bytes
Column 64 bytes (512 bits)

1 page = 32 blocks
1 block = 4 columns

PSMCT32 page 64x32,   block 8x8,   column 8x2
PSMCT16 page 64x64,   block 16x8,  column 16x2
PSMT8   page 128x128, block 16x16, column 16x4
PSMT4   page 128x64 , block 32x16, column 32x4

PSMT8
		4 columns per block, stacked vertically.
		
*/

