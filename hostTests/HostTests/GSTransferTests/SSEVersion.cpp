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

	uint32 m_nPointer;
	uint32 m_nWidth;
	uint8* m_pMemory;

	static bool m_pageOffsetsInitialized;
	static uint32 m_pageOffsets[Storage::PAGEHEIGHT][Storage::PAGEWIDTH];
};

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

template <typename Storage>
bool CPixelIndexor<Storage>::m_pageOffsetsInitialized = false;

template <typename Storage>
uint32 CPixelIndexor<Storage>::m_pageOffsets[Storage::PAGEHEIGHT][Storage::PAGEWIDTH];

static
uint8* m_pCvtBuffer;

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

inline
void convertColumn8Slow(uint8* dest, const int destStride, uint8* src, int colNum)
{
	// not meant to be faster ... just to clarify what the transform is to help
	// implementing the SSE code.
	const uint32* src32 = (uint32*)src;
	uint32* dest32 = (uint32*)dest;

	const uint32 s0 = src32[0];
	const uint32 s1 = src32[1];
	const uint32 s4 = src32[4];
	const uint32 s5 = src32[5];

	uint32 d = s0 & 0xFF | ((s1 & 0xFF) << 8) | ((s4 & 0xFF) << 16) | ((s5 & 0xFF) << 24);
	dest32[0] = d;

	const uint32 s8 = src32[8];
	const uint32 s9 = src32[9];
	const uint32 s12 = src32[12];
	const uint32 s13 = src32[13];
	d = s8 & 0xFF | ((s9 & 0xFF) << 8) | ((s12 & 0xFF) << 16) | ((s13 & 0xFF) << 24);
	dest32[1] = d;

	d = ((s0 >> 8) & 0xFF) | ((s1 & 0xFF00)) | ((s4 & 0xFF00) << 8) | ((s5 & 0xFF00) << 16);
	dest32[2] = d;
	d = ((s8 >> 8) & 0xFF) | ((s9 & 0xFF00)) | ((s12 & 0xFF00) << 8) | ((s13 & 0xFF00) << 16);
	dest32[3] = d;

	// rubbish but representative timing.
	for (int x = 0; x < 12; ++x) {
		d = ((s8 >> 8) & 0xFF) | ((s9 & 0xFF00)) | ((s12 & 0xFF00) << 8) | ((s13 & 0xFF00) << 16);
		dest32[3] = d;
	}
}

template <typename IndexorType>
void TexUpdater_Psm48(uint8* pCvtBuffer, uint8* pRam, unsigned int bufPtr, unsigned int bufWidth, unsigned int texX, unsigned int texY, unsigned int texWidth, unsigned int texHeight)
{
	// Assume transfer is aligned to block boundaries (16 x 16 pixels)
	IndexorType indexor(pRam, bufPtr, bufWidth);

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
				convertColumn8Slow(colDst + x, texWidth, src, colNum++);
				colDst += 4;
			}
		}

		dst += texWidth * 16;
	}
}


enum CVTBUFFERSIZE
{
	CVTBUFFERSIZE = 0x800000,
};

void runSSEVersion(uint8* pCvtBuffer, uint8* pRAM)
{
	for (int i = 0; i < 10000; ++i) {
		TexUpdater_Psm48<CPixelIndexorPSMT8>(pCvtBuffer, pRAM, 0, 1024, 0, 0, 512, 512);
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

