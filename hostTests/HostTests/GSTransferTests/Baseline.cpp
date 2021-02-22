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

enum RAMSIZE
{
	RAMSIZE = 0x00400000,
};

struct STORAGEPSMT8
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

struct STORAGEPSMT4
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
inline uint8 CPixelIndexor<STORAGEPSMT4>::GetPixel(unsigned int nX, unsigned int nY)
{
	typedef STORAGEPSMT4 Storage;

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
inline void CPixelIndexor<STORAGEPSMT4>::SetPixel(unsigned int nX, unsigned int nY, uint8 nPixel)
{
	typedef STORAGEPSMT4 Storage;

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
inline void CPixelIndexor<STORAGEPSMT4>::BuildPageOffsetTable()
{
	if (m_pageOffsetsInitialized) return;

	typedef STORAGEPSMT4 Storage;

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
inline void CPixelIndexor<STORAGEPSMT8>::BuildPageOffsetTable()
{
	if (m_pageOffsetsInitialized) return;
	typedef STORAGEPSMT8 Storage;

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

typedef CPixelIndexor<STORAGEPSMT8> CPixelIndexorPSMT8;
typedef CPixelIndexor<STORAGEPSMT4> CPixelIndexorPSMT4;

template <typename Storage>
bool CPixelIndexor<Storage>::m_pageOffsetsInitialized = false;

template <typename Storage>
uint32 CPixelIndexor<Storage>::m_pageOffsets[Storage::PAGEHEIGHT][Storage::PAGEWIDTH];

const int STORAGEPSMT4::m_nBlockSwizzleTable[8][4] =
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

const int STORAGEPSMT4::m_nColumnWordTable[2][2][8] =
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

const int STORAGEPSMT8::m_nBlockSwizzleTable[4][8] =
{
	{	0,	1,	4,	5,	16,	17,	20,	21	},
	{	2,	3,	6,	7,	18,	19,	22,	23	},
	{	8,	9,	12,	13,	24,	25,	28,	29	},
	{	10,	11,	14,	15,	26,	27,	30,	31	},
};

const int STORAGEPSMT8::m_nColumnWordTable[2][2][8] =
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

template <typename IndexorType>
void TexUpdater_Psm48(uint8* pCvtBuffer, uint8* pRam, unsigned int bufPtr, unsigned int bufWidth, unsigned int texX, unsigned int texY, unsigned int texWidth, unsigned int texHeight)
{
	IndexorType indexor(pRam, bufPtr, bufWidth);

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

void runBaselinePSMT8(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH)
{
	for (int i = 0; i < loops; ++i) {
		TexUpdater_Psm48<CPixelIndexorPSMT8>(pCvtBuffer, pRAM, 0, 1024/64, 0, 0, texW, texH);
	}
}

void runBaselinePSMT4(int loops, uint8* pCvtBuffer, uint8* pRAM, int texW, int texH)
{
	for (int i = 0; i < loops; ++i) {
		TexUpdater_Psm48<CPixelIndexorPSMT4>(pCvtBuffer, pRAM, 0, 1024 / 64, 0, 0, texW, texH);
	}
}
