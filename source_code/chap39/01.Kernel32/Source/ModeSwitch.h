/**
 *  file    ModeSwitch.h
 *  date    2009/01/01
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   모드 전환에 관련된 함수들을 정의한 파일
 */
 
#ifndef __MODESWITCH_H__
#define __MODESWITCH_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
void kReadCPUID( DWORD dwEAX, DWORD* pdwEAX, DWORD* pdwEBX, DWORD* pdwECX, 
        DWORD* pdwEDX );
void kSwitchAndExecute64bitKernel( void );

#endif /*__MODESWITCH_H__*/
