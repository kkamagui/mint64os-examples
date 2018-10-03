/**
 *  file    MultiProcessor.c
 *  date    2009/06/29
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   멀티 프로세서 또는 멀티코어 프로세서에 관련된 소스 파일
 */

#include "MultiProcessor.h"
#include "MPConfigurationTable.h"
#include "AssemblyUtility.h"
#include "LocalAPIC.h"
#include "PIT.h"

// 활성화된 Application Processor의 개수
volatile int g_iWakeUpApplicationProcessorCount = 0;
// APIC ID 레지스터의 어드레스
volatile QWORD g_qwAPICIDAddress = 0;

/**
 *  로컬 APIC를 활성화하고 AP(Application Processor)를 활성화
 */
BOOL kStartUpApplicationProcessor( void )
{
    // MP 설정 테이블 분석
    if( kAnalysisMPConfigurationTable() == FALSE )
    {
        return FALSE;
    }
    
    // 모든 프로세서에서 로컬 APIC를 사용하도록 활성화
    kEnableGlobalLocalAPIC();
    
    // BSP(Bootstrap Processor)의 로컬 APIC를 활성화
    kEnableSoftwareLocalAPIC();    
    
    // AP를 깨움
    if( kWakeUpApplicationProcessor() == FALSE )
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 *  AP(Application Processor)를 활성화
 */
static BOOL kWakeUpApplicationProcessor( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;
    BOOL bInterruptFlag;
    int i;

    // 인터럽트를 불가로 설정
    bInterruptFlag = kSetInterruptFlag( FALSE );

    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    pstMPManager = kGetMPConfigurationManager(); 
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;

    // APIC ID 레지스터의 어드레스(0xFEE00020)를 저장하여, Application Processor가
    // 자신의 APIC ID를 읽을 수 있게 함
    g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;
    
    //--------------------------------------------------------------------------
    // 하위 인터럽트 커맨드 레지스터(Lower Interrupt Command Register, 0xFEE00300)에
    // 초기화(INIT) IPI와 시작(Start Up) IPI를 전송하여 AP를 깨움
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    // 초기화(INIT) IPI 전송
    //--------------------------------------------------------------------------
    // 하위 인터럽트 커맨드 레지스터(0xFEE00300)을 사용해서 BSP(Bootstrap Processor)를
    // 제외한 나머지 코어에 INIT IPI를 전송
    // AP(Application Processor)는 보호 모드 커널(0x10000)에서 시작
    // All Excluding Self, Edge Trigger, Assert, Physical Destination, INIT
    *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = 
        APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |
        APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | APIC_DELIVERYMODE_INIT;
    
    // PIT를 직접 제어하여 10ms 동안 대기
    kWaitUsingDirectPIT( MSTOCOUNT( 10 ) );
        
    // 하위 인터럽트 커맨드 레지스터(0xFEE00300)에서 전달 상태 비트(비트 12)를 
    // 확인하여 성공 여부 판별
    if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) &
            APIC_DELIVERYSTATUS_PENDING )
    {
        // 타이머 인터럽트가 1초에 1000번 발생하도록 재설정
        kInitializePIT( MSTOCOUNT( 1 ), TRUE );
        
        // 인터럽트 플래그를 복원
        kSetInterruptFlag( bInterruptFlag );
        return FALSE;
    }
    
    //--------------------------------------------------------------------------
    // 시작(Start Up) IPI 전송, 2회 반복 전송함
    //--------------------------------------------------------------------------
    for( i = 0 ; i < 2 ; i++ )
    {
        // 하위 인터럽트 커맨드 레지스터(0xFEE00300)을 사용해서 BSP를 제외한 
        // 나머지 코어에 SIPI 메시지를 전송
        // 보호 모드 커널이 시작하는 0x10000에서 실행시키려고 0x10(0x10000 / 4Kbyte)를
        // 인터럽트 벡터로 설정
        // All Excluding Self, Edge Trigger, Assert, Physical Destination, Start Up
        *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = 
            APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |
            APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | 
            APIC_DELIVERYMODE_STARTUP | 0x10;
        
        // PIT를 직접 제어하여 200us 동안 대기
        kWaitUsingDirectPIT( USTOCOUNT( 200 ) );
            
        // 하위 인터럽트 커맨드 레지스터(0xFEE00300)에서 전달 상태 비트(비트 12)를 
        // 확인하여 성공 여부 판별
        if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) &
                APIC_DELIVERYSTATUS_PENDING )
        {
            // 타이머 인터럽트가 1초에 1000번 발생하도록 재설정
            kInitializePIT( MSTOCOUNT( 1 ), TRUE );
            
            // 인터럽트 플래그를 복원
            kSetInterruptFlag( bInterruptFlag );
            return FALSE;
        }
    }
    
    // 타이머 인터럽트가 1초에 1000번 발생하도록 재설정
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );
    
    // 인터럽트 플래그를 복원
    kSetInterruptFlag( bInterruptFlag );
    
    // Application Processor가 모두 깨어날 때까지 대기
    while( g_iWakeUpApplicationProcessorCount < 
            ( pstMPManager->iProcessorCount - 1 ) )
    {
        kSleep( 50 );
    }    
    return TRUE;
}

/**
 *  APIC ID 레지스터에서 APIC ID를 반환
 */
BYTE kGetAPICID( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;

    // APIC ID 어드레스의 값이 설정되지 않았으면, MP 설정 테이블에서 값을 읽어서 설정
    if( g_qwAPICIDAddress == 0 )
    {
        // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
        pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
        if( pstMPHeader == NULL )
        {
            return 0;
        }
        
        // APIC ID 레지스터의 어드레스((0xFEE00020)를 저장하여, 자신의 APIC ID를
        // 읽을 수 있게 함
        qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
        g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;
    }
    
    // APIC ID 레지스터의 Bit 24~31의 값을 반환    
    return *( ( DWORD* ) g_qwAPICIDAddress ) >> 24;
}
