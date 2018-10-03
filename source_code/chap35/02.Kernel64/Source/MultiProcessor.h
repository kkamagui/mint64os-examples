/**
 *  file    MultiProcessor.h
 *  date    2009/06/29
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   멀티 프로세서 또는 멀티코어 프로세서에 관련된 헤더 파일
 */

#ifndef __MULTIPROCESSOR_H__
#define __MULTIPROCESSOR_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// MultiProcessor 관련 매크로
#define BOOTSTRAPPROCESSOR_FLAGADDRESS      0x7C09
// 지원 가능한 최대 프로세서 또는 코어의 개수
#define MAXPROCESSORCOUNT                   16


////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
BOOL kStartUpApplicationProcessor( void );
BYTE kGetAPICID( void );
static BOOL kWakeUpApplicationProcessor( void );

#endif /*__MULTIPROCESSOR_H__*/
