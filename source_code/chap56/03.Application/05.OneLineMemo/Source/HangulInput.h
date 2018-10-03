/**
 *  file    Main.c
 *  date    2010/03/30
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   한글 입력에 관련된 헤더 파일
 */

#ifndef __HANGULINPUT_H__
#define __HANGULINPUT_H__

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 영어 대문자를 소문자로 바꿔주는 매크로
#define TOLOWER( x ) ( ( ( 'A' <= ( x ) ) && ( ( x ) <= 'Z' ) ) ? \
    ( ( x ) - 'A' + 'a' ) : ( x ) )

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 한글 입력 테이블을 구성하는 단위
typedef struct HangulInputTableItemStruct
{
    // 한글
    char* pcHangul;

    // 한글에 대응하는 키 입력
    char* pcInput;
} HANGULINPUTITEM;

// 한글 입력 테이블의 인덱스 테이블을 구성하는 단위
typedef struct HangulIndexTableItemStruct
{
    // 한글 낱자를 입력할 때 사용하는 첫 번째 키
    char cStartCharactor;

    // 한글 입력 테이블의 시작 인덱스
    DWORD dwIndex;
} HANGULINDEXITEM;


////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
BOOL IsJaum( char cInput );
BOOL IsMoum( char cInput );
BOOL FindLongestHangulInTable( const char* pcInputBuffer, int iBufferCount,
                        int* piMatchIndex, int* piMatchLength );
BOOL ComposeHangul( char* pcInputBuffer, int* piInputBufferLength,
    char* pcOutputBufferForProcessing, char* pcOutputBufferForComplete );

#endif /* __HANGULINPUT_H__ */
