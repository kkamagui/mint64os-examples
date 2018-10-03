/**
 *  file    DynmaicMemory.h
 *  date    2009/04/11
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   동적 메모리 할당과 해제에 관련된 소스 파일
 */
#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"

static DYNAMICMEMORY gs_stDynamicMemory;

/**
 *  동적 메모리 영역 초기화    
 */
void kInitializeDynamicMemory( void )
{
    QWORD qwDynamicMemorySize;
    int i, j;
    BYTE* pbCurrentBitmapPosition;
    int iBlockCountOfLevel, iMetaBlockCount;

    // 동적 메모리 영역으로 사용할 메모리 크기를 이용하여 블록을 관리하는데
    // 필요한 메모리 크기를 최소 블록 단위로 계산
    qwDynamicMemorySize = kCalculateDynamicMemorySize();
    iMetaBlockCount = kCalculateMetaBlockCount( qwDynamicMemorySize );

    // 전체 블록 개수에서 관리에 필요한 메타블록의 개수를 제외한 나머지 영역에 대해서
    // 메타 정보를 구성
    gs_stDynamicMemory.iBlockCountOfSmallestBlock = 
        ( qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE ) - iMetaBlockCount;

    // 최대 몇 개의 블록 리스트로 구성되는지를 계산
    for( i = 0 ; ( gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i ) > 0 ; i++ )
    {
        //DO Nothing
        ;
    }
    gs_stDynamicMemory.iMaxLevelCount = i;
    
    // 할당된 메모리가 속한 블록 리스트의 인덱스를 저장하는 영역을 초기화
    gs_stDynamicMemory.pbAllocatedBlockListIndex = ( BYTE* ) DYNAMICMEMORY_START_ADDRESS;
    for( i = 0 ; i < gs_stDynamicMemory.iBlockCountOfSmallestBlock ; i++ )
    {
        gs_stDynamicMemory.pbAllocatedBlockListIndex[ i ] = 0xFF;
    }
    
    // 비트맵 자료구조의 시작 어드레스 지정
    gs_stDynamicMemory.pstBitmapOfLevel = ( BITMAP* ) ( DYNAMICMEMORY_START_ADDRESS +
        ( sizeof( BYTE ) * gs_stDynamicMemory.iBlockCountOfSmallestBlock ) );
    // 실제 비트맵의 어드레스를 지정
    pbCurrentBitmapPosition = ( ( BYTE* ) gs_stDynamicMemory.pstBitmapOfLevel ) + 
        ( sizeof( BITMAP ) * gs_stDynamicMemory.iMaxLevelCount );
    // 블록 리스트 별로 루프를 돌면서 비트맵을 생성 
    // 초기 상태는 가장 큰 블록과 자투리 블록만 존재하므로 나머지는 비어있는 것으로 설정
    for( j = 0 ; j < gs_stDynamicMemory.iMaxLevelCount ; j++ )
    {
        gs_stDynamicMemory.pstBitmapOfLevel[ j ].pbBitmap = pbCurrentBitmapPosition;
        gs_stDynamicMemory.pstBitmapOfLevel[ j ].qwExistBitCount = 0;
        iBlockCountOfLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;

        // 8개 이상 남았으면 상위 블록으로 모두 결합할 수 있으므로, 모두 비어있는 것으로 설정
        for( i = 0 ; i < iBlockCountOfLevel / 8 ; i++ )
        {
            *pbCurrentBitmapPosition = 0x00;
            pbCurrentBitmapPosition++;
        }

        // 8로 나누어 떨어지지 않는 나머지 블록들에 대한 처리
        if( ( iBlockCountOfLevel % 8 ) != 0 )
        {
            *pbCurrentBitmapPosition = 0x00;
            // 남은 블록이 홀수라면 마지막 한 블록은 결합되어 상위 블록으로 이동하지 못함
            // 따라서 마지막 블록은 현재 블록 리스트에 존재하는 자투리 블록으로 설정
            i = iBlockCountOfLevel % 8;
            if( ( i % 2 ) == 1 )
            {
                *pbCurrentBitmapPosition |= ( DYNAMICMEMORY_EXIST << ( i - 1 ) );
                gs_stDynamicMemory.pstBitmapOfLevel[ j ].qwExistBitCount = 1;
            }
            pbCurrentBitmapPosition++;
        }
    }        
    
    // 블록 풀의 어드레스와 사용된 메모리 크기 설정
    gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS + 
        iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
    gs_stDynamicMemory.qwEndAddress = kCalculateDynamicMemorySize() + 
        DYNAMICMEMORY_START_ADDRESS;
    gs_stDynamicMemory.qwUsedSize = 0;
}

/**
 *  동적 메모리 영역의 크기를 계산  
 */
