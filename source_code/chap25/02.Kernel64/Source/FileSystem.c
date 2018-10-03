/**
 *  file    FileSystem.c
 *  date    2009/05/01
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief  파일 시스템에 관련된 헤더 파일
 */

#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"

// 파일 시스템 자료구조
static FILESYSTEMMANAGER   gs_stFileSystemManager;
// 파일 시스템 임시 버퍼
static BYTE gs_vbTempBuffer[ FILESYSTEM_SECTORSPERCLUSTER * 512 ];

// 하드 디스크 제어에 관련된 함수 포인터 선언
fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;

/**
 *  파일 시스템을 초기화
 */
BOOL kInitializeFileSystem( void )
{
    // 자료구조 초기화와 동기화 객체 초기화
    kMemSet( &gs_stFileSystemManager, 0, sizeof( gs_stFileSystemManager ) );
    kInitializeMutex( &( gs_stFileSystemManager.stMutex ) );
    
    // 하드 디스크를 초기화
    if( kInitializeHDD() == TRUE )
    {
        // 초기화가 성공하면 함수 포인터를 하드 디스크용 함수로 설정
        gs_pfReadHDDInformation = kReadHDDInformation;
        gs_pfReadHDDSector = kReadHDDSector;
        gs_pfWriteHDDSector = kWriteHDDSector;
    }
    else
    {
        return FALSE;
    }
    
    // 파일 시스템 연결
    if( kMount() == FALSE )
    {
        return FALSE;
    }
    
    return TRUE;
}

//==============================================================================
//  저수준 함수(Low Level Function)
//==============================================================================
/**
 *  하드 디스크의 MBR을 읽어서 MINT 파일 시스템인지 확인
 *      MINT 파일 시스템이라면 파일 시스템에 관련된 각종 정보를 읽어서
 *      자료구조에 삽입
 */
