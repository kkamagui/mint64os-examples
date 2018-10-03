/**
 *  file    PIC.h
 *  date    2009/01/17
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   PIC 컨트롤러에 관련된 헤더 파일
 */

#ifndef __PIC_H__
#define __PIC_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// I/O 포트 정의
#define PIC_MASTER_PORT1    0x20
#define PIC_MASTER_PORT2    0x21
#define PIC_SLAVE_PORT1     0xA0
#define PIC_SLAVE_PORT2     0xA1

// IDT 테이블에서 인터럽트 벡터가 시작되는 위치, 0x20
#define PIC_IRQSTARTVECTOR  0x20


////////////////////////////////////////////////////////////////////////////////
//
//  함수
//
////////////////////////////////////////////////////////////////////////////////
void kInitializePIC( void );
void kMaskPICInterrupt( WORD wIRQBitmask );
void kSendEOIToPIC( int iIRQNumber );

#endif /*__PIC_H__*/
