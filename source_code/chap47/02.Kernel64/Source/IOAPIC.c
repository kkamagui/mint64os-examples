/**
 *  file    IOAPIC.c
 *  date    2009/07/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   I/O APIC에 관련된 소스 파일
 */

#include "IOAPIC.h"
#include "MPConfigurationTable.h"
#include "PIC.h"

// I/O APIC를 관리하는 자료구조
static IOAPICMANAGER gs_stIOAPICManager;

/**
 *  ISA 버스가 연결된 I/O APIC의 기준 어드레스를 반환
 */
QWORD kGetIOAPICBaseAddressOfISA( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    IOAPICENTRY* pstIOAPICEntry;
    
    // I/O APIC의 어드레스가 저장되어 있지 않으면 엔트리를 찾아서 저장
    if( gs_stIOAPICManager.qwIOAPICBaseAddressOfISA == NULL )
    {
        pstIOAPICEntry = kFindIOAPICEntryForISA();
        if( pstIOAPICEntry != NULL )
        {
            gs_stIOAPICManager.qwIOAPICBaseAddressOfISA = 
                pstIOAPICEntry->dwMemoryMapAddress & 0xFFFFFFFF;
        }
    }

    // I/O APIC의 기준 어드레스를 찾아서 저장한 다음 반환
    return gs_stIOAPICManager.qwIOAPICBaseAddressOfISA;
}

/**
 *  I/O 리다이렉션 테이블 자료구조에 값을 설정
 */
void kSetIOAPICRedirectionEntry( IOREDIRECTIONTABLE* pstEntry, BYTE bAPICID,
        BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector )
{
    kMemSet( pstEntry, 0, sizeof( IOREDIRECTIONTABLE ) );
    
    pstEntry->bDestination = bAPICID;
    pstEntry->bFlagsAndDeliveryMode = bFlagsAndDeliveryMode;
    pstEntry->bInterruptMask = bInterruptMask;
    pstEntry->bVector = bVector;
}

/**
 *  인터럽트 입력 핀(INTIN)에 해당하는 I/O 리다이렉션 테이블에서 값을 읽음
 */
void kReadIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry )
{
    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;
    
    // ISA 버스가 연결된 I/O APIC의 메모리 맵 I/O 어드레스
    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();
    
    // I/O 리다이렉션 테이블은 8바이트이므로, 8바이트 정수로 변환해서 처리
    pqwData = ( QWORD* ) pstEntry;
    
    //--------------------------------------------------------------------------
    // I/O 리다이렉션 테이블의 상위 4바이트를 읽음
    // I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로, INTIN에
    // 2를 곱하여 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //--------------------------------------------------------------------------
    // I/O 레지스터 선택 레지스터(0xFEC00000)에 상위 I/O 리다이렉션 테이블 레지스터의
    // 인덱스를 전송
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = 
        IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;
    // I/O 윈도우 레지스터(0xFEC00010)에서 상위 I/O 리다이렉션 테이블 레지스터의
    // 값을 읽음
    *pqwData = *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
    *pqwData = *pqwData << 32;
    
    //--------------------------------------------------------------------------
    // I/O 리다이렉션 테이블의 하위 4바이트를 읽음
    // I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로, INTIN에
    // 2를 곱하여 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //--------------------------------------------------------------------------
    // I/O 레지스터 선택 레지스터(0xFEC00000)에 하위 I/O 리다이렉션 테이블 레지스터의
    // 인덱스를 전송
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) =
        IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2 ;
    // I/O 윈도우 레지스터(0xFEC00010)에서 하위 I/O 리다이렉션 테이블 레지스터의
    // 값을 읽음
    *pqwData |= *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
}

/**
 *  인터럽트 입력 핀(INTIN)에 해당하는 I/O 리다이렉션 테이블에 값을 씀
 */
void kWriteIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry )
{
    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;
    
    // ISA 버스가 연결된 I/O APIC의 메모리 맵 I/O 어드레스
    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();

    // I/O 리다이렉션 테이블은 8바이트이므로, 8바이트 정수로 변환해서 처리
    pqwData = ( QWORD* ) pstEntry;
    
    //--------------------------------------------------------------------------
    // I/O 리다이렉션 테이블에 상위 4바이트를 씀
    // I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로, INTIN에
    // 2를 곱하여 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //--------------------------------------------------------------------------
    // I/O 레지스터 선택 레지스터(0xFEC00000)에 상위 I/O 리다이렉션 테이블 레지스터의
    // 인덱스를 전송
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = 
        IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;
    // I/O 윈도우 레지스터(0xFEC00010)에 상위 I/O 리다이렉션 테이블 레지스터의
    // 값을 씀
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData >> 32;
    
    //--------------------------------------------------------------------------
    // I/O 리다이렉션 테이블에 하위 4바이트를 씀
    // I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로, INTIN에
    // 2를 곱하여 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //--------------------------------------------------------------------------
    // I/O 레지스터 선택 레지스터(0xFEC00000)에 하위 I/O 리다이렉션 테이블 레지스터의
    // 인덱스를 전송
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) =
        IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2 ;
    // I/O 윈도우 레지스터(0xFEC00010)에 하위 I/O 리다이렉션 테이블 레지스터의
    // 값을 씀
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData;
}

/**
 *  I/O APIC에 연결된 모든 인터럽트 핀을 마스크하여 인터럽트가 전달되지 않도록 함
 */
void kMaskAllInterruptInIOAPIC( void )
{
    IOREDIRECTIONTABLE stEntry;
    int i;
    
    // 모든 인터럽트를 비활성화
    for( i = 0 ; i < IOAPIC_MAXIOREDIRECTIONTABLECOUNT ; i++ )
    {
        // I/O 리다이렉션 테이블을 읽어서 인터럽트 마스크 필드(비트 0)를 1로 
        // 설정하여 다시 저장
        kReadIOAPICRedirectionTable( i, &stEntry );
        stEntry.bInterruptMask = IOAPIC_INTERRUPT_MASK;
        kWriteIOAPICRedirectionTable( i, &stEntry );
    }
}

/**
 *  I/O APIC의 I/O 리다이렉션 테이블을 초기화
 */
