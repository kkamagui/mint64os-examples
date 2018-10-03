/**
 *  file    Task.h
 *  date    2009/02/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   태스크를 처리하는 함수에 관련된 파일
 */

#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// SS, RSP, RFLAGS, CS, RIP + ISR에서 저장하는 19개의 레지스터
#define TASK_REGISTERCOUNT     ( 5 + 19 )
#define TASK_REGISTERSIZE       8

// Context 자료구조의 레지스터 오프셋
#define TASK_GSOFFSET           0
#define TASK_FSOFFSET           1
#define TASK_ESOFFSET           2
#define TASK_DSOFFSET           3
#define TASK_R15OFFSET          4
#define TASK_R14OFFSET          5
#define TASK_R13OFFSET          6
#define TASK_R12OFFSET          7
#define TASK_R11OFFSET          8
#define TASK_R10OFFSET          9
#define TASK_R9OFFSET           10
#define TASK_R8OFFSET           11
#define TASK_RSIOFFSET          12
#define TASK_RDIOFFSET          13
#define TASK_RDXOFFSET          14
#define TASK_RCXOFFSET          15
#define TASK_RBXOFFSET          16
#define TASK_RAXOFFSET          17
#define TASK_RBPOFFSET          18
#define TASK_RIPOFFSET          19
#define TASK_CSOFFSET           20
#define TASK_RFLAGSOFFSET       21
#define TASK_RSPOFFSET          22
#define TASK_SSOFFSET           23

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// 콘텍스트에 관련된 자료구조
typedef struct kContextStruct
{
    QWORD vqRegister[ TASK_REGISTERCOUNT ];
} CONTEXT;

// 태스크의 상태를 관리하는 자료구조
typedef struct kTaskControlBlockStruct
{
    // 콘텍스트
    CONTEXT stContext;

    // ID 및 플래그
    QWORD qwID;
    QWORD qwFlags;

    // 스택의 어드레스와 크기
    void* pvStackAddress;
    QWORD qwStackSize;
} TCB;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
void kSetUpTask( TCB* pstTCB, QWORD qwID, QWORD qwFlags, QWORD qwEntryPointAddress,
        void* pvStackAddress, QWORD qwStackSize );

#endif /*__TASK_H__*/
