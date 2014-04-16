//-------------------------------------------------------------------------
/*FAT16.C - LINO TECH
  Designed by Carter
  2008-03-19
*/

#include 	<stdint.h>

#include	"FAT16.h"
#include	"sd.h"
//------------------------------------------------------------------------
#define	SEC_Size				512
#define	MBR_Sector				0				//绝对地址
#define	FAT_Sector				0				//逻辑地址
//-------------------------------------------------------------------------
uint8_t		BUFFER[SEC_Size];
uint8_t		PB_RelativeSector;
uint16_t	BPB_BytesPerSec;
uint8_t		BPB_SecPerClus;
uint16_t	BPB_RsvdSecCnt;
uint8_t		BPB_NumFATs;
uint16_t	BPB_RootEntCnt;
uint16_t	BPB_TotSec16;
uint16_t	BPB_FATSz16;							//FAT占用的sectors
uint32_t	BPB_HiddSec;
//-------------------------------------------------------------------------
uint8_t ReadSector(uint32_t sector, uint8_t* buffer )
{
    int8 stat;
    
    hwInterface sdNow;  
    sdNow.sectorCount = 1;
    stat = sd_readSector(&sdNow,sector,buffer,512);
    
    if(stat==0)
    {
        return SD_SUCC;
    }
    else
    {
        return SD_FAIL;
    };
}
uint8_t WriteSector(uint32_t sector, uint8_t* buffer)
{
    hwInterface sdNow;  
    sdNow.sectorCount = 1;
    sd_writeSector(&sdNow,sector,buffer);
    
    return 0;
}

