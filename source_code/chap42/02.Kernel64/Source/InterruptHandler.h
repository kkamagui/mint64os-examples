/**
 *  file    InterruptHandler.h
 *  date    2009/01/24
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   인터럽트 및 예외 핸들러에 관련된 헤더 파일
 */

#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"
#include "MultiProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//
//  매크로
//
////////////////////////////////////////////////////////////////////////////////
// 인터럽트 벡터의 최대 개수, ISA 버스의 인터럽트만 처리하므로 16
#define INTERRUPT_MAXVECTORCOUNT            16
// 인터럽트 부하 분산을 수행하는 시점, 인터럽트 처리 횟수가 10의 배수가 되는 시점
#define INTERRUPT_LOADBALANCINGDIVIDOR      10

////////////////////////////////////////////////////////////////////////////////
//
//  구조체
//
////////////////////////////////////////////////////////////////////////////////
// 인터럽트에 관련된 정보를 저장하는 자료구조
typedef struct kInterruptManagerStruct
{
    // 코어 별 인터럽트 처리 횟수, 최대 코어 개수 X 최대 인터럽트 벡터 개수로 정의된 2차원 배열
    QWORD vvqwCoreInterruptCount[ MAXPROCESSORCOUNT ][ INTERRUPT_MAXVECTORCOUNT ];
    
    // 부하 분산 기능 사용 여부
    BOOL bUseLoadBalancing;
    
    // 대칭 I/O 모드(Symmetric I/O Mode) 사용 여부
    BOOL bSymmetricIOMode;
} INTERRUPTMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
void kSetSymmetricIOMode( BOOL bSymmetricIOMode );
void kSetInterruptLoadBalancing( BOOL bUseLoadBalancing );
void kIncreaseInterruptCount( int iIRQ );
void kSendEOI( int iIRQ );
INTERRUPTMANAGER* kGetInterruptManager( void );
void kProcessLoadBalancing( int iIRQ );

void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void kCommonInterruptHandler( int iVectorNumber );
void kKeyboardHandler( int iVectorNumber );
void kTimerHandler( int iVectorNumber );
void kDeviceNotAvailableHandler( int iVectorNumber );
void kHDDHandler( int iVectorNumber );
void kMouseHandler( int iVectorNumber );

#endif /*__INTERRUPTHANDLER_H__*/