static QWORD kCalculateDynamicMemorySize( void )
{
    QWORD qwRAMSize;
    
    // 3GB 이상의 메모리에는 비디오 메모리와 프로세서가 사용하는 레지스터가
    // 존재하므로 최대 3GB까지만 사용
    qwRAMSize = ( kGetTotalRAMSize() * 1024 * 1024 );
    if( qwRAMSize > ( QWORD ) 3 * 1024 * 1024 * 1024 )
    {
        qwRAMSize = ( QWORD ) 3 * 1024 * 1024 * 1024;
    }   
    
    return qwRAMSize - DYNAMICMEMORY_START_ADDRESS;
}

/**
 *  동적 메모리 영역을 관리하는데 필요한 정보가 차지하는 공간을 계산
 *      최소 블록 단위로 정렬해서 반환
 */
static int kCalculateMetaBlockCount( QWORD qwDynamicRAMSize )
{
    long lBlockCountOfSmallestBlock;
    DWORD dwSizeOfAllocatedBlockListIndex;
    DWORD dwSizeOfBitmap;
    long i;
    
    // 가장 크기가 작은 블록의 개수를 계산하여 이를 기준으로 비트맵 영역과 
    // 할당된 크기를 저장하는 영역을 계산
    lBlockCountOfSmallestBlock = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;
    // 할당된 블록이 속한 블록 리스트의 인덱스를 저장하는데 필요한 영역을 계산
    dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof( BYTE );
    
    // 비트맵을 저장하는데 필요한 공간 계산
    dwSizeOfBitmap = 0;
    for( i = 0 ; ( lBlockCountOfSmallestBlock >> i ) > 0 ; i++ )
    {
        // 블록 리스트의 비트맵 포인터를 위한 공간
        dwSizeOfBitmap += sizeof( BITMAP );
        // 블록 리스트의 비트맵 크기, 바이트 단위로 올림 처리
        dwSizeOfBitmap += ( ( lBlockCountOfSmallestBlock >> i ) + 7 ) / 8;
    }
    
    // 사용한 메모리 영역의 크기를 최소 블록 크기로 올림해서 반환
    return ( dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + 
        DYNAMICMEMORY_MIN_SIZE - 1 ) / DYNAMICMEMORY_MIN_SIZE;
}

/**
 *  메모리를 할당
 */
void* kAllocateMemory( QWORD qwSize )
{
    QWORD qwAlignedSize;
    QWORD qwRelativeAddress;
    long lOffset;
    int iSizeArrayOffset;
    int iIndexOfBlockList;

    // 메모리 크기를 버디 블록의 크기로 맞춤
    qwAlignedSize = kGetBuddyBlockSize( qwSize );
    if( qwAlignedSize == 0 )
    {
        return NULL;
    }
    
    // 만약 여유 공간이 충분하지 않으면 실패
    if( gs_stDynamicMemory.qwStartAddress + gs_stDynamicMemory.qwUsedSize +
            qwAlignedSize > gs_stDynamicMemory.qwEndAddress )
    {
        return NULL;
    }

    // 버디 블록 할당하고 할당된 블록이 속한 블록 리스트의 인덱스를 반환
    lOffset = kAllocationBuddyBlock( qwAlignedSize );
    if( lOffset == -1 )
    {
        return NULL;
    }
    
    iIndexOfBlockList = kGetBlockListIndexOfMatchSize( qwAlignedSize );
    
    // 블록 크기를 저장하는 영역에 실제로 할당된 버디 블록이 속한 블럭 리스트의 
    // 인덱스를 저장
    // 메모리를 해제할 때 블록 리스트의 인덱스를 사용
    qwRelativeAddress = qwAlignedSize * lOffset;
    iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;
    gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] = 
        ( BYTE ) iIndexOfBlockList;
    gs_stDynamicMemory.qwUsedSize += qwAlignedSize;
    
    return ( void* ) ( qwRelativeAddress + gs_stDynamicMemory.qwStartAddress );
}

/**
 *  가장 가까운 버디 블록의 크기로 정렬된 크기를 반환
 */
static QWORD kGetBuddyBlockSize( QWORD qwSize )
{
    long i;

    for( i = 0 ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        if( qwSize <= ( DYNAMICMEMORY_MIN_SIZE << i ) )
        {
            return ( DYNAMICMEMORY_MIN_SIZE << i );
        }
    }
    return 0;
}

/**
 *  버디 블록 알고리즘으로 메모리 블록을 할당
 *      메모리 크기는 버디 블록의 크기로 요청해야 함
 */
