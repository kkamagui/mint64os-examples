/**
 *  file    FileSystem.h
 *  date    2009/05/01
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief  파일 시스템에 관련된 헤더 파일
 */

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로와 함수 포인터
//
////////////////////////////////////////////////////////////////////////////////
// MINT 파일 시스템 시그너처(Signature)
#define FILESYSTEM_SIGNATURE                0x7E38CF10
// 클러스터의 크기(섹터 수), 4Kbyte
#define FILESYSTEM_SECTORSPERCLUSTER        8
// 파일 클러스터의 마지막 표시
#define FILESYSTEM_LASTCLUSTER              0xFFFFFFFF
// 빈 클러스터 표시
#define FILESYSTEM_FREECLUSTER              0x00
// 루트 디렉터리에 있는 최대 디렉터리 엔트리의 수
#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT   ( ( FILESYSTEM_SECTORSPERCLUSTER * 512 ) / \
        sizeof( DIRECTORYENTRY ) )
// 클러스터의 크기(바이트 수)
#define FILESYSTEM_CLUSTERSIZE              ( FILESYSTEM_SECTORSPERCLUSTER * 512 )

// 파일 이름의 최대 길이
#define FILESYSTEM_MAXFILENAMELENGTH        24
// 하드 디스크 제어에 관련된 함수 포인터 타입 정의
typedef BOOL (* fReadHDDInformation ) ( BOOL bPrimary, BOOL bMaster, 
        HDDINFORMATION* pstHDDInformation );
typedef int (* fReadHDDSector ) ( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, 
        int iSectorCount, char* pcBuffer );
typedef int (* fWriteHDDSector ) ( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, 
        int iSectorCount, char* pcBuffer );


////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// 파티션 자료구조
typedef struct kPartitionStruct
{
    // 부팅 가능 플래그. 0x80이면 부팅 가능을 나타내며 0x00은 부팅 불가
    BYTE bBootableFlag;
    // 파티션의 시작 어드레스. 현재는 거의 사용하지 않으며 아래의 LBA 어드레스를 대신 사용
    BYTE vbStartingCHSAddress[ 3 ];
    // 파티션 타입
    BYTE bPartitionType;
    // 파티션의 마지막 어드레스. 현재는 거의 사용 안 함
    BYTE vbEndingCHSAddress[ 3 ];
    // 파티션의 시작 어드레스. LBA 어드레스로 나타낸 값
    DWORD dwStartingLBAAddress;
    // 파티션에 포함된 섹터 수
    DWORD dwSizeInSector;
} PARTITION;


// MBR 자료구조
typedef struct kMBRStruct
{
    // 부트 로더 코드가 위치하는 영역
    BYTE vbBootCode[ 430 ];

    // 파일 시스템 시그너처, 0x7E38CF10
    DWORD dwSignature;
    // 예약된 영역의 섹터 수
    DWORD dwReservedSectorCount;
    // 클러스터 링크 테이블 영역의 섹터 수
    DWORD dwClusterLinkSectorCount;
    // 클러스터의 전체 개수
    DWORD dwTotalClusterCount;
    
    // 파티션 테이블
    PARTITION vstPartition[ 4 ];
    
    // 부트 로더 시그너처, 0x55, 0xAA
    BYTE vbBootLoaderSignature[ 2 ];
} MBR;


// 디렉터리 엔트리 자료구조
typedef struct kDirectoryEntryStruct
{
    // 파일 이름
    char vcFileName[ FILESYSTEM_MAXFILENAMELENGTH ];
    // 파일의 실제 크기
    DWORD dwFileSize;
    // 파일이 시작하는 클러스터 인덱스
    DWORD dwStartClusterIndex;
} DIRECTORYENTRY;

#pragma pack( pop )

// 파일 시스템을 관리하는 구조체
typedef struct kFileSystemManagerStruct
{
    // 파일 시스템이 정상적으로 인식되었는지 여부
    BOOL bMounted;
    
    // 각 영역의 섹터 수와 시작 LBA 어드레스
    DWORD dwReservedSectorCount;
    DWORD dwClusterLinkAreaStartAddress;
    DWORD dwClusterLinkAreaSize;
    DWORD dwDataAreaStartAddress;   
    // 데이터 영역의 클러스터의 총 개수
    DWORD dwTotalClusterCount;
    
    // 마지막으로 클러스터를 할당한 클러스터 링크 테이블의 섹터 오프셋을 저장
    DWORD dwLastAllocatedClusterLinkSectorOffset;
    
    // 파일 시스템 동기화 객체
    MUTEX stMutex;    
} FILESYSTEMMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
BOOL kInitializeFileSystem( void );
BOOL kFormat( void );
BOOL kMount( void );
BOOL kGetHDDInformation( HDDINFORMATION* pstInformation);

BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer );
BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer );
BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer );
BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer );
DWORD kFindFreeCluster( void );
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData );
BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData );
int kFindFreeDirectoryEntry( void );
BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry );
BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry );
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry );
void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager );

#endif /*__FILESYSTEM_H__*/
