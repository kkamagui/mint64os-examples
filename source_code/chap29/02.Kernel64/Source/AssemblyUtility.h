/**
 *  file    Utility.h
 *  date    2009/01/07
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   어셈블리어 유틸리티 함수들을 정의한 헤더 파일
 */

#ifndef __ASSEMBLYUTILITY_H__
#define __ASSEMBLYUTILITY_H__

#include "Types.h"
#include "Task.h"

////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
BYTE kInPortByte( WORD wPort );
void kOutPortByte( WORD wPort, BYTE bData );
WORD kInPortWord( WORD wPort );
void kOutPortWord( WORD wPort, WORD wData );
void kLoadGDTR( QWORD qwGDTRAddress );
void kLoadTR( WORD wTSSSegmentOffset );
void kLoadIDTR( QWORD qwIDTRAddress);
void kEnableInterrupt( void );
void kDisableInterrupt( void );
QWORD kReadRFLAGS( void );
QWORD kReadTSC( void );
void kSwitchContext( CONTEXT* pstCurrentContext, CONTEXT* pstNextContext );
void kHlt( void );
BOOL kTestAndSet( volatile BYTE* pbDestination, BYTE bCompare, BYTE bSource );
void kInitializeFPU( void );
void kSaveFPUContext( void* pvFPUContext );
void kLoadFPUContext( void* pvFPUContext );
void kSetTS( void );
void kClearTS( void );

#endif /*__ASSEMBLYUTILITY_H__*/
