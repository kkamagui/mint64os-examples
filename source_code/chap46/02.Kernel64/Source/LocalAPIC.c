/**
 *  file    LocalAPIC.c
 *  date    2009/06/28
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   로컬 APIC(Local APIC)에 관련된 소스 파일
 */
#include "LocalAPIC.h"
#include "MPConfigurationTable.h"

/**
 *  로컬 APIC의 메모리 맵 I/O 어드레스를 반환
 */
QWORD kGetLocalAPICBaseAddress( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
    return pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
}

/**
 *  의사 인터럽트 벡터 레지스터(Spurious Interrupt Vector Register)에 있는 
 *  APIC 소프트웨어 활성/비활성 필드를 1로 설정하여 로컬 APIC를 활성화함
 */
void kEnableSoftwareLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // 의사 인터럽트 벡터 레지스터(Spurious Interrupt Vector Register, 0xFEE000F0)의 
    // APIC 소프트웨어 활성/비활성 필드(비트 8)를 1로 설정해서 로컬 APIC를 활성화
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_SVR ) |= 0x100;
}

/**
 *  로컬 APIC에 EOI(End of Interrupt)를 전송
 */
void kSendEOIToLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // EOI 레지스터(0xFEE000B0)에 0x00을 출력하여 EOI를 전송
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_EOI ) = 0;
}

/*
 *  태스크 우선 순위 레지스터(Task Priority Register) 설정
 */
void kSetTaskPriority( BYTE bPriority )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // 태스크 우선 순위 레지스터(0xFEE00080)에 우선 순위 값을 전송
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TASKPRIORITY ) = bPriority;
}

/**
 *  로컬 벡터 테이블 초기화
 */
void kInitializeLocalVectorTable( void )
{
    QWORD qwLocalAPICBaseAddress;
    DWORD dwTempValue;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();

    // 타이머 인터럽트가 발생하지 않도록 기존 값에 마스크 값을 더해서 
    // LVT 타이머 레지스터(0xFEE00320)에 저장
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TIMER ) |= APIC_INTERRUPT_MASK;
    
    // LINT0 인터럽트가 발생하지 않도록 기존 값에 마스크 값을 더해서
    // LVT LINT0 레지스터(0xFEE00350)에 저장
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT0 ) |= APIC_INTERRUPT_MASK;

    // LINT1 인터럽트는 NMI가 발생하도록 NMI로 설정하여 LVT LINT1 
    // 레지스터(0xFEE00360)에 저장
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT1 ) = APIC_TRIGGERMODE_EDGE | 
        APIC_POLARITY_ACTIVEHIGH | APIC_DELIVERYMODE_NMI;

    // 에러 인터럽트가 발생하지 않도록 기존 값에 마스크 값을 더해서
    // LVT 에러 레지스터(0xFEE00370)에 저장
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_ERROR ) |= APIC_INTERRUPT_MASK;

    // 성능 모니터링 카운터 인터럽트가 발생하지 않도록 기존 값에 마스크 값을 더해서
    // LVT 성능 모니터링 카운터 레지스터(0xFEE00340)에 저장
    *( DWORD* ) ( qwLocalAPICBaseAddress + 
            APIC_REGISTER_PERFORMANCEMONITORINGCOUNTER ) |= APIC_INTERRUPT_MASK;

    // 온도 센서 인터럽트가 발생하지 않도록 기존 값에 마스크 값을 더해서
    // LVT 온도 센서 레지스터(0xFEE00330)에 저장
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_THERMALSENSOR ) |= 
        APIC_INTERRUPT_MASK;
}
