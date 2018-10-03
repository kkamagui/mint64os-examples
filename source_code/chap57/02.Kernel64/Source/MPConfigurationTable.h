/**
 *  file    MPConfigurationTable.h
 *  date    2009/06/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   MP 설정 테이블(MP Configuration Table)에 관련된 헤더 파일
 */

#ifndef __MPCONFIGURATIONTABLE__
#define __MPCONFIGURATIONTABLE__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// MP 플로팅 포인터의 특성 바이트(Feature Byte)
#define MP_FLOATINGPOINTER_FEATUREBYTE1_USEMPTABLE  0x00
#define MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE     0x80

// 엔트리 타입(Entry Type)
#define MP_ENTRYTYPE_PROCESSOR                  0
#define MP_ENTRYTYPE_BUS                        1
#define MP_ENTRYTYPE_IOAPIC                     2
#define MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT      3
#define MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT   4

// 프로세서 CPU 플래그
#define MP_PROCESSOR_CPUFLAGS_ENABLE            0x01
#define MP_PROCESSOR_CPUFLAGS_BSP               0x02

// 버스 타입 스트링(Bus Type String)
#define MP_BUS_TYPESTRING_ISA                   "ISA"
#define MP_BUS_TYPESTRING_PCI                   "PCI"
#define MP_BUS_TYPESTRING_PCMCIA                "PCMCIA"
#define MP_BUS_TYPESTRING_VESALOCALBUS          "VL"

// 인터럽트 타입(Interrupt Type)
#define MP_INTERRUPTTYPE_INT                    0
#define MP_INTERRUPTTYPE_NMI                    1
#define MP_INTERRUPTTYPE_SMI                    2
#define MP_INTERRUPTTYPE_EXTINT                 3

// 인터럽트 플래그(Interrupt Flags)
#define MP_INTERRUPT_FLAGS_CONFORMPOLARITY      0x00
#define MP_INTERRUPT_FLAGS_ACTIVEHIGH           0x01
#define MP_INTERRUPT_FLAGS_ACTIVELOW            0x03
#define MP_INTERRUPT_FLAGS_CONFORMTRIGGER       0x00
#define MP_INTERRUPT_FLAGS_EDGETRIGGERED        0x04
#define MP_INTERRUPT_FLAGS_LEVELTRIGGERED       0x0C

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// MP 플로팅 포인터 자료구조(MP Floating Pointer Data Structure)
typedef struct kMPFloatingPointerStruct
{
    // 시그너처, _MP_
    char vcSignature[ 4 ]; 
    // MP 설정 테이블이 위치하는 어드레스
    DWORD dwMPConfigurationTableAddress;
    // MP 플로팅 포인터 자료구조의 길이, 16 바이트
    BYTE bLength;
    // MultiProcessor Specification의 버전
    BYTE bRevision;
    // 체크섬
    BYTE bCheckSum;
    // MP 특성 바이트 1~5
    BYTE vbMPFeatureByte[ 5 ];
} MPFLOATINGPOINTER;

// MP 설정 테이블 헤더(MP Configuration Table Header) 자료구조
typedef struct kMPConfigurationTableHeaderStruct
{
    // 시그너처, PCMP
    char vcSignature[ 4 ];
    // 기본 테이블 길이
    WORD wBaseTableLength;
    // MultiProcessor Specification의 버전
    BYTE bRevision;
    // 체크섬
    BYTE bCheckSum;
    // OEM ID 문자열
    char vcOEMIDString[ 8 ];
    // PRODUCT ID 문자열
    char vcProductIDString[ 12 ];
    // OEM이 정의한 테이블의 어드레스
    DWORD dwOEMTablePointerAddress;
    // OEM이 정의한 테이블의 크기
    WORD wOEMTableSize;
    // 기본 MP 설정 테이블 엔트리의 개수
    WORD wEntryCount;
    // 로컬 APIC의 메모리 맵 I/O 어드레스
    DWORD dwMemoryMapIOAddressOfLocalAPIC;
    // 확장 테이블의 길이
    WORD wExtendedTableLength;
    // 확장 테이블의 체크섬
    BYTE bExtendedTableChecksum;
    // 예약됨
    BYTE bReserved;
} MPCONFIGURATIONTABLEHEADER;