uint8_t ReadBlock(uint32_t LBA){					//绝对地址读一个扇区
	if(ReadSector(LBA,BUFFER)!=0)return SD_FAIL;
	return SD_SUCC;
}
//-------------------------------------------------------------------------
uint8_t WriteBlock(uint32_t LBA){					//绝对地址写一个扇区
	if(WriteSector(LBA,BUFFER)!=0)return SD_FAIL;
	return SD_SUCC;
}
//-------------------------------------------------------------------------
uint8_t ReadFatBlock(uint32_t Add){					//逻辑地址读一个扇区
//	return ReadBlock(Add+PB_RelativeSector);  //for HardDisk
  	return ReadBlock(Add+BPB_HiddSec);
}
//-------------------------------------------------------------------------
uint8_t WriteFatBlock(uint32_t Add){				//逻辑地址写一个扇区
//	return WriteBlock(Add+PB_RelativeSector);  //for HardDisk
    return WriteBlock(Add+BPB_HiddSec);
}
//-------------------------------------------------------------------------
void CopyBytes(uint8_t *ps,uint8_t *pd,uint16_t size){	//内存拷贝
	for(;size;size--)*pd++=*ps++;
}
//-------------------------------------------------------------------------
uint8_t IsEqual(uint8_t *pa,uint8_t *pb,uint8_t size){	//内存比较
	for(;size;size--)if(*pa++!=*pb++)return 0;
	return 1;
}
//-------------------------------------------------------------------------
void EmptyBytes(uint8_t *pd,uint16_t size){			//内存清空
	for(;size;size--)*pd++ =0;
}
//-------------------------------------------------------------------------
uint8_t ReadMBR(void){									//读取MBR数据结构
	uint8_t ok;
	FAT_MBR * MBR=(FAT_MBR*)BUFFER;
	ok=ReadBlock(MBR_Sector);
	if(ok==SD_FAIL)return SD_FAIL;
	if(MBR->MBR_Signature!=0xAA55)return SD_FAIL;		//读有效标志
		
	//获取参数
	PB_RelativeSector=MBR->MBR_pb[0].PB_RelativeSector;//读逻辑地址与绝对地址的偏移
	return SD_SUCC;
}
//-------------------------------------------------------------------------
uint8_t ReadBPB(void){									//读取BPB数据结构
	uint8_t ok;
	FAT_BPB * BPB=(FAT_BPB*)BUFFER;
//	ok=ReadFatBlock(FAT_Sector);
   	ok=ReadBlock(FAT_Sector);
	if(ok==SD_FAIL)return SD_FAIL;
	
	//获取参数
	BPB_BytesPerSec = BPB->BPB_BytesPerSec;
	BPB_SecPerClus = BPB->BPB_SecPerClus;
	BPB_RsvdSecCnt = BPB->BPB_RsvdSecCnt;
	BPB_NumFATs = BPB->BPB_NumFATs;
	BPB_RootEntCnt = BPB->BPB_RootEntCnt;
	BPB_TotSec16 = BPB->BPB_TotSec16;
	BPB_FATSz16 = BPB->BPB_FATSz16;
	BPB_HiddSec = BPB->BPB_HiddSec;
	return SD_SUCC;
}
//-------------------------------------------------------------------------
uint32_t DirStartSec(void){							//获取根目录开始扇区号
	return BPB_RsvdSecCnt+BPB_NumFATs*BPB_FATSz16;
}
//-------------------------------------------------------------------------
uint16_t GetDirSecCount(void){							//目录项占用的扇区数
	return BPB_RootEntCnt*32/BPB_BytesPerSec;
}
//-------------------------------------------------------------------------
uint32_t DataStartSec(void){							//获取数据区开始扇区号
	return DirStartSec()+GetDirSecCount();
}
//-------------------------------------------------------------------------
uint32_t ClusConvLBA(uint16_t ClusID){					//获取一个簇的开始扇区
	return DataStartSec()+BPB_SecPerClus*(ClusID-2);
}
//-------------------------------------------------------------------------
uint16_t ReadFAT(uint16_t Index){						//读取文件分配表的指定项
	uint16_t *RAM=(uint16_t*)BUFFER;
	uint32_t SecID;
	
	SecID=BPB_RsvdSecCnt+Index/256;
	ReadFatBlock(SecID);
	return RAM[Index%256];
}
//-------------------------------------------------------------------------
void WriteFAT(uint16_t Index,uint16_t Value){			//写文件分配表的指定项
	uint16_t *RAM=(uint16_t*)BUFFER;
	uint32_t SecID;
	
	SecID=BPB_RsvdSecCnt+Index/256;
	ReadFatBlock(SecID);
	RAM[Index%256]=Value;
	WriteFatBlock(SecID);
}
//-------------------------------------------------------------------------
uint16_t GetEmptyDIR(void){							//获取根目录中可以使用的一项
	uint16_t i,DirSecCut,DirStart,m,ID=0;
	
	DirSecCut=GetDirSecCount();
	DirStart=DirStartSec();
	for(i=0;i<DirSecCut;i++){
		ReadFatBlock(DirStart+i);
		for(m=0;m<16;m++){
			if(BUFFER[m*32]==0)return ID;
			if(BUFFER[m*32]==0xe5)return ID;
			ID++;
		}
	}
	return ID;
}
//-------------------------------------------------------------------------
//获得和文件名对应的目录
uint8_t GetFileID(uint8_t Name[11],DIR *ID,uint16_t *pIndex){
	uint16_t  i,DirSecCut,DirStart,m;
	
	DirSecCut = GetDirSecCount();
	DirStart = DirStartSec();
	for(i=0,*pIndex=0;i<DirSecCut;i++){
		ReadFatBlock(DirStart+i);
		for(m=0;m<16;m++){
			if(IsEqual(Name,(uint8_t *)&((DIR*)&BUFFER[m*32])->FileName,11)){
				*ID = *((DIR*)&BUFFER[m*32]);
				return 1; 						//找到对应的目录项,返回1.
			}
			(*pIndex)++;
		}
	}
	return 0; 									//没有找到对应的目录项,返回0.
}
//-------------------------------------------------------------------------
uint16_t GetNextFAT(void){							//获取一个空的FAT项
	uint16_t FAT_Count,i;
	FAT_Count=BPB_FATSz16*256; 						//FAT表总项数
	for(i=0;i<FAT_Count;i++){
		if(ReadFAT(i)==0)return i;
	}
	return 0;
}
//-------------------------------------------------------------------------
void ReadDIR(uint16_t Index, DIR* Value){			//读取根目录的指定项
	uint32_t LBA = DirStartSec()+Index/16;
	ReadFatBlock(LBA);
	CopyBytes((uint8_t *)&BUFFER[(Index%16)*32],(uint8_t *)Value,32);
}
//-------------------------------------------------------------------------
void WriteDIR(uint16_t Index, DIR* Value){			//写根目录的指定项
	uint32_t LBA = DirStartSec()+Index/16;
	ReadFatBlock(LBA);
	CopyBytes((uint8_t *)Value,(uint8_t *)&BUFFER[(Index%16)*32],32);
	WriteFatBlock(LBA);
}
//-------------------------------------------------------------------------
void CopyFAT(void){						//复制文件分配表,使其和备份一致
	uint16_t i;

	for(i=0;i<BPB_FATSz16;i++){
		ReadFatBlock(BPB_RsvdSecCnt+i);
		WriteFatBlock(BPB_RsvdSecCnt+BPB_FATSz16+i);
	}
}
//-------------------------------------------------------------------------
uint8_t CreateFile(uint8_t *Name,uint32_t Size){	//创建一个空文件
	uint16_t ClusID, ClusNum, ClusNext, i,dirIndex;
	DIR FileDir;
	
	if(GetFileID(Name,&FileDir,&dirIndex)==1)return SD_FAIL;	//文件已存在
	
	ClusNum=Size/(BPB_SecPerClus*BPB_BytesPerSec)+1;
	
	EmptyBytes((uint8_t *)&FileDir,sizeof(DIR));
	CopyBytes(Name,(uint8_t *)&FileDir.FileName,11);
	FileDir.FilePosit.Size=Size;
	FileDir.FilePosit.Start=GetNextFAT();
	ClusID=FileDir.FilePosit.Start;
//	for(i=0;i<ClusNum-1;i++){
   	for(i=0;i<ClusNum;i++){
		WriteFAT(ClusID,0xffff);
		ClusNext=GetNextFAT();
		WriteFAT(ClusID,ClusNext);
		ClusID=ClusNext;
	}
	WriteFAT(ClusID, 0xffff);
	WriteDIR(GetEmptyDIR(),&FileDir);
	CopyFAT();
	return SD_SUCC;
}
//-------------------------------------------------------------------------
//读文件
uint8_t ReadFile(uint8_t Name[11],uint32_t Start,uint32_t len,uint8_t *p){
	uint16_t BytePerClus,ClusID,m,dirIndex;
	uint32_t LBA;
	uint8_t	 i;
	DIR      FileDir;
	
	if(GetFileID(Name,&FileDir,&dirIndex)==0)return SD_FAIL;//文件不存在
	
	BytePerClus=BPB_SecPerClus*BPB_BytesPerSec;		//每簇的字节数	
	m=Start/BytePerClus;							//计算开始位置包含的簇数
	ClusID=FileDir.FilePosit.Start;					//文件的开始簇号
	for(i=0;i<m;i++)ClusID=ReadFAT(ClusID);		//计算开始位置所在簇的簇号	
	i=(Start%BytePerClus)/BPB_BytesPerSec;			//计算开始位置所在扇区的簇内偏移
	LBA=ClusConvLBA(ClusID)+i;						//计算开始位置的逻辑扇区号
	m=(Start%BytePerClus)%BPB_BytesPerSec;			//计算开始位置在扇区内偏移

READ:
	for(;i<BPB_SecPerClus;i++){
		ReadFatBlock(LBA++);
		for(;m<BPB_BytesPerSec;m++){
			*p++=BUFFER[m];
			if(--len==0)return SD_SUCC;			//如果读取完成就退出
		}
		m=0;
	}
	i=0;
	ClusID=ReadFAT(ClusID);							//下一簇簇号
	LBA=ClusConvLBA(ClusID);
	goto READ;
}
//-------------------------------------------------------------------------
//写文件
uint8_t WriteFile(uint8_t Name[11],uint32_t Start,uint32_t len,uint8_t *p){
	uint16_t BytePerClus,ClusID,m,dirIndex;
	uint32_t LBA;
	uint8_t	 i;
	DIR      FileDir;
	
	if(GetFileID(Name,&FileDir,&dirIndex)==0)return SD_FAIL;//文件不存在
	
	BytePerClus=BPB_SecPerClus*BPB_BytesPerSec;		// 每簇的字节数	
	m=Start/BytePerClus;							//计算开始位置包含的簇数
	ClusID=FileDir.FilePosit.Start;					//文件的开始簇号
	for(i=0;i<m;i++)ClusID=ReadFAT(ClusID);		//计算开始位置所在簇的簇号	
	i=(Start%BytePerClus)/BPB_BytesPerSec;			//计算开始位置所在扇区的簇内偏移
	LBA=ClusConvLBA(ClusID)+i;						//计算开始位置的逻辑扇区号
	m=(Start%BytePerClus)%BPB_BytesPerSec;			//计算开始位置在扇区内偏移

WRITE:
	for(;i<BPB_SecPerClus;i++){
		ReadFatBlock(LBA);
		for(;m<BPB_BytesPerSec;m++){
			BUFFER[m]=*p++;
			if(--len==0){							//如果读取完成就退出
				WriteFatBlock(LBA);					//回写扇区
				return SD_SUCC;				
			}
		}
		m=0;
		WriteFatBlock(LBA++);						//回写扇区
	}
	i=0;
	ClusID=ReadFAT(ClusID);							//下一簇簇号
	LBA=ClusConvLBA(ClusID);
	goto WRITE;
}
//-------------------------------------------------------------------------
uint8_t InitFat16(void){							//初始化FAT16的变量
//	if(ReadMBR()==SD_FAIL)return SD_FAIL;  //for HardDisk
	if(ReadBPB()==SD_FAIL)return SD_FAIL;

	return SD_SUCC;
}
//-------------------------------------------------------------------------
//删除文件
uint8_t EreaseFile(uint8_t Name[11]){
	uint16_t ClusID,ClusNext,dirIndex;
	DIR FileDir;
	
	if(GetFileID(Name,&FileDir,&dirIndex)==0)return SD_FAIL;	//文件不存在
	ClusID=FileDir.FilePosit.Start;					//文件的开始簇号

EREASEFAT:
	if((ClusNext=ReadFAT(ClusID))!=0xffff){		//删除FAT表中的链表
		WriteFAT(ClusID,0x0000);
		ClusID=ClusNext;
	}else{
		WriteFAT(ClusID,0x0000);
		goto EREASEFATEND;
	}
	goto EREASEFAT;
EREASEFATEND:
	
	FileDir.FileName.NAME[0]=0xe5;					//删除Dir中的文件记录
	WriteDIR(dirIndex,&FileDir);
	CopyFAT();										//FAT2<-FAT1
	return SD_SUCC;
}
//-------------------------------------------------------------------------
