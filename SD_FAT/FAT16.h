/*FAT16.h - LINO TECH
  Designed by Carter
  2008-03-19
*/

#include "xtype.h"

#pragma pack(1)
//-------------------------------------------------------------------------
typedef struct{
	uint8_t		PB_BootIndicator;
	uint8_t		PB_StartHead;
	uint16_t	PB_StartSector;
	uint8_t		PB_SystemID;
	uint8_t		PB_EndHead;
	uint16_t	PB_EndSector;
	uint32_t	PB_RelativeSector;
	uint32_t	PB_TotalSector;		
} FAT_PB;
//-------------------------------------------------------------------------
typedef struct{
	uint8_t		MBR_mbr[446];
	FAT_PB		MBR_pb[4];
	uint16_t	MBR_Signature;
} FAT_MBR;
//-------------------------------------------------------------------------
typedef struct{
	uint8_t		BS_jmpBoot[3];
	uint8_t		BS_OEMName[8];
	uint16_t	BPB_BytesPerSec;
	uint8_t		BPB_SecPerClus;
	uint16_t	BPB_RsvdSecCnt;
	uint8_t		BPB_NumFATs;
	uint16_t	BPB_RootEntCnt;
	uint16_t	BPB_TotSec16;
	uint8_t		BPB_Media;
	uint16_t	BPB_FATSz16;
	uint16_t	BPB_SecPerTrk;
	uint16_t	BPB_NumHeads;
	uint32_t	BPB_HiddSec;
	uint32_t	BPB_TotSec32;
	uint8_t		BS_DrvNum;
	uint8_t		BS_Reservedl;
	uint8_t		BS_BootSig;
	uint32_t	BS_VolID;
	uint8_t		BS_VolLab[11];
	uint8_t		BS_FilSysType[8];
	uint8_t		ExecutableCode[448];
	uint8_t		ExecutableMarker[2];
} FAT_BPB;
//-------------------------------------------------------------------------
typedef struct{
	uint8_t		NAME[8];
	uint8_t		TYPE[3];
} FILE_NAME;
//-------------------------------------------------------------------------
typedef struct{
	uint16_t	Start;
	uint32_t	Size;
} FILE_POSIT;
//-------------------------------------------------------------------------
typedef struct{
	FILE_NAME	FileName;
	uint8_t		FileAttrib;
	uint8_t		UnUsed[10];
	uint8_t		FileUpdateTime[2];
	uint8_t		FileUpdateData[2];
	FILE_POSIT	FilePosit;
} DIR;
//-------------------------------------------------------------------------
typedef struct{
	uint16_t	ClusID;
	uint16_t	SecOfClus;
	uint16_t	ByteOfSec;
} DATA_POSIT;

#pragma pack()
//-------------------------------------------------------------------------
#define	SD_SUCC				0
#define	SD_FAIL				1
//-------------------------------------------------------------------------
uint8_t InitFat16(void);
uint8_t CreateFile(uint8_t* Name,uint32_t Size);
uint8_t EreaseFile(uint8_t Name[11]);
uint8_t ReadFile(uint8_t Name[11],uint32_t Start,uint32_t len,uint8_t *p);
uint8_t WriteFile(uint8_t Name[11],uint32_t Start,uint32_t len,uint8_t *p);
//-------------------------------------------------------------------------