// 프로세서 엔트리 자료구조(Processor Entry)
typedef struct kProcessorEntryStruct
{
    // 엔트리 타입 코드, 0
    BYTE bEntryType;
    // 프로세서에 포함된 로컬 APIC의 ID
    BYTE bLocalAPICID;
    // 로컬 APIC의 버전
    BYTE bLocalAPICVersion;
    // CPU 플래그
    BYTE bCPUFlags;
    // CPU 시그너처
    BYTE vbCPUSignature[ 4 ];
    // 특성 플래그
    DWORD dwFeatureFlags;
    // 예약됨
    DWORD vdwReserved[ 2 ];
} PROCESSORENTRY;

// 버스 엔트리 자료구조(Bus Entry)
typedef struct kBusEntryStruct
{
    // 엔트리 타입 코드, 1
    BYTE bEntryType;
    // 버스 ID
    BYTE bBusID;
    // 버스 타입 문자열
    char vcBusTypeString[ 6 ];
} BUSENTRY;

// I/O APIC 엔트리 자료구조(I/O APIC Entry)
typedef struct kIOAPICEntryStruct
{
    // 엔트리 타입 코드, 2
    BYTE bEntryType;
    // I/O APIC ID
    BYTE bIOAPICID;
    // I/O APIC 버전
    BYTE bIOAPICVersion;
    // I/O APIC 플래그
    BYTE bIOAPICFlags;
    // 메모리 맵 I/O 어드레스
    DWORD dwMemoryMapAddress;
} IOAPICENTRY;

// I/O 인터럽트 지정 엔트리 자료구조(I/O Interrupt Assignment Entry)
typedef struct kIOInterruptAssignmentEntryStruct
{
    // 엔트리 타입 코드, 3
    BYTE bEntryType;
    // 인터럽트 타입
    BYTE bInterruptType;
    // I/O 인터럽트 플래그
    WORD wInterruptFlags;
    // 발생한 버스 ID
    BYTE bSourceBUSID;
    // 발생한 버스 IRQ
    BYTE bSourceBUSIRQ;
    // 전달할 I/O APIC ID
    BYTE bDestinationIOAPICID;
    // 전달할 I/O APIC INTIN
    BYTE bDestinationIOAPICINTIN;
} IOINTERRUPTASSIGNMENTENTRY;

// 로컬 인터럽트 지정 엔트리 자료구조(Local Interrupt Assignment Entry)
typedef struct kLocalInterruptEntryStruct
{
    // 엔트리 타입 코드, 4
    BYTE bEntryType;
    // 인터럽트 타입
    BYTE bInterruptType;
    // 로컬 인터럽트 플래그
    WORD wInterruptFlags;
    // 발생한 버스 ID
    BYTE bSourceBUSID;
    // 발생한 버스 IRQ
    BYTE bSourceBUSIRQ;
    // 전달할 로컬 APIC ID
    BYTE bDestinationLocalAPICID;
    // 전달할 로컬 APIC INTIN
    BYTE bDestinationLocalAPICLINTIN;
} LOCALINTERRUPTASSIGNMENTENTRY;

#pragma pack( pop)

// MP 설정 테이블을 관리하는 자료구조
typedef struct kMPConfigurationManagerStruct
{
    // MP 플로팅 테이블
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    // MP 설정 테이블 헤더
    MPCONFIGURATIONTABLEHEADER* pstMPConfigurationTableHeader;
    // 기본 MP 설정 테이블 엔트리의 시작 어드레스
    QWORD qwBaseEntryStartAddress;
    // 프로세서 또는 코어의 수
    int iProcessorCount;
    // PIC 모드 지원 여부
    BOOL bUsePICMode;
    // ISA 버스의 ID
    BYTE bISABusID;
} MPCONFIGRUATIONMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
BOOL kFindMPFloatingPointerAddress( QWORD* pstAddress );
BOOL kAnalysisMPConfigurationTable( void );
MPCONFIGRUATIONMANAGER* kGetMPConfigurationManager( void );
void kPrintMPConfigurationTable( void );
int kGetProcessorCount( void );
IOAPICENTRY* kFindIOAPICEntryForISA( void );

#endif /*__MPCONFIGURATIONTABLE__*/