void kInitializeIORedirectionTable( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    IOREDIRECTIONTABLE stIORedirectionEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BYTE bDestination;
    int i;

    //==========================================================================
    // I/O APIC를 관리하는 자료구조를 초기화
    //==========================================================================
    kMemSet( &gs_stIOAPICManager, 0, sizeof( gs_stIOAPICManager ) );
    
    // I/O APIC의 메모리 맵 I/O 어드레스 저장, 아래 함수에서 내부적으로 처리함
    kGetIOAPICBaseAddressOfISA();
    
    // IRQ를 I/O APIC의 INTIN 핀과 연결한 테이블(IRQ->INTIN 매핑 테이블)을 초기화
    for( i = 0 ; i < IOAPIC_MAXIRQTOINTINMAPCOUNT ; i++ )
    {
        gs_stIOAPICManager.vbIRQToINTINMap[ i ] = 0xFF;
    }
    
    //==========================================================================
    // I/O APIC를 마스크하여 인터럽트가 발생하지 않도록 하고 I/O 리다이렉션 테이블 초기화
    //==========================================================================
    // 먼저 I/O APIC의 인터럽트를 마스크하여 인터럽트가 발생하지 않도록 함
    kMaskAllInterruptInIOAPIC();
    
    // IO 인터럽트 지정 엔트리 중에서 ISA 버스와 관련된 인터럽트만 추려서 I/O 리다이렉션
    // 테이블에 설정
    // MP 설정 테이블 헤더의 시작 어드레스와 엔트리의 시작 어드레스를 저장
    pstMPManager = kGetMPConfigurationManager();
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwEntryAddress = pstMPManager->qwBaseEntryStartAddress;
    
    // 모든 엔트리를 확인하여 ISA 버스와 관련된 I/O 인터럽트 지정 엔트리를 검색
    for( i = 0 ; i < pstMPHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        switch( bEntryType )
        {
            // IO 인터럽트 지정 엔트리이면, ISA 버스인지 확인하여 I/O 리다이렉션
            // 테이블에 설정하고 IRQ->INITIN 매핑 테이블을 구성
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwEntryAddress;

            // 인터럽트 타입이 인터럽트(INT)인 것만 처리
            if( ( pstIOAssignmentEntry->bSourceBUSID == pstMPManager->bISABusID ) &&
                ( pstIOAssignmentEntry->bInterruptType == MP_INTERRUPTTYPE_INT ) )                        
            {
                // 목적지 필드는 IRQ 0를 제외하고 0x00으로 설정하여 Bootstrap Processor만 전달
                // IRQ 0는 스케줄러에 사용해야 하므로 0xFF로 설정하여 모든 코어로 전달
                if( pstIOAssignmentEntry->bSourceBUSIRQ == 0 )
                {
                    bDestination = 0xFF;
                }
                else
                {
                    bDestination = 0x00;
                }
                
                // ISA 버스는 엣지 트리거(Edge Trigger)와 1일 때 활성화(Active High)를
                // 사용
                // 목적지 모드는 물리 모드, 전달 모드는 고정(Fixed)으로 할당
                // 인터럽트 벡터는 PIC 컨트롤러의 벡터와 같이 0x20 + IRQ로 설정
                kSetIOAPICRedirectionEntry( &stIORedirectionEntry, bDestination, 
                    0x00, IOAPIC_TRIGGERMODE_EDGE | IOAPIC_POLARITY_ACTIVEHIGH |
                    IOAPIC_DESTINATIONMODE_PHYSICALMODE | IOAPIC_DELIVERYMODE_FIXED, 
                    PIC_IRQSTARTVECTOR + pstIOAssignmentEntry->bSourceBUSIRQ );
                
                // ISA 버스에서 전달된 IRQ는 I/O APIC의 INTIN 핀에 있으므로, INTIN 값을
                // 이용하여 처리
                kWriteIOAPICRedirectionTable( pstIOAssignmentEntry->bDestinationIOAPICINTIN, 
                        &stIORedirectionEntry );
                
                // IRQ와 인터럽트 입력 핀(INTIN)의 관계를 저장(IRQ->INTIN 매핑 테이블 구성)
                gs_stIOAPICManager.vbIRQToINTINMap[ pstIOAssignmentEntry->bSourceBUSIRQ ] =
                    pstIOAssignmentEntry->bDestinationIOAPICINTIN;                
            }                    
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        
            // 프로세스 엔트리는 무시
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
            // 버스 엔트리, I/O APIC 엔트리, 로컬 인터럽트 지정 엔트리는 무시
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
        }
    }  
}

/**
 *  IRQ와 I/O APIC의 인터럽트 핀(INT IN)간의 매핑 관계를 출력
 */
void kPrintIRQToINTINMap( void )
{
    int i;
    
    kPrintf( "=========== IRQ To I/O APIC INT IN Mapping Table ===========\n" );
    
    for( i = 0 ; i < IOAPIC_MAXIRQTOINTINMAPCOUNT ; i++ )
    {
        kPrintf( "IRQ[%d] -> INTIN [%d]\n", i, gs_stIOAPICManager.vbIRQToINTINMap[ i ] );
    }
}

/**
 *  IRQ를 로컬 APIC ID로 전달하도록 변경
 */
void kRoutingIRQToAPICID( int iIRQ, BYTE bAPICID )
{
    int i;
    IOREDIRECTIONTABLE stEntry;

    // 범위 검사
    if( iIRQ > IOAPIC_MAXIRQTOINTINMAPCOUNT )
    {
        return ;
    }
    
    // 설정된 I/O 리다이렉션 테이블을 읽어서 목적지(Destination) 필드만 수정
    kReadIOAPICRedirectionTable( gs_stIOAPICManager.vbIRQToINTINMap[ iIRQ ],
            &stEntry );
    stEntry.bDestination = bAPICID;
    kWriteIOAPICRedirectionTable( gs_stIOAPICManager.vbIRQToINTINMap[ iIRQ ],
            &stEntry );
}

