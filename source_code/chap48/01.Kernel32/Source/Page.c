/**
 *  file    Types.h
 *  date    2008/12/28
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   페이징에 관련된 각종 정보를 정의한 파일
 */

#include "Page.h"
#include "../../02.Kernel64/Source/Task.h"

// 02.Kernel64/Source/DynamicMemory.h에 있는 동적 메모리 영역의 시작 위치를 옮겨옴
// 동적 메모리의 시작 위치가 수정되면 여기도 같이 수정해야 함
#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + 0x1fffff ) & \
                                          0xffe00000 )

/**
 *  IA-32e 모드 커널을 위한 페이지 테이블 생성
 */
void kInitializePageTables( void )
{
    PML4TENTRY* pstPML4TEntry;
    PDPTENTRY* pstPDPTEntry;
    PDENTRY* pstPDEntry;
    DWORD dwMappingAddress;
    DWORD dwPageEntryFlags;
    int i;

    // PML4 테이블 생성
    // 첫 번째 엔트리 외에 나머지는 모두 0으로 초기화
    pstPML4TEntry = ( PML4TENTRY* ) 0x100000;
    // 페이지 엔트리를 유저 레벨로 설정하여 유저 레벨에서 접근 가능하도록 설정
    kSetPageEntryData( &( pstPML4TEntry[ 0 ] ), 0x00, 0x101000, 
            PAGE_FLAGS_DEFAULT | PAGE_FLAGS_US, 0 );
    for( i = 1 ; i < PAGE_MAXENTRYCOUNT ; i++ )
    {
        kSetPageEntryData( &( pstPML4TEntry[ i ] ), 0, 0, 0, 0 );
    }
    
    // 페이지 디렉터리 포인터 테이블 생성
    // 하나의 PDPT로 512GByte까지 매핑 가능하므로 하나로 충분함
    // 64개의 엔트리를 설정하여 64GByte까지 매핑함
    pstPDPTEntry = ( PDPTENTRY* ) 0x101000;
    for( i = 0 ; i < 64 ; i++ )
    {
        // 페이지 엔트리를 유저 레벨로 설정하여 유저 레벨에서 접근 가능하도록 설정
        kSetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0x102000 + ( i * PAGE_TABLESIZE ), 
                PAGE_FLAGS_DEFAULT | PAGE_FLAGS_US, 0 );
    }
    for( i = 64 ; i < PAGE_MAXENTRYCOUNT ; i++ )
    {
        kSetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0, 0, 0 );
    }
    
    // 페이지 디렉터리 테이블 생성
    // 하나의 페이지 디렉터리가 1GByte까지 매핑 가능 
    // 여유있게 64개의 페이지 디렉터리를 생성하여 총 64GB까지 지원
    pstPDEntry = ( PDENTRY* ) 0x102000;
    dwMappingAddress = 0;
    for( i = 0 ; i < PAGE_MAXENTRYCOUNT * 64 ; i++ )
    {
        // 영역에 따라 페이지 엔트리의 속성을 설정
        // 동적 메모리 영역이 시작하는 영역까지는 커널 영역이므로 페이지의 속성을
        // 관리자(Supervisor)로 설정하고 그 이상은 유저(User)로 설정
        if( i < ( ( DWORD ) DYNAMICMEMORY_START_ADDRESS / PAGE_DEFAULTSIZE ) )
        {
            dwPageEntryFlags = PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS;
        }
        else
        {
            dwPageEntryFlags = PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS | PAGE_FLAGS_US;
        }
        
        // 32비트로는 상위 어드레스를 표현할 수 없으므로, Mbyte 단위로 계산한 다음
        // 최종 결과를 다시 4Kbyte로 나누어 32비트 이상의 어드레스를 계산함
        kSetPageEntryData( &( pstPDEntry[ i ] ), 
                ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, 
                dwPageEntryFlags, 0 );

        dwMappingAddress += PAGE_DEFAULTSIZE;
    }   
}

/**
 *  페이지 엔트리에 기준 주소와 속성 플래그를 설정
 */
void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress,
        DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags )
{
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) | 
        dwUpperFlags;
}
