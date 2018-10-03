/**
 *  file    Types.h
 *  date    2008/12/14
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   커널에서 사용하는 각종 타입을 정의한 파일
 */

#ifndef __TYPES_H__
#define __TYPES_H__

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define QWORD   unsigned long
#define BOOL    unsigned char

#define TRUE    1
#define FALSE   0
#define NULL    0


////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )

// 비디오 모드 중 텍스트 모드 화면을 구성하는 자료구조
typedef struct kCharactorStruct
{
    BYTE bCharactor;
    BYTE bAttribute;
} CHARACTER;

#pragma pack( pop )

#endif /*__TYPES_H__*/