BOOL kMount( void )
{
    MBR* pstMBR;
    
    // 동기화 처리
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // MBR을 읽음
    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // 동기화 처리
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    
    // 시그너처를 확인하여 같다면 자료구조에 각 영역에 대한 정보 삽입
    pstMBR = ( MBR* ) gs_vbTempBuffer;
    if( pstMBR->dwSignature != FILESYSTEM_SIGNATURE )
    {
        // 동기화 처리
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    
    // 파일 시스템 인식 성공
    gs_stFileSystemManager.bMounted = TRUE;
    
    // 각 영역의 시작 LBA 어드레스와 섹터 수를 계산
    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress =
        pstMBR->dwReservedSectorCount + 1;
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    gs_stFileSystemManager.dwDataAreaStartAddress = 
        pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    // 동기화 처리
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}

/**
 *  하드 디스크에 파일 시스템을 생성
 */
BOOL kFormat( void )
{
    HDDINFORMATION* pstHDD;
    MBR* pstMBR;
    DWORD dwTotalSectorCount, dwRemainSectorCount;
    DWORD dwMaxClusterCount, dwClsuterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;
    
    // 동기화 처리
    kLock( &( gs_stFileSystemManager.stMutex ) );

    //==========================================================================
    //  하드 디스크 정보를 읽어서 메타 영역의 크기와 클러스터의 개수를 계산
    //==========================================================================
    // 하드 디스크의 정보를 얻어서 하드 디스크의 총 섹터 수를 구함
    pstHDD = ( HDDINFORMATION* ) gs_vbTempBuffer;
    if( gs_pfReadHDDInformation( TRUE, TRUE, pstHDD ) == FALSE )
    {
        // 동기화 처리
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }    
    dwTotalSectorCount = pstHDD->dwTotalSectors;
    
    // 전체 섹터 수를 4Kbyte, 즉 클러스터 크기로 나누어 최대 클러스터 수를 계산
    dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    
    // 최대 클러스터의 수에 맞추어 클러스터 링크 테이블의 섹터 수를 계산
    // 링크 데이터는 4바이트이므로, 한 섹터에는 128개가 들어감. 따라서 총 개수를
    // 128로 나눈 후 올림하여 클러스터 링크의 섹터 수를 구함
    dwClusterLinkSectorCount = ( dwMaxClusterCount + 127 ) / 128;
    
    // 예약된 영역은 현재 사용하지 않으므로, 디스크 전체 영역에서 MBR 영역과 클러스터
    // 링크 테이블 영역의 크기를 뺀 나머지가 실제 데이터 영역이 됨
    // 해당 영역을 클러스터 크기로 나누어 실제 클러스터의 개수를 구함
    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    dwClsuterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    
    // 실제 사용 가능한 클러스터 수에 맞추어 다시 한번 계산
    dwClusterLinkSectorCount = ( dwClsuterCount + 127 ) / 128;

    //==========================================================================
    // 계산된 정보를 MBR에 덮어 쓰고, 루트 디렉터리 영역까지 모두 0으로 초기화하여
    // 파일 시스템을 생성
    //==========================================================================
    // MBR 영역 읽기
    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // 동기화 처리
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }        
    
    // 파티션 정보와 파일 시스템 정보 설정    
    pstMBR = ( MBR* ) gs_vbTempBuffer;
    kMemSet( pstMBR->vstPartition, 0, sizeof( pstMBR->vstPartition ) );
    pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    pstMBR->dwReservedSectorCount = 0;
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClsuterCount;
    
    // MBR 영역에 1 섹터를 씀
    if( gs_pfWriteHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // 동기화 처리
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    
    // MBR 이후부터 루트 디렉터리까지 모두 0으로 초기화
    kMemSet( gs_vbTempBuffer, 0, 512 );
    for( i = 0 ; i < ( dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER );
         i++ )
    {
        // 루트 디렉터리(클러스터 0)는 이미 파일 시스템이 사용하고 있으므로,
        // 할당된 것으로 표시
        if( i == 0 )
        {
            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_LASTCLUSTER;
        }
        else
        {
            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_FREECLUSTER;
        }
        
        // 1 섹터씩 씀
        if( gs_pfWriteHDDSector( TRUE, TRUE, i + 1, 1, gs_vbTempBuffer ) == FALSE )
        {
            // 동기화 처리
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return FALSE;
        }
    }    
    
    // 동기화 처리
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}

/**
 *  파일 시스템에 연결된 하드 디스크의 정보를 반환
 */
BOOL kGetHDDInformation( HDDINFORMATION* pstInformation)
{
    BOOL bResult;
    
    // 동기화 처리
    kLock( &( gs_stFileSystemManager.stMutex ) );
    
    bResult = gs_pfReadHDDInformation( TRUE, TRUE, pstInformation );
    
    // 동기화 처리
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    
    return bResult;
}

/**
 *  클러스터 링크 테이블 내의 오프셋에서 한 섹터를 읽음
 */
BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    // 클러스터 링크 테이블 영역의 시작 어드레스를 더함
    return gs_pfReadHDDSector( TRUE, TRUE, dwOffset + 
              gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

/**
 *  클러스터 링크 테이블 내의 오프셋에 한 섹터를 씀
 */
BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    // 클러스터 링크 테이블 영역의 시작 어드레스를 더함
    return gs_pfWriteHDDSector( TRUE, TRUE, dwOffset + 
               gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

/**
 *  데이터 영역의 오프셋에서 한 클러스터를 읽음
 */
BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    // 데이터 영역의 시작 어드레스를 더함
    return gs_pfReadHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + 
              gs_stFileSystemManager.dwDataAreaStartAddress, 
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

/**
 *  데이터 영역의 오프셋에 한 클러스터를 씀
 */
BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    // 데이터 영역의 시작 어드레스를 더함
    return gs_pfWriteHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + 
              gs_stFileSystemManager.dwDataAreaStartAddress, 
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

/**
 *  클러스터 링크 테이블 영역에서 빈 클러스터를 검색함
 */
DWORD kFindFreeCluster( void )
{
    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;
    
    // 파일 시스템을 인식하지 못했으면 실패
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FILESYSTEM_LASTCLUSTER;
    }
    
    // 마지막으로 클러스터를 할당한 클러스터 링크 테이블의 섹터 오프셋을 가져옴
    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    // 마지막으로 할당한 위치부터 루프를 돌면서 빈 클러스터를 검색
    for( i = 0 ; i < gs_stFileSystemManager.dwClusterLinkAreaSize ; i++ )
    {
        // 클러스터 링크 테이블의 마지막 섹터이면 전체 섹터만큼 도는 것이 아니라
        // 남은 클러스터의 수만큼 루프를 돌아야 함
        if( ( dwLastSectorOffset + i ) == 
            ( gs_stFileSystemManager.dwClusterLinkAreaSize - 1 ) )
        {
            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128; 
        }
        else
        {
            dwLinkCountInSector = 128;
        }
        
        // 이번에 읽어야 할 클러스터 링크 테이블의 섹터 오프셋을 구해서 읽음
        dwCurrentSectorOffset = ( dwLastSectorOffset + i ) % 
            gs_stFileSystemManager.dwClusterLinkAreaSize;
        if( kReadClusterLinkTable( dwCurrentSectorOffset, gs_vbTempBuffer ) == FALSE )
        {
            return FILESYSTEM_LASTCLUSTER;
        }
        
        // 섹터 내에서 루프를 돌면서 빈 클러스터를 검색
        for( j = 0 ; j < dwLinkCountInSector ; j++ )
        {
            if( ( ( DWORD* ) gs_vbTempBuffer )[ j ] == FILESYSTEM_FREECLUSTER )
            {
                break;
            }
        }
            
        // 찾았다면 클러스터 인덱스를 반환
        if( j != dwLinkCountInSector )
        {
            // 마지막으로 클러스터를 할당한 클러스터 링크 내의 섹터 오프셋을 저장
            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = 
                dwCurrentSectorOffset;
            
            // 현재 클러스터 링크 테이블의 오프셋을 감안하여 클러스터 인덱스를 계산
            return ( dwCurrentSectorOffset * 128 ) + j;
        }
    }
    
    return FILESYSTEM_LASTCLUSTER;
}

/**
 *  클러스터 링크 테이블에 값을 설정
 */
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData )
{
    DWORD dwSectorOffset;
    
    // 파일 시스템을 인식하지 못했으면 실패
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FALSE;
    }
    
    // 한 섹터에 128개의 클러스터 링크가 들어가므로 128로 나누면 섹터 오프셋을 
    // 구할 수 있음
    dwSectorOffset = dwClusterIndex / 128;

    // 해당 섹터를 읽어서 링크 정보를 설정한 후, 다시 저장
    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    
    ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ] = dwData;

    if( kWriteClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    return TRUE;
}

