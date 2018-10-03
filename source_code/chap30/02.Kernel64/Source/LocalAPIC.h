/**
 *  file    LocalAPIC.h
 *  date    2009/06/28
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   로컬 APIC(Local APIC)에 관련된 헤더 파일
 */

#ifndef __LOCALAPIC_H__
#define __LOCALAPIC_H__

#include "Types.h"


////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 로컬 APIC 레지스터 오프셋 관련 매크로
#define APIC_REGISTER_EOI                           0x0000B0
#define APIC_REGISTER_SVR                           0x0000F0
#define APIC_REGISTER_APICID                        0x000020
#define APIC_REGISTER_TASKPRIORITY                  0x000080
#define APIC_REGISTER_TIMER                         0x000320
#define APIC_REGISTER_THERMALSENSOR                 0x000330
#define APIC_REGISTER_PERFORMANCEMONITORINGCOUNTER  0x000340
#define APIC_REGISTER_LINT0                         0x000350
#define APIC_REGISTER_LINT1                         0x000360
#define APIC_REGISTER_ERROR                         0x000370
#define APIC_REGISTER_ICR_LOWER                     0x000300
#define APIC_REGISTER_ICR_UPPER                     0x000310

// 전달 모드(Delivery Mode) 관련 매크로
#define APIC_DELIVERYMODE_FIXED                     0x000000
#define APIC_DELIVERYMODE_LOWESTPRIORITY            0x000100
#define APIC_DELIVERYMODE_SMI                       0x000200
#define APIC_DELIVERYMODE_NMI                       0x000400
#define APIC_DELIVERYMODE_INIT                      0x000500
#define APIC_DELIVERYMODE_STARTUP                   0x000600
#define APIC_DELIVERYMODE_EXTINT                    0x000700

// 목적지 모드(Destination Mode) 관련 매크로
#define APIC_DESTINATIONMODE_PHYSICAL               0x000000
#define APIC_DESTINATIONMODE_LOGICAL                0x000800

// 전달 상태(Delivery Status) 관련 매크로
#define APIC_DELIVERYSTATUS_IDLE                    0x000000
#define APIC_DELIVERYSTATUS_PENDING                 0x001000

// 레벨(Level) 관련 매크로
#define APIC_LEVEL_DEASSERT                         0x000000
#define APIC_LEVEL_ASSERT                           0x004000

// 트리거 모드(Trigger Mode) 관련 매크로
#define APIC_TRIGGERMODE_EDGE                       0x000000
#define APIC_TRIGGERMODE_LEVEL                      0x008000

// 목적지 약어(Destination Shorthand) 관련 매크로
#define APIC_DESTINATIONSHORTHAND_NOSHORTHAND       0x000000
#define APIC_DESTIANTIONSHORTHAND_SELF              0x040000
#define APIC_DESTINATIONSHORTHAND_ALLINCLUDINGSELF  0x080000
#define APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF  0x0C0000

////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
QWORD kGetLocalAPICBaseAddress( void );
void kEnableSoftwareLocalAPIC( void );

#endif /*__LOCALAPIC_H__*/
