/**
 *  file    WindowManager.h
 *  date    2009/10/04
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   윈도우 매니저에 관련된 함수를 정의한 헤더 파일
 */

#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
void kStartWindowManager( void );
BOOL kProcessMouseData( void );
BOOL kProcessKeyData( void );
BOOL kProcessEventQueueData( void );

#endif /*__WINDOWMANAGER_H__*/