static int kAllocationBuddyBlock( QWORD qwAlignedSize )
{
    int iBlockListIndex, iFreeOffset;
    int i;
    BOOL bPreviousInterruptFlag;

    // 블록 크기를 만족하는 블록 리스트의 인덱스를 검색
    iBlockListIndex = kGetBlockListIndexOfMatchSize( qwAlignedSize );
    if( iBlockListIndex == -1 )
    {
        return -1;
    }
    
    // 동기화 처리
    bPreviousInterruptFlag = kLockForSystemData();
    
    // 만족하는 블록 리스트부터 최상위 블록 리스트까지 검색하여 블록을 선택
    for( i = iBlockListIndex ; i< gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        // 블록 리스트의 비트맵을 확인하여 블록이 존재하는지 확인
        iFreeOffset = kFindFreeBlockInBitmap( i );
        if( iFreeOffset != -1 )
        {
            break;
        }
    }
    
    // 마지막 블록 리스트까지 검색했는데도 없으면 실패
    if( iFreeOffset == -1 )
    {
        kUnlockForSystemData( bPreviousInterruptFlag );
        return -1;
    }

    // 블록을 찾았으니 빈 것으로 표시
    kSetFlagInBitmap( i, iFreeOffset, DYNAMICMEMORY_EMPTY );

    // 상위 블록에서 블록을 찾았다면 상위 블록을 분할
    if( i > iBlockListIndex )
    {
        // 검색된 블록 리스트에서 검색을 시작한 블록 리스트까지 내려가면서 왼쪽 블록은
        // 빈 것으로 표시하고 오른쪽 블록은 존재하는 것으로 표시함
        for( i = i - 1 ; i >= iBlockListIndex ; i-- )
        {
            // 왼쪽 블록은 빈 것으로 표시
            kSetFlagInBitmap( i, iFreeOffset * 2, DYNAMICMEMORY_EMPTY );
            // 오른쪽 블록은 존재하는 것으로 표시
            kSetFlagInBitmap( i, iFreeOffset * 2 + 1, DYNAMICMEMORY_EXIST ); 
            // 왼쪽 블록을 다시 분할
            iFreeOffset = iFreeOffset * 2;
        }
    }    
    kUnlockForSystemData( bPreviousInterruptFlag );
    
    return iFreeOffset;
}

/**
 *  전달된 크기와 가장 근접한 블록 리스트의 인덱스를 반환
 */
static int kGetBlockListIndexOfMatchSize( QWORD qwAlignedSize )
{
    int i;

    for( i = 0 ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        if( qwAlignedSize <= ( DYNAMICMEMORY_MIN_SIZE << i ) )
        {
            return i;
        }
    }
    return -1;
}

/**
 *  블록 리스트의 비트맵를 검색한 후, 블록이 존재하면 블록의 오프셋을 반환
 */
static int kFindFreeBlockInBitmap( int iBlockListIndex )
{
    int i, iMaxCount;
    BYTE* pbBitmap;
    QWORD* pqwBitmap;

    // 비트맵에 데이터가 존재하지 않는다면 실패
    if( gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount == 0 )
    {
        return -1;
    }
    
    // 블록 리스트에 존재하는 총 블록의 수를 구한 후, 그 개수만큼 비트맵을 검색
    iMaxCount = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> iBlockListIndex;
    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    for( i = 0 ; i < iMaxCount ; )
    {
        // QWORD는 8 * 8비트 => 64비트이므로, 64비트를 한꺼번에 검사해서 1인 비트가
        // 있는 지 확인함
        if( ( ( iMaxCount - i ) / 64 ) > 0 )
        {
            pqwBitmap = ( QWORD* ) &( pbBitmap[ i / 8 ] );
            // 만약 8바이트가 모두 0이면 8바이트 모두 제외
            if( *pqwBitmap == 0 )
            {
                i += 64;
                continue;
            }
        }                
        
        if( ( pbBitmap[ i / 8 ] & ( DYNAMICMEMORY_EXIST << ( i % 8 ) ) ) != 0 )
        {
            return i;
        }
        i++;
    }
    return -1;
}

/**
 *  비트맵에 플래그를 설정
 */
static void kSetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag )
{
    BYTE* pbBitmap;

    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    if( bFlag == DYNAMICMEMORY_EXIST )
    {
        // 해당 위치에 데이터가 비어 있다면 개수 증가
        if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) == 0 )
        {
            gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount++;
        }
        pbBitmap[ iOffset / 8 ] |= ( 0x01 << ( iOffset % 8 ) );
    }
    else 
    {
        // 해당 위치에 데이터가 존재한다면 개수 감소
        if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) != 0 )
        {
            gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount--;
        }
        pbBitmap[ iOffset / 8 ] &= ~( 0x01 << ( iOffset % 8 ) );
    }
}

/**
 *  할당 받은 메모리를 해제
 */
