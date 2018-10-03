/**
 *  file    Main.h
 *  date    2010/03/10
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C 언어로 작성된 응용프로그램의 엔트리 포인트 파일
 */

#ifndef __MAIN_H__
#define __MAIN_H__

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 게임판의 너비와 높이
#define BOARDWIDTH      8
#define BOARDHEIGHT     12

// 블록 하나의 크기
#define BLOCKSIZE       32
// 움직이는 블록의 개수
#define BLOCKCOUNT      3
// 빈 블록을 나타내는 값
#define EMPTYBLOCK      0
// 지울 블록을 나타내는 값
#define ERASEBLOCK      0xFF
// 블록의 종류
#define BLOCKKIND       5

// 윈도우의 너비와 높이
#define WINDOW_WIDTH        ( BOARDWIDTH * BLOCKSIZE )
#define WINDOW_HEIGHT       ( WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT + \
                              BOARDHEIGHT * BLOCKSIZE )

// 게임 정보 영역의 높이
#define INFORMATION_HEIGHT  20


////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 게임 정보를 저장하는 자료구조
typedef struct GameInfoStruct
{
    //-------------------------------------------------------------------------
    // 현재 움직이는 블록과 게임판에 고정된 블록을 관리하는데 필요한 필드
    //-------------------------------------------------------------------------
    // 블록 종류에 따른 색깔(블록 내부 색깔과 테두리 색깔)
    COLOR vstBlockColor[ BLOCKKIND + 1 ];
    COLOR vstEdgeColor[ BLOCKKIND + 1 ];

    // 현재 움직이는 블록의 위치
    int iBlockX;
    int iBlockY;

    // 게임판에 고정된 블록의 상태를 관리하는 영역
    BYTE vvbBoard[ BOARDHEIGHT ][ BOARDWIDTH ];

    // 게임판에 고정된 블록 중에서 삭제해야 할 블록을 관리하는 영역
    BYTE vvbEraseBlock[ BOARDHEIGHT ][ BOARDWIDTH ];

    // 현재 움직이는 블록의 구성을 저장하는 영역
    BYTE vbBlock[ BLOCKCOUNT ];

    //-------------------------------------------------------------------------
    // 게임을 진행하는데 필요한 필드
    //-------------------------------------------------------------------------
    // 게임의 시작 여부
    BOOL bGameStart;

    // 유저의 점수
    QWORD qwScore;

    // 게임의 레벨
    QWORD qwLevel;
} GAMEINFO;


////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
void Initialize( void );
void CreateBlock( void );
BOOL IsMovePossible( int iBlockX, int iBlockY );
BOOL FreezeBlock( int iBlockX, int iBlockY );
void EraseAllContinuousBlockOnBoard( QWORD qwWindowID );

BOOL MarkContinuousVerticalBlockOnBoard( void );
BOOL MarkContinuousHorizonBlockOnBoard( void );
BOOL MarkContinuousDiagonalBlockInBoard( void );
void EraseMarkedBlock( void );
void CompactBlockOnBoard( void );

void DrawInformation( QWORD qwWindowID );
void DrawGameArea( QWORD qwWindowID );

#endif /* __MAIN_H__ */