/**
 *  클러스터 링크 테이블의 값을 반환
 */
BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData )
{
    DWORD dwSectorOffset;
    
    // 파일 시스템을 인식하지 못했으면 실패
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FALSE;
    }
    
    // 한 섹터에 128개의 클러스터 링크가 들어가므로 128로 나누면 섹터 오프셋을 
    // 구할 수 있음
    dwSectorOffset = dwClusterIndex / 128;
    
    if( dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize )
    {
        return FALSE;
    }
    
    
    // 해당 섹터를 읽어서 링크 정보를 반환
    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    

    *pdwData = ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ];
    return TRUE;
}


/**
 *  루트 디렉터리에서 빈 디렉터리 엔트리를 반환
 */
int kFindFreeDirectoryEntry( void )
{
    DIRECTORYENTRY* pstEntry;
    int i;

    // 파일 시스템을 인식하지 못했으면 실패
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return -1;
    }

    // 루트 디렉터리를 읽음
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return -1;
    }
    
    // 루트 디렉터리 안에서 루프를 돌면서 빈 엔트리, 즉 시작 클러스터 번호가 0인
    // 엔트리를 검색
    pstEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    {
        if( pstEntry[ i ].dwStartClusterIndex == 0 )
        {
            return i;
        }
    }
    return -1;
}

/**
 *  루트 디렉터리의 해당 인덱스에 디렉터리 엔트리를 설정
 */
BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    
    // 파일 시스템을 인식하지 못했거나 인덱스가 올바르지 않으면 실패
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // 루트 디렉터리를 읽음
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    
    // 루트 디렉터리에 있는 해당 데이터를 갱신
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstRootEntry + iIndex, pstEntry, sizeof( DIRECTORYENTRY ) );

    // 루트 디렉터리에 씀
    if( kWriteCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    return TRUE;
}

/**
 *  루트 디렉터리의 해당 인덱스에 위치하는 디렉터리 엔트리를 반환
 */
BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    
    // 파일 시스템을 인식하지 못했거나 인덱스가 올바르지 않으면 실패
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // 루트 디렉터리를 읽음
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }    
    
    // 루트 디렉터리에 있는 해당 데이터를 갱신
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstEntry, pstRootEntry + iIndex, sizeof( DIRECTORYENTRY ) );
    return TRUE;
}

/**
 *  루트 디렉터리에서 파일 이름이 일치하는 엔트리를 찾아서 인덱스를 반환
 */
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    int i;
    int iLength;

    // 파일 시스템을 인식하지 못했으면 실패
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return -1;
    }

    // 루트 디렉터리를 읽음
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return -1;
    }
    
    iLength = kStrLen( pcFileName );
    // 루트 디렉터리 안에서 루프를 돌면서 파일 이름이 일치하는 엔트리를 반환
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    {
        if( kMemCmp( pstRootEntry[ i ].vcFileName, pcFileName, iLength ) == 0 )
        {
            kMemCpy( pstEntry, pstRootEntry + i, sizeof( DIRECTORYENTRY ) );
            return i;
        }
    }
    return -1;
}

/**
 *  파일 시스템의 정보를 반환
 */
void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager )
{
    kMemCpy( pstManager, &gs_stFileSystemManager, sizeof( gs_stFileSystemManager ) );
}