BOOL kFreeMemory( void* pvAddress )
{
    QWORD qwRelativeAddress;
    int iSizeArrayOffset;
    QWORD qwBlockSize;
    int iBlockListIndex;
    int iBitmapOffset;

    if( pvAddress == NULL )
    {
        return FALSE;
    }

    // 넘겨 받은 어드레스를 블록 풀을 기준으로 하는 어드레스로 변환하여 할당했던
    // 블록의 크기를 검색
    qwRelativeAddress = ( ( QWORD ) pvAddress ) - gs_stDynamicMemory.qwStartAddress;
    iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;

    // 할당되어있지 않으면 반환 안 함
    if( gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] == 0xFF )
    {
        return FALSE;
    }

    // 할당된 블록이 속한 블록 리스트의 인덱스가 저장된 곳을 초기화하고, 할당된 
    // 블록이 포함된 블록 리스트를 검색
    iBlockListIndex = ( int ) gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ];
    gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] = 0xFF;
    // 버디 블록의 최소 크기를 블록 리스트 인덱스로 시프트하여 할당된 블록의 크기 계산
    qwBlockSize = DYNAMICMEMORY_MIN_SIZE << iBlockListIndex;

    // 블록 리스트 내의 블록 오프셋을 구해서 블록 해제
    iBitmapOffset = qwRelativeAddress / qwBlockSize;
    if( kFreeBuddyBlock( iBlockListIndex, iBitmapOffset ) == TRUE )
    {
        gs_stDynamicMemory.qwUsedSize -= qwBlockSize;
        return TRUE;
    }
    
    return FALSE;
}

/**
 *  블록 리스트의 버디 블록을 해제
 */
static BOOL kFreeBuddyBlock( int iBlockListIndex, int iBlockOffset )
{
    int iBuddyBlockOffset;
    int i;
    BOOL bFlag;
    BOOL bPreviousInterruptFlag;

    // 동기화 처리
    bPreviousInterruptFlag = kLockForSystemData();
    
    // 블록 리스트의 끝까지 인접한 블록을 검사하여 결합할 수 없을 때까지 반복
    for( i = iBlockListIndex ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        // 현재 블록은 존재하는 상태로 설정
        kSetFlagInBitmap( i, iBlockOffset, DYNAMICMEMORY_EXIST );
        
        // 블록의 오프셋이 짝수(왼쪽)이면 홀수(오른쪽)을 검사하고, 홀수이면 짝수의
        // 비트맵을 검사하여 인접한 블록이 존재한다면 결합
        if( ( iBlockOffset % 2 ) == 0 )
        {
            iBuddyBlockOffset = iBlockOffset + 1;
        }
        else
        {
            iBuddyBlockOffset = iBlockOffset - 1;
        }
        bFlag = kGetFlagInBitmap( i, iBuddyBlockOffset );

        // 블록이 비어있으면 종료
        if( bFlag == DYNAMICMEMORY_EMPTY )
        {
            break;
        }
        
        // 여기까지 왔다면 인접한 블록이 존재하므로, 블록을 결합
        // 블록을 모두 빈 것으로 만들고 상위 블록으로 이동
        kSetFlagInBitmap( i, iBuddyBlockOffset, DYNAMICMEMORY_EMPTY );
        kSetFlagInBitmap( i, iBlockOffset, DYNAMICMEMORY_EMPTY );
        
        // 상위 블록 리스트의 블록 오프셋으로 변경하고, 위의 과정을 상위 블록에서
        // 다시 반복
        iBlockOffset = iBlockOffset/ 2;
    }
    
    kUnlockForSystemData( bPreviousInterruptFlag );
    return TRUE;
}

/**
 *  블록 리스트의 해당 위치에 비트맵을 반환
*/
static BYTE kGetFlagInBitmap( int iBlockListIndex, int iOffset )
{
    BYTE* pbBitmap;
    
    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) != 0x00 )
    {
        return DYNAMICMEMORY_EXIST;
    }
    
    return DYNAMICMEMORY_EMPTY;
}

/**
 *  동적 메모리 영역에 대한 정보를 반환
 */
void kGetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, 
        QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, 
        QWORD* pqwUsedMemorySize )
{
    *pqwDynamicMemoryStartAddress = DYNAMICMEMORY_START_ADDRESS;
    *pqwDynamicMemoryTotalSize = kCalculateDynamicMemorySize();    
    *pqwMetaDataSize = kCalculateMetaBlockCount( *pqwDynamicMemoryTotalSize ) * 
        DYNAMICMEMORY_MIN_SIZE;
    *pqwUsedMemorySize = gs_stDynamicMemory.qwUsedSize;
}

/**
 *  동적 메모리 영역을 관리하는 자료구조를 반환
 */
DYNAMICMEMORY* kGetDynamicMemoryManager( void )
{
    return &gs_stDynamicMemory;
}
