/**
 *  file    SystemCall.h
 *  date    2009/12/08
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   시스템 콜에 관련된 함수를 정의한 헤더 파일
 */

#ifndef __SYSTEMCALL_H__
#define __SYSTEMCALL_H__

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 파라미터로 전달할 수 있는 최대 개수
#define SYSTEMCALL_MAXPARAMETERCOUNT    20

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1 바이트로 정렬
#pragma pack( push, 1 )

// 시스템 콜을 호출할 때 전달하는 파라미터를 관리하는 자료구조
typedef struct kSystemCallParameterTableStruct
{
    QWORD vqwValue[ SYSTEMCALL_MAXPARAMETERCOUNT ];
} PARAMETERTABLE;

#pragma pack( pop )

// 파라미터 자료구조에서 N 번째를 가리키는 매크로
#define PARAM( x )   ( pstParameter->vqwValue[ ( x ) ] )

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
void kSystemCallEntryPoint( QWORD qwServiceNumber, PARAMETERTABLE* pstParameter );
QWORD kProcessSystemCall( QWORD qwServiceNumber, PARAMETERTABLE* pstParameter );

void kSystemCallTestTask( void );

#endif /*__SYSTEMCALL_H__*/
