/**
 *  file    Window.h
 *  date    2009/09/28
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI 시스템에 관련된 함수를 정의한 헤더 파일
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Types.h"
#include "Synchronization.h"
#include "2DGraphics.h"
#include "List.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 윈도우를 생성할 수 있는 최대 개수
#define WINDOW_MAXCOUNT             2048
// 윈도우 ID로 윈도우 풀 내의 오프셋을 계산하는 매크로
// 하위 32비트가 풀 내의 오프셋을 나타냄
#define GETWINDOWOFFSET( x )        ( ( x ) & 0xFFFFFFFF )
// 윈도우 제목의 최대 길이
#define WINDOW_TITLEMAXLENGTH       40
// 유효하지 않은 윈도우 ID
#define WINDOW_INVALIDID            0xFFFFFFFFFFFFFFFF

// 윈도우의 속성
// 윈도우를 화면에 나타냄
#define WINDOW_FLAGS_SHOW           0x00000001
// 윈도우 테두리 그림
#define WINDOW_FLAGS_DRAWFRAME      0x00000002
// 윈도우 제목 표시줄 그림
#define WINDOW_FLAGS_DRAWTITLE      0x00000004
// 윈도우 기본 속성, 제목 표시줄과 프레임을 모두 그리고 화면에 윈도우를 보이게 설정
#define WINDOW_FLAGS_DEFAULT        ( WINDOW_FLAGS_SHOW | WINDOW_FLAGS_DRAWFRAME | \
                                      WINDOW_FLAGS_DRAWTITLE )

// 제목 표시줄의 높이
#define WINDOW_TITLEBAR_HEIGHT      21
// 윈도우의 닫기 버튼의 크기
#define WINDOW_XBUTTON_SIZE         19

// 윈도우의 색깔
#define WINDOW_COLOR_FRAME                  RGB( 109, 218, 22 )
#define WINDOW_COLOR_BACKGROUND             RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARTEXT           RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARBACKGROUND     RGB( 79, 204, 11 )
#define WINDOW_COLOR_TITLEBARBRIGHT1        RGB( 183, 249, 171 )
#define WINDOW_COLOR_TITLEBARBRIGHT2        RGB( 150, 210, 140 )
#define WINDOW_COLOR_TITLEBARUNDERLINE      RGB( 46, 59, 30 )
#define WINDOW_COLOR_BUTTONBRIGHT           RGB( 229, 229, 229 )
#define WINDOW_COLOR_BUTTONDARK             RGB( 86, 86, 86 )
#define WINDOW_COLOR_SYSTEMBACKGROUND       RGB( 232, 255, 232 )
#define WINDOW_COLOR_XBUTTONLINECOLOR       RGB( 71, 199, 21 )

// 배경 윈도우의 제목
#define WINDOW_BACKGROUNDWINDOWTITLE            "SYS_BACKGROUND"

// 마우스 커서의 너비와 높이
#define MOUSE_CURSOR_WIDTH                  20
#define MOUSE_CURSOR_HEIGHT                 20

// 커서 이미지의 색깔
#define MOUSE_CURSOR_OUTERLINE              RGB(0, 0, 0 )
#define MOUSE_CURSOR_OUTER                  RGB( 79, 204, 11 )
#define MOUSE_CURSOR_INNER                  RGB( 232, 255, 232 )

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 윈도우의 정보를 저장하는 자료구조
typedef struct kWindowStruct
{
    // 다음 데이터의 위치와 현재 윈도우의 ID
    LISTLINK stLink;

    // 자료구조 동기화를 위한 뮤텍스
    MUTEX stLock;

    // 윈도우 영역 정보
    RECT stArea;

    // 윈도우의 화면 버퍼 어드레스
    COLOR* pstWindowBuffer;

    // 윈도우를 가지고 있는 태스크의 ID
    QWORD qwTaskID;

    // 윈도우 속성
    DWORD dwFlags;
    
    // 윈도우 제목
    char vcWindowTitle[ WINDOW_TITLEMAXLENGTH + 1 ];
} WINDOW;

// 윈도우 풀의 상태를 관리하는 자료구조
typedef struct kWindowPoolManagerStruct
{
    // 자료구조 동기화를 위한 뮤텍스
    MUTEX stLock;

    // 윈도우 풀에 대한 정보
    WINDOW* pstStartAddress;
    int iMaxCount;
    int iUseCount;

    // 윈도우가 할당된 횟수
    int iAllocatedCount;
} WINDOWPOOLMANAGER;

// 윈도우 매니저 자료구조
typedef struct kWindowManagerStruct
{
    // 자료구조 동기화를 위한 뮤텍스
    MUTEX stLock;

    // 윈도우 리스트
    LIST stWindowList;

    // 현재 마우스 커서의 X, Y좌표
    int iMouseX;
    int iMouseY;

    // 화면 영역 정보
    RECT stScreenArea;

    // 비디오 메모리의 어드레스
    COLOR* pstVideoMemory;

    // 배경 윈도우의 ID
    QWORD qwBackgoundWindowID;

} WINDOWMANAGER;

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
// 윈도우 풀 관련
static void kInitializeWindowPool( void );
static WINDOW* kAllocateWindow( void );
static void kFreeWindow( QWORD qwID );

// 윈도우와 윈도우 매니저 관련
void kInitializeGUISystem( void );
WINDOWMANAGER* kGetWindowManager( void );
QWORD kGetBackgroundWindowID( void );
void kGetScreenArea( RECT* pstScreenArea );
QWORD kCreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags,
        const char* pcTitle );
BOOL kDeleteWindow( QWORD qwWindowID );
BOOL kDeleteAllWindowInTaskID( QWORD qwTaskID );
WINDOW* kGetWindow( QWORD qwWindowID );
WINDOW* kGetWindowWithWindowLock( QWORD qwWindowID );
BOOL kShowWindow( QWORD qwWindowID, BOOL bShow );
BOOL kRedrawWindowByArea( const RECT* pstArea );
static void kCopyWindowBufferToFrameBuffer( const WINDOW* pstWindow,
        const RECT* pstCopyArea );

// 윈도우 내부에 그리는 함수와 마우스 커서 관련
BOOL kDrawWindowFrame( QWORD qwWindowID );
BOOL kDrawWindowBackground( QWORD qwWindowID );
BOOL kDrawWindowTitle( QWORD qwWindowID, const char* pcTitle );
BOOL kDrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor,
        const char* pcText, COLOR stTextColor );
BOOL kDrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor );
BOOL kDrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor );
BOOL kDrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2,
        COLOR stColor, BOOL bFill );
BOOL kDrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor,
        BOOL bFill );
BOOL kDrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor,
        COLOR stBackgroundColor, const char* pcString, int iLength );
static void kDrawCursor( int iX, int iY );
void kMoveCursor( int iX, int iY );
void kGetCursorPosition( int* piX, int* piY );

#endif /*__WINDOW_H__*/
