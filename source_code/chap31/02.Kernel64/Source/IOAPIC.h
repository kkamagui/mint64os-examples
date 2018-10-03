/**
 *  file    IOAPIC.h
 *  date    2009/07/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   I/O APIC에 관련된 헤더 파일
 */

#ifndef __IOAPIC_H__
#define __IOAPIC_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
//  매크로
//
////////////////////////////////////////////////////////////////////////////////
// I/O APIC 레지스터 오프셋 관련 매크로
#define IOAPIC_REGISTER_IOREGISTERSELECTOR              0x00
#define IOAPIC_REGISTER_IOWINDOW                        0x10

// 위의 두 레지스터로 접근할 때 사용하는 레지스터의 인덱스
#define IOAPIC_REGISTERINDEX_IOAPICID                   0x00
#define IOAPIC_REGISTERINDEX_IOAPICVERSION              0x01
#define IOAPIC_REGISTERINDEX_IOAPICARBID                0x02
#define IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE      0x10
#define IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE     0x11

// IO 리다이렉션 테이블의 최대 개수
#define IOAPIC_MAXIOREDIRECTIONTABLECOUNT               24

// 인터럽트 마스크(Interrupt Mask) 관련 매크로
#define IOAPIC_INTERRUPT_MASK                           0x01

// 트리거 모드(Trigger Mode) 관련 매크로
#define IOAPIC_TRIGGERMODE_LEVEL                        0x80
#define IOAPIC_TRIGGERMODE_EDGE                         0x00

// 리모트 IRR(Remote IRR) 관련 매크로
#define IOAPIC_REMOTEIRR_EOI                            0x40
#define IOAPIC_REMOTEIRR_ACCEPT                         0x00

// 인터럽트 입력 핀 극성(Interrupt Input Pin Polarity) 관련 매크로
#define IOAPIC_POLARITY_ACTIVELOW                       0x20
#define IOAPIC_POLARITY_ACTIVEHIGH                      0x00

// 전달 상태(Delivery Status) 관련 매크로
#define IOAPIC_DELIFVERYSTATUS_SENDPENDING              0x10
#define IOAPIC_DELIFVERYSTATUS_IDLE                     0x00

// 목적지 모드(Destination Mode) 관련 매크로
#define IOAPIC_DESTINATIONMODE_LOGICALMODE              0x08
#define IOAPIC_DESTINATIONMODE_PHYSICALMODE             0x00

// 전달 모드(Delivery Mode) 관련 매크로
#define IOAPIC_DELIVERYMODE_FIXED                       0x00
#define IOAPIC_DELIVERYMODE_LOWESTPRIORITY              0x01
#define IOAPIC_DELIVERYMODE_SMI                         0x02
#define IOAPIC_DELIVERYMODE_NMI                         0x04
#define IOAPIC_DELIVERYMODE_INIT                        0x05
#define IOAPIC_DELIVERYMODE_EXTINT                      0x07

// IRQ를 I/O APIC의 인터럽트 입력 핀(INTIN)으로 대응시키는 테이블의 최대 크기
#define IOAPIC_MAXIRQTOINTINMAPCOUNT                    16

////////////////////////////////////////////////////////////////////////////////
//
//  구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// IO 리다이렉션 테이블의 자료구조
typedef struct kIORedirectionTableStruct
{
    // 인터럽트 벡터
    BYTE bVector;  
    
    // 트리거 모드, 리모트 IRR, 인터럽트 입력 핀 극성, 전달 상태, 목적지 모드, 
    // 전달 모드를 담당하는 필드 
    BYTE bFlagsAndDeliveryMode;
    
    // 인터럽트 마스크
    BYTE bInterruptMask;
    
    // 예약된 영역
    BYTE vbReserved[ 4 ];
    
    // 인터럽트를 전달할 APIC ID
    BYTE bDestination;
} IOREDIRECTIONTABLE;

#pragma pack( pop )

// I/O APIC를 관리하는 자료구조
typedef struct kIOAPICManagerStruct
{
    // ISA 버스가 연결된 I/O APIC의 메모리 맵 어드레스
    QWORD qwIOAPICBaseAddressOfISA;
    
    // IRQ와 I/O APIC의 인터럽트 입력 핀(INTIN)간의 연결 관계를 저장하는 테이블
    BYTE vbIRQToINTINMap[ IOAPIC_MAXIRQTOINTINMAPCOUNT ];    
} IOAPICMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
QWORD kGetIOAPICBaseAddressOfISA( void );
void kSetIOAPICRedirectionEntry( IOREDIRECTIONTABLE* pstEntry, BYTE bAPICID,
        BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector );
void kReadIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry );
void kWriteIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry );
void kMaskAllInterruptInIOAPIC( void );
void kInitializeIORedirectionTable( void );
void kPrintIRQToINTINMap( void );

#endif /*__IOAPIC_H__*/
