/**
 *  file    Window.c
 *  date    2009/09/28
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI 시스템에 관련된 함수를 정의한 소스 파일
 */
#include "Window.h"
#include "VBE.h"
#include "Task.h"
#include "Font.h"
#include "DynamicMemory.h"

// GUI 시스템 관련 자료구조
static WINDOWPOOLMANAGER gs_stWindowPoolManager;
// 윈도우 매니저 관련 자료구조
static WINDOWMANAGER gs_stWindowManager;

//==============================================================================
//  윈도우 풀 관련
//==============================================================================
/**
 *  윈도우 풀을 초기화
 */
static void kInitializeWindowPool( void )
{
    int i;
    void* pvWindowPoolAddress;
    
    // 자료구조 초기화
    kMemSet( &gs_stWindowPoolManager, 0, sizeof( gs_stWindowPoolManager ) );
    
    // 윈도우 풀의 메모리를 할당
    pvWindowPoolAddress = ( void* ) kAllocateMemory( sizeof( WINDOW ) * WINDOW_MAXCOUNT );
    if( pvWindowPoolAddress == NULL )
    {
        kPrintf( "Window Pool Allocate Fail\n" );
        while( 1 )
        {
            ;
        }
    }
    
    // 윈도우 풀의 어드레스를 지정하고 초기화
    gs_stWindowPoolManager.pstStartAddress = ( WINDOW* ) pvWindowPoolAddress;
    kMemSet( pvWindowPoolAddress, 0, sizeof( WINDOW ) * WINDOW_MAXCOUNT );

    // 윈도우 풀에 ID를 할당
    for( i = 0 ; i < WINDOW_MAXCOUNT ; i++ )
    {
        gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    }
    
    // 윈도우의 최대 개수와 할당된 횟수를 초기화
    gs_stWindowPoolManager.iMaxCount = WINDOW_MAXCOUNT;
    gs_stWindowPoolManager.iAllocatedCount = 1;
    
    // 뮤텍스 초기화
    kInitializeMutex( &( gs_stWindowPoolManager.stLock ) );
}


/**
 *  윈도우 자료구조를 할당
 */
static WINDOW* kAllocateWindow( void )
{
    WINDOW* pstEmptyWindow;
    int i;

    // 동기화 처리
    kLock( &( gs_stWindowPoolManager.stLock ) );

    // 윈도우가 모두 할당되었으면 실패
    if( gs_stWindowPoolManager.iUseCount == gs_stWindowPoolManager.iMaxCount )
    {
        // 동기화 처리
        kUnlock( &gs_stWindowPoolManager.stLock );
        return NULL;
    }

    // 윈도우 풀을 모두 돌면서 빈 영역을 검색
    for( i = 0 ; i < gs_stWindowPoolManager.iMaxCount ; i++ )
    {
        // ID의 상위 32비트가 0이면 비어있는 윈도우 자료구조임
        if( ( gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 )
        {
            pstEmptyWindow = &( gs_stWindowPoolManager.pstStartAddress[ i ] );
            break;
        }
    }

    // 상위 32비트를 0이 아닌 값으로 설정해서 할당된 윈도우로 설정
    pstEmptyWindow->stLink.qwID =
        ( ( QWORD ) gs_stWindowPoolManager.iAllocatedCount << 32 ) | i;

    // 자료구조가 사용 중인 개수와 할당된 횟수를 증가
    gs_stWindowPoolManager.iUseCount++;
    gs_stWindowPoolManager.iAllocatedCount++;
    if( gs_stWindowPoolManager.iAllocatedCount == 0 )
    {
        gs_stWindowPoolManager.iAllocatedCount = 1;
    }

    // 동기화 처리
    kUnlock( &( gs_stWindowPoolManager.stLock ) );

    // 윈도우의 뮤텍스 초기화
    kInitializeMutex( &( pstEmptyWindow->stLock ) );

    return pstEmptyWindow;
}

/**
 *  윈도우 자료구조를 해제
 */
static void kFreeWindow( QWORD qwID )
{
    int i;

    // 윈도우 ID로 윈도우 풀의 오프셋을 계산, 윈도우 ID의 하위 32비트가 인덱스 역할을 함
    i = GETWINDOWOFFSET( qwID );

    // 동기화 처리
    kLock( &( gs_stWindowPoolManager.stLock ) );
    
    // 윈도우 자료구조를 초기화하고 ID 설정
    kMemSet( &( gs_stWindowPoolManager.pstStartAddress[ i ] ), 0, sizeof( WINDOW ) );
    gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    // 사용 중인 자료구조의 개수를 감소
    gs_stWindowPoolManager.iUseCount--;

    // 동기화 처리
    kUnlock( &( gs_stWindowPoolManager.stLock ) );
}

//==============================================================================
//  윈도우와 윈도우 매니저 관련
//==============================================================================
/**
 *  GUI 시스템을 초기화
 */
void kInitializeGUISystem( void )
{
    VBEMODEINFOBLOCK* pstModeInfo;
    QWORD qwBackgroundWindowID;

    // 윈도우 풀을 초기화
    kInitializeWindowPool();

    // VBE 모드 정보 블록을 반환
    pstModeInfo = kGetVBEModeInfoBlock();

    // 비디오 메모리 어드레스 설정
    gs_stWindowManager.pstVideoMemory = ( COLOR* )
        ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer & 0xFFFFFFFF );

    // 마우스 커서의 초기 위치 설정
    gs_stWindowManager.iMouseX = pstModeInfo->wXResolution / 2;
    gs_stWindowManager.iMouseY = pstModeInfo->wYResolution / 2;

    // 화면 영역의 범위 설정
    gs_stWindowManager.stScreenArea.iX1 = 0;
    gs_stWindowManager.stScreenArea.iY1 = 0;
    gs_stWindowManager.stScreenArea.iX2 = pstModeInfo->wXResolution - 1;
    gs_stWindowManager.stScreenArea.iY2 = pstModeInfo->wYResolution - 1;

    // 뮤텍스 초기화
    kInitializeMutex( &( gs_stWindowManager.stLock ) );

    // 윈도우 리스트 초기화
    kInitializeList( &( gs_stWindowManager.stWindowList ) );

    //--------------------------------------------------------------------------
    // 배경 윈도우 생성
    //--------------------------------------------------------------------------
    // 플래그에 0을 넘겨서 화면에 윈도우를 그리지 않도록 함. 배경 윈도우는 윈도우 내에 
    // 배경색을 모두 칠한 뒤 나타냄
    qwBackgroundWindowID = kCreateWindow( 0, 0, pstModeInfo->wXResolution, 
            pstModeInfo->wYResolution, 0, WINDOW_BACKGROUNDWINDOWTITLE );
    gs_stWindowManager.qwBackgoundWindowID = qwBackgroundWindowID; 
    // 배경 윈도우 내부에 배경색을 채움
    kDrawRect( qwBackgroundWindowID, 0, 0, pstModeInfo->wXResolution - 1, 
            pstModeInfo->wYResolution - 1, WINDOW_COLOR_SYSTEMBACKGROUND, TRUE );
    // 배경 윈도우를 화면에 나타냄
    kShowWindow( qwBackgroundWindowID, TRUE );
}

/**
 *  윈도우 매니저를 반환
 */
WINDOWMANAGER* kGetWindowManager( void )
{
    return &gs_stWindowManager;
}

/**
 *  배경 윈도우의 ID를 반환
 */
QWORD kGetBackgroundWindowID( void )
{
    return gs_stWindowManager.qwBackgoundWindowID;
}

/**
 *  화면 영역의 크기를 반환
 */
void kGetScreenArea( RECT* pstScreenArea )
{
    kMemCpy( pstScreenArea, &( gs_stWindowManager.stScreenArea ), sizeof( RECT ) );
}

/**
 *  윈도우를 생성
 */
QWORD kCreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags,
        const char* pcTitle )
{
    WINDOW* pstWindow;
    TCB* pstTask;

    // 크기가 0인 윈도우는 만들 수 없음
    if( ( iWidth <= 0 ) || ( iHeight <= 0 ) )
    {
        return WINDOW_INVALIDID;
    }

    // 윈도우 자료구조를 할당
    pstWindow = kAllocateWindow();
    if( pstWindow == NULL )
    {
        return WINDOW_INVALIDID;
    }

    // 윈도우 영역 설정
    pstWindow->stArea.iX1 = iX;
    pstWindow->stArea.iY1 = iY;
    pstWindow->stArea.iX2 = iX + iWidth - 1;
    pstWindow->stArea.iY2 = iY + iHeight - 1;
    
    // 윈도우 제목 저장
    kMemCpy( pstWindow->vcWindowTitle, pcTitle, WINDOW_TITLEMAXLENGTH );
    pstWindow->vcWindowTitle[ WINDOW_TITLEMAXLENGTH ] = '\0';

    // 윈도우 화면 버퍼 할당
    pstWindow->pstWindowBuffer = ( COLOR* ) kAllocateMemory( iWidth * iHeight *
            sizeof( COLOR ) );
    if( pstWindow == NULL )
    {
        // 윈도우 화면 버퍼 할당에 실패하면 윈도우 자료구조 반환
        kFreeWindow( pstWindow->stLink.qwID );
        return WINDOW_INVALIDID;
    }

    // 윈도우를 생성한 태스크의 ID를 저장
    pstTask = kGetRunningTask( kGetAPICID() );
    pstWindow->qwTaskID =  pstTask->stLink.qwID;

    // 윈도우 속성 설정
    pstWindow->dwFlags = dwFlags;

    // 윈도우 배경 그리기
    kDrawWindowBackground( pstWindow->stLink.qwID );

    // 윈도우 테두리 그리기
    if( dwFlags & WINDOW_FLAGS_DRAWFRAME )
    {
        kDrawWindowFrame( pstWindow->stLink.qwID );
    }

    // 윈도우 제목 표시줄 그리기
    if( dwFlags & WINDOW_FLAGS_DRAWTITLE )
    {
        kDrawWindowTitle( pstWindow->stLink.qwID, pcTitle );
    }

    // 동기화 처리
    kLock( &( gs_stWindowManager.stLock ) );

    // 윈도우 리스트의 가장 마지막에 추가하여 최상위 윈도우로 설정
    kAddListToTail( &gs_stWindowManager.stWindowList, pstWindow );

    // 동기화 처리
    kUnlock( &( gs_stWindowManager.stLock ) );

    // 윈도우를 그리는 옵션이 들어있으면 해당 윈도우를 그림
    if( dwFlags & WINDOW_FLAGS_SHOW )
    {
        // 윈도우 영역만큼 화면에 업데이트
        kRedrawWindowByArea( &( pstWindow->stArea ) );
    }

    return pstWindow->stLink.qwID;
}

/**
 *  윈도우를 삭제
 */
BOOL kDeleteWindow( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    RECT stArea;

    // 동기화 처리
    kLock( &( gs_stWindowManager.stLock ) );
    
    // 윈도우 검색
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        // 동기화 처리
        kUnlock( &( gs_stWindowManager.stLock ) );
        return FALSE;
    }

    // 윈도우를 삭제하기 전에 영역을 저장해둠
    kMemCpy( &stArea, &( pstWindow->stArea ), sizeof( RECT ) );

    // 윈도우 리스트에서 윈도우 삭제
    if( kRemoveList( &( gs_stWindowManager.stWindowList ), qwWindowID ) == NULL )
    {
        // 동기화 처리
        kUnlock( &( pstWindow->stLock ) );
        kUnlock( &( gs_stWindowManager.stLock ) );
        return FALSE;
    }

    // 동기화 처리
    kLock( &( pstWindow->stLock ) );

    // 윈도우 화면 버퍼를 반환
    kFreeMemory( pstWindow->pstWindowBuffer );

    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );

    // 윈도우 자료구조를 반환
    kFreeWindow( qwWindowID );
    
    // 동기화 처리
    kUnlock( &( gs_stWindowManager.stLock ) );

    // 삭제되기 전에 윈도우가 있던 영역을 화면에 다시 업데이트
    kRedrawWindowByArea( &stArea );
    return TRUE;
}

/**
 *  태스크 ID가 일치하는 모든 윈도우를 삭제
 */
BOOL kDeleteAllWindowInTaskID( QWORD qwTaskID )
{
    WINDOW* pstWindow;
    WINDOW* pstNextWindow;

    // 동기화 처리
    kLock( &( gs_stWindowManager.stLock ) );

    // 리스트에서 첫 번째 윈도우를 반환
    pstWindow = kGetHeaderFromList( &( gs_stWindowManager.stWindowList ) );
    while( pstWindow != NULL )
    {
        // 다음 윈도우를 미리 구함
        pstNextWindow = kGetNextFromList( &( gs_stWindowManager.stWindowList ),
                pstWindow );

        // 배경 윈도우가 아니고 태스크 ID가 일치하면 윈도우 삭제
        if( ( pstWindow->stLink.qwID != gs_stWindowManager.qwBackgoundWindowID ) &&
            ( pstWindow->qwTaskID == qwTaskID ) )
        {
            kDeleteWindow( pstWindow->stLink.qwID );
        }

        // 미리 구해둔 다음 윈도우의 값을 설정
        pstWindow = pstNextWindow;
    }

    // 동기화 처리
    kUnlock( &( gs_stWindowManager.stLock ) );
}

/**
 *  윈도우 ID로 윈도우 포인터를 반환
 */
WINDOW* kGetWindow( QWORD qwWindowID )
{
    WINDOW* pstWindow;

    // 윈도우 ID의 유효 범위 검사
    if( GETWINDOWOFFSET( qwWindowID ) >= WINDOW_MAXCOUNT )
    {
        return NULL;
    }

    // ID로 윈도우 포인터를 찾은 뒤 ID가 일치하면 반환
    pstWindow = &gs_stWindowPoolManager.pstStartAddress[ GETWINDOWOFFSET( qwWindowID )];
    if( pstWindow->stLink.qwID == qwWindowID )
    {
        return pstWindow;
    }

    return NULL;
}

/**
 *  윈도우 ID로 윈도우 포인터를 찾아 윈도우 뮤텍스를 잠근 뒤 반환
 */
WINDOW* kGetWindowWithWindowLock( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    BOOL bResult;

    // 윈도우를 검색
    pstWindow = kGetWindow( qwWindowID );
    if( pstWindow == NULL )
    {
        return NULL;
    }
    
    // 동기화 처리한 뒤 다시 윈도우 ID로 윈도우 검색
    kLock( &(pstWindow->stLock ) );
    // 윈도우 동기화를 한 뒤에 윈도우 ID로 윈도우를 검색할 수 없다면 도중에 윈도우가
    // 바뀐 것이므로 NULL 반환
    pstWindow = kGetWindow( qwWindowID );
    if( pstWindow == NULL )
    {
        // 동기화 처리
        kUnlock( &(pstWindow->stLock ) );
        return NULL;
    }
    
    return pstWindow;
}

/**
 *  윈도우를 화면에 나타내거나 숨김
 */
BOOL kShowWindow( QWORD qwWindowID, BOOL bShow )
{
    WINDOW* pstWindow;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우 속성 설정
    if( bShow == TRUE )
    {
        pstWindow->dwFlags |= WINDOW_FLAGS_SHOW;
    }
    else
    {
        pstWindow->dwFlags &= ~WINDOW_FLAGS_SHOW;
    }

    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );
    
    // 윈도우가 있던 영역을 다시 업데이트함으로써 윈도우를 나타내거나 숨김
    kRedrawWindowByArea( &( pstWindow->stArea ) );
    return TRUE;
}

/**
 *  특정 영역을 포함하는 윈도우는 모두 그림
 */
BOOL kRedrawWindowByArea( const RECT* pstArea )
{
    WINDOW* pstWindow;
    WINDOW* pstTargetWindow = NULL;
    RECT stOverlappedArea;
    RECT stCursorArea;

    // 화면 영역과 겹치는 영역이 없으면 그릴 필요가 없음
    if( kGetOverlappedRectangle( &( gs_stWindowManager.stScreenArea ), pstArea,
            &stOverlappedArea ) == FALSE )
    {
        return FALSE;
    }

    //--------------------------------------------------------------------------
    // Z 순서의 최하위, 즉 윈도우 리스트의 첫 번째부터 마지막까지 루프를 돌면서 
    // 업데이트할 영역과 겹치는 윈도우를 찾아 비디오 메모리로 전송
    //--------------------------------------------------------------------------
    // 동기화 처리
    kLock( &( gs_stWindowManager.stLock ) );

    // 현재 윈도우 리스트는 Z 순서의 역방향, 즉 처음에 있는 윈도우가 Z 순서의 최하위
    // 윈도우가 되고 마지막 윈도우가 최상위 윈도우가 되도록 정렬되어 있음 
    // 따라서 윈도우 리스트를 처음부터 따라가면서 그릴 영역을 포함하는 윈도우를 찾고 
    // 그 윈도우부터 최상위 윈도우까지 화면에 전송하면 됨
    pstWindow = kGetHeaderFromList( &( gs_stWindowManager.stWindowList ) );
    while( pstWindow != NULL )
    {
        // 윈도우를 화면에 나타내는 옵션이 설정되어있으며,
        // 업데이트할 부분과 윈도우가 차지하는 영역이 겹치면 겹치는 만큼을 화면에 전송
        if( ( pstWindow->dwFlags & WINDOW_FLAGS_SHOW ) &&
            ( kIsRectangleOverlapped( &( pstWindow->stArea ), &stOverlappedArea )
                == TRUE ) )
        {
            // 동기화 처리
            kLock( &( pstWindow->stLock ) );

            // 실제로 비디오 메모리로 전송하는 함수
            kCopyWindowBufferToFrameBuffer( pstWindow, &stOverlappedArea );

            // 동기화 처리
            kUnlock( &( pstWindow->stLock ) );
        }

        // 다음 윈도우를 찾음
        pstWindow = kGetNextFromList( &( gs_stWindowManager.stWindowList ),
                pstWindow );
    }

    // 동기화 처리
    kUnlock( &( gs_stWindowManager.stLock ) );

    //--------------------------------------------------------------------------
    // 마우스 커서 영역이 포함되면 마우스 커서도 같이 그림
    //--------------------------------------------------------------------------
    // 마우스 영역을 RECT 자료구조에 설정
    kSetRectangleData( gs_stWindowManager.iMouseX, gs_stWindowManager.iMouseY,
            gs_stWindowManager.iMouseX + MOUSE_CURSOR_WIDTH,
            gs_stWindowManager.iMouseY + MOUSE_CURSOR_HEIGHT, &stCursorArea );
    
    // 겹치는지 확인하여 겹친다면 마우스 커서도 그림
    if( kIsRectangleOverlapped( &stOverlappedArea, &stCursorArea ) == TRUE )
    {
        kDrawCursor( gs_stWindowManager.iMouseX, gs_stWindowManager.iMouseY );
    }
}

/**
 *  윈도우 화면 버퍼의 일부 또는 전체를 프레임 버퍼로 복사
 */
static void kCopyWindowBufferToFrameBuffer( const WINDOW* pstWindow,
        const RECT* pstCopyArea )
{
    RECT stTempArea;
    RECT stOverlappedArea;
    int iOverlappedWidth;
    int iOverlappedHeight;
    int iScreenWidth;
    int iWindowWidth;
    int i;
    COLOR* pstCurrentVideoMemoryAddress;
    COLOR* pstCurrentWindowBufferAddress;

    // 전송해야 하는 영역과 화면 영역이 겹치는 부분을 임시로 계산
    if( kGetOverlappedRectangle( &( gs_stWindowManager.stScreenArea ), pstCopyArea,
            &stTempArea ) == FALSE )
    {
        return ;
    }

    // 윈도우 영역과 임시로 계산한 영역이 겹치는 부분을 다시 계산
    // 두 영역이 겹치지 않는다면 비디오 메모리로 전송할 필요 없음
    if( kGetOverlappedRectangle( &stTempArea, &( pstWindow->stArea ),
            &stOverlappedArea ) == FALSE )
    {
        return ;
    }

    // 각 영역의 너비와 높이를 계산
    iScreenWidth = kGetRectangleWidth( &( gs_stWindowManager.stScreenArea ) );
    iWindowWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iOverlappedWidth = kGetRectangleWidth( &stOverlappedArea );
    iOverlappedHeight = kGetRectangleHeight( &stOverlappedArea );

    // 전송을 시작할 비디오 메모리 어드레스와 윈도우 화면 버퍼의 어드레스를 계산
    pstCurrentVideoMemoryAddress = gs_stWindowManager.pstVideoMemory +
        stOverlappedArea.iY1 * iScreenWidth + stOverlappedArea.iX1;

    // 윈도우 화면 버퍼는 화면 전체가 아닌 윈도우를 기준으로 한 좌표이므로,
    // 겹치는 영역을 윈도우 내부 좌표 기준으로 변환
    pstCurrentWindowBufferAddress = pstWindow->pstWindowBuffer +
        ( stOverlappedArea.iY1 - pstWindow->stArea.iY1 ) * iWindowWidth +
        ( stOverlappedArea.iX1 - pstWindow->stArea.iX1 );

    // 루프를 돌면서 윈도우 화면 버퍼의 내용을 비디오 메모리로 복사
    for( i = 0 ; i < iOverlappedHeight ; i++ )
    {
        // 라인 별로 한번에 전송
        kMemCpy( pstCurrentVideoMemoryAddress, pstCurrentWindowBufferAddress,
                iOverlappedWidth * sizeof( COLOR ) );

        // 다음 라인으로 메모리 어드레스 이동
        pstCurrentVideoMemoryAddress += iScreenWidth;
        pstCurrentWindowBufferAddress += iWindowWidth;
    }
}


//==============================================================================
//  윈도우 내부에 그리는 함수와 마우스 커서 관련
//==============================================================================
/**
 *  윈도우 화면 버퍼에 윈도우 테두리 그리기
 */
BOOL kDrawWindowFrame( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    RECT stArea;
    int iWidth;
    int iHeight;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우의 너비와 높이를 계산
    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // 클리핑 영역 설정
    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    // 윈도우 프레임의 가장자리를 그림, 2 픽셀 두께
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            0, 0, iWidth - 1, iHeight - 1, WINDOW_COLOR_FRAME, FALSE );

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            1, 1, iWidth - 2, iHeight - 2, WINDOW_COLOR_FRAME, FALSE );

    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );

    return TRUE;
}


/**
 *  윈도우 화면 버퍼에 배경 그리기
 */
BOOL kDrawWindowBackground( QWORD qwWindowID )
{
    WINDOW* pstWindow;
    int iWidth;
    int iHeight;
    RECT stArea;
    int iX;
    int iY;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }

    // 윈도우의 너비와 높이를 계산
    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // 클리핑 영역 설정
    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    // 윈도우에 제목 표시줄이 있으면 그 아래부터 채움
    if( pstWindow->dwFlags & WINDOW_FLAGS_DRAWTITLE )
    {
        iY = WINDOW_TITLEBAR_HEIGHT;
    }
    else
    {
        iY = 0;
    }

    // 윈도우 테두리를 그리는 옵션이 설정되어 있으면 테두리를 제외한 영역을 채움
    if( pstWindow->dwFlags & WINDOW_FLAGS_DRAWFRAME )
    {
        iX = 2;
    }
    else
    {
        iX = 0;
    }

    // 윈도우의 내부를 채움
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            iX, iY, iWidth - 1 - iX, iHeight - 1 - iX, WINDOW_COLOR_BACKGROUND, 
            TRUE );
    
    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );

    return TRUE;
}

/**
 *  윈도우 화면 버퍼에 윈도우 제목 표시줄 그리기
 */
BOOL kDrawWindowTitle( QWORD qwWindowID, const char* pcTitle )
{
    WINDOW* pstWindow;
    int iWidth;
    int iHeight;
    int iX;
    int iY;
    RECT stArea;
    RECT stButtonArea;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우의 너비와 높이를 계산
    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // 클리핑 영역 설정
    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    //--------------------------------------------------------------------------
    // 제목 표시줄 그리기
    //--------------------------------------------------------------------------
    // 제목 표시줄을 채움
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            0, 3, iWidth - 1, WINDOW_TITLEBAR_HEIGHT - 1,
            WINDOW_COLOR_TITLEBARBACKGROUND, TRUE );

    // 윈도우 제목을 표시
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer,
            6, 3, WINDOW_COLOR_TITLEBARTEXT, WINDOW_COLOR_TITLEBARBACKGROUND,
            pcTitle, kStrLen( pcTitle ) );

    // 제목 표시줄을 입체로 보이게 위쪽의 선을 그림, 2 픽셀 두께
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            1, 1, iWidth - 1, 1, WINDOW_COLOR_TITLEBARBRIGHT1 );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            1, 2, iWidth - 1, 2, WINDOW_COLOR_TITLEBARBRIGHT2 );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            1, 2, 1, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBRIGHT1 );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            2, 2, 2, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBRIGHT2 );

    // 제목 표시줄의 아래쪽에 선을 그림
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            2, WINDOW_TITLEBAR_HEIGHT - 2, iWidth - 2, WINDOW_TITLEBAR_HEIGHT - 2,
            WINDOW_COLOR_TITLEBARUNDERLINE );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            2, WINDOW_TITLEBAR_HEIGHT - 1, iWidth - 2, WINDOW_TITLEBAR_HEIGHT - 1,
            WINDOW_COLOR_TITLEBARUNDERLINE );

    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );

    //--------------------------------------------------------------------------
    // 닫기 버튼 그리기
    //--------------------------------------------------------------------------
    // 닫기 버튼을 그림, 오른쪽 위에 표시
    stButtonArea.iX1 = iWidth - WINDOW_XBUTTON_SIZE - 1;
    stButtonArea.iY1 = 1;
    stButtonArea.iX2 = iWidth - 2;
    stButtonArea.iY2 = WINDOW_XBUTTON_SIZE - 1;
    kDrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, "", 
            WINDOW_COLOR_BACKGROUND );

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 닫기 버튼 내부에 대각선 X를 3 픽셀로 그림
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 1 + 4, iWidth - 2 - 4,
            WINDOW_TITLEBAR_HEIGHT - 6, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 5, 1 + 4, iWidth - 2 - 4,
            WINDOW_TITLEBAR_HEIGHT - 7, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 1 + 5, iWidth - 2 - 5,
            WINDOW_TITLEBAR_HEIGHT - 6, WINDOW_COLOR_XBUTTONLINECOLOR );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 19 - 4, iWidth - 2 - 4, 1 + 4,
            WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 5, 19 - 4, iWidth - 2 - 4, 1 + 5,
            WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            iWidth - 2 - 18 + 4, 19 - 5, iWidth - 2 - 5, 1 + 4,
            WINDOW_COLOR_XBUTTONLINECOLOR );

    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );
    
    return TRUE;
}

/**
 *  윈도우 내부에 버튼 그리기
 */
BOOL kDrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor,
        const char* pcText, COLOR stTextColor )
{
    WINDOW* pstWindow;
    RECT stArea;
    int iWindowWidth;
    int iWindowHeight;
    int iTextLength;
    int iTextWidth;
    int iButtonWidth;
    int iButtonHeight;
    int iTextX;
    int iTextY;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우의 너비와 높이를 계산
    iWindowWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iWindowHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    // 클리핑 영역 설정
    kSetRectangleData( 0, 0, iWindowWidth - 1, iWindowHeight - 1, &stArea );

    // 버튼의 배경색을 표시
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX2,
            pstButtonArea->iY2, stBackgroundColor, TRUE );

    // 버튼과 텍스트의 너비와 높이를 계산
    iButtonWidth = kGetRectangleWidth( pstButtonArea );
    iButtonHeight = kGetRectangleHeight( pstButtonArea );
    iTextLength = kStrLen( pcText );
    iTextWidth = iTextLength * FONT_ENGLISHWIDTH;
    
    // 텍스트가 버튼의 가운데에 위치하도록 출력함
    iTextX = ( pstButtonArea->iX1 + iButtonWidth / 2 ) - iTextWidth / 2;
    iTextY = ( pstButtonArea->iY1 + iButtonHeight / 2 ) - FONT_ENGLISHHEIGHT / 2;
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, iTextX, iTextY, 
            stTextColor, stBackgroundColor, pcText, iTextLength );      
    
    // 버튼을 입체로 보이게 테두리를 그림, 2 픽셀 두께로 그림
    // 버튼의 왼쪽과 위는 밝게 표시
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX2,
            pstButtonArea->iY1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1 + 1, pstButtonArea->iX2 - 1,
            pstButtonArea->iY1 + 1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX1,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1 + 1, pstButtonArea->iY1, pstButtonArea->iX1 + 1,
            pstButtonArea->iY2 - 1, WINDOW_COLOR_BUTTONBRIGHT );

    // 버튼의 오른쪽과 아래는 어둡게 표시
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1 + 1, pstButtonArea->iY2, pstButtonArea->iX2,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONDARK );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX1 + 2, pstButtonArea->iY2 - 1, pstButtonArea->iX2,
            pstButtonArea->iY2 - 1, WINDOW_COLOR_BUTTONDARK );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX2, pstButtonArea->iY1 + 1, pstButtonArea->iX2,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONDARK );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer,
            pstButtonArea->iX2 - 1, pstButtonArea->iY1 + 2, pstButtonArea->iX2 -1,
            pstButtonArea->iY2, WINDOW_COLOR_BUTTONDARK );
    
    // 동기화 처리
    kUnlock( &( pstWindow->stLock ) );

    return TRUE;
}

// 마우스 커서의 이미지를 저장하는 데이터
static BYTE gs_vwMouseBuffer[ MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT ] =
{
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 0, 0,
    0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 2, 3, 3, 3, 2, 2, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 3, 2, 1, 1, 2, 3, 2, 2, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 2, 1, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0,
    0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1,
    0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
};

/**
 *  X, Y 위치에 마우스 커서를 출력
 */
static void kDrawCursor( int iX, int iY )
{
    int i;
    int j;
    BYTE* pbCurrentPos;

    // 커서 데이터의 시작 위치를 설정
    pbCurrentPos = gs_vwMouseBuffer;

    // 커서의 너비와 높이만큼 루프를 돌면서 픽셀을 화면에 출력
    for( j = 0 ; j < MOUSE_CURSOR_HEIGHT ; j++ )
    {
        for( i = 0 ; i < MOUSE_CURSOR_WIDTH ; i++ )
        {
            switch( *pbCurrentPos )
            {
                // 0은 출력하지 않음
            case 0:
                // nothing
                break;

                // 가장 바깥쪽 테두리, 검은색으로 출력
            case 1:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ),
                        gs_stWindowManager.pstVideoMemory, i + iX, j + iY,
                        MOUSE_CURSOR_OUTERLINE );
                break;

                // 안쪽과 바깥쪽의 경계, 어두운 녹색으로 출력
            case 2:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ),
                        gs_stWindowManager.pstVideoMemory, i + iX, j + iY,
                        MOUSE_CURSOR_OUTER );
                break;

                // 커서의 안, 밝은 색으로 출력
            case 3:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ),
                        gs_stWindowManager.pstVideoMemory, i + iX, j + iY,
                        MOUSE_CURSOR_INNER );
                break;
            }

            // 커서의 픽셀이 표시됨에 따라 커서 데이터의 위치도 같이 이동
            pbCurrentPos++;
        }
    }
}

/**
 *  마우스 커서를 해당 위치로 이동해서 그려줌
 */
void kMoveCursor( int iX, int iY )
{
    RECT stPreviousArea;

    // 마우스 커서가 화면을 벗어나지 못하도록 보정
    if( iX < gs_stWindowManager.stScreenArea.iX1 )
    {
        iX = gs_stWindowManager.stScreenArea.iX1;
    }
    else if( iX > gs_stWindowManager.stScreenArea.iX2 )
    {
        iX = gs_stWindowManager.stScreenArea.iX2;
    }

    if( iY < gs_stWindowManager.stScreenArea.iY1 )
    {
        iY = gs_stWindowManager.stScreenArea.iY1;
    }
    else if( iY > gs_stWindowManager.stScreenArea.iY2 )
    {
        iY = gs_stWindowManager.stScreenArea.iY2;
    }

    // 동기화 처리
    kLock( &( gs_stWindowManager.stLock ) );
    
    // 이전에 마우스 커서가 있던 자리를 저장
    stPreviousArea.iX1 = gs_stWindowManager.iMouseX;
    stPreviousArea.iY1 = gs_stWindowManager.iMouseY;
    stPreviousArea.iX2 = gs_stWindowManager.iMouseX + MOUSE_CURSOR_WIDTH - 1;
    stPreviousArea.iY2 = gs_stWindowManager.iMouseY + MOUSE_CURSOR_HEIGHT - 1;
    
    // 마우스 커서의 새 위치를 저장
    gs_stWindowManager.iMouseX = iX;
    gs_stWindowManager.iMouseY = iY;

    // 동기화 처리
    kUnlock( &( gs_stWindowManager.stLock ) );
    
    // 마우스가 이전에 있던 영역을 다시 그림
    kRedrawWindowByArea( &stPreviousArea );

    // 새로운 위치에 마우스 커서를 출력
    kDrawCursor( iX, iY );
}

/**
 *  현재 마우스 커서의 위치를 반환
 */
void kGetCursorPosition( int* piX, int* piY )
{
    *piX = gs_stWindowManager.iMouseX;
    *piY = gs_stWindowManager.iMouseY;
}

/**
 *  윈도우 내부에 점 그리기
 */
BOOL kDrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor )
{
    WINDOW* pstWindow;
    RECT stArea;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우 시작 좌표를 0,0으로 하는 좌표로 영역을 변환
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    // 내부 함수를 호출
    kInternalDrawPixel( &stArea, pstWindow->pstWindowBuffer, iX, iY,
            stColor );

    // 동기화 처리
    kUnlock( &pstWindow->stLock );

    return TRUE;
}


/**
 *  윈도우 내부에 직선 그리기
 */
BOOL kDrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    WINDOW* pstWindow;
    RECT stArea;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우 시작 좌표를 0,0으로 하는 윈도우 기준 좌표로 영역을 변환
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // 내부 함수를 호출
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iX1, iY1,
            iX2, iY2, stColor );

    // 동기화 처리
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

/**
 *  윈도우 내부에 사각형 그리기
 */
BOOL kDrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2,
        COLOR stColor, BOOL bFill )
{
    WINDOW* pstWindow;
    RECT stArea;
    
    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }

    // 윈도우 시작 좌표를 0,0으로 하는 윈도우 기준 좌표로 영역을 변환
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // 내부 함수를 호출
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, iX1, iY1,
            iX2, iY2, stColor, bFill );

    // 동기화 처리
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

/**
 *  윈도우 내부에 원 그리기
 */
BOOL kDrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor,
        BOOL bFill )
{
    WINDOW* pstWindow;
    RECT stArea;
    
    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우 시작 좌표를 0,0으로 하는 윈도우 기준 좌표로 영역을 변환
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // 내부 함수를 호출
    kInternalDrawCircle( &stArea, pstWindow->pstWindowBuffer,
            iX, iY, iRadius, stColor, bFill );

    // 동기화 처리
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

/**
 *  윈도우 내부에 문자 출력
 */
BOOL kDrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor,
        COLOR stBackgroundColor, const char* pcString, int iLength )
{
    WINDOW* pstWindow;
    RECT stArea;

    // 윈도우 검색과 동기화 처리
    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL )
    {
        return FALSE;
    }
    
    // 윈도우 시작 좌표를 0,0으로 하는 윈도우 기준 좌표로 영역을 변환
    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, 
            pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );
    
    // 내부 함수를 호출
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, iX, iY,
            stTextColor, stBackgroundColor, pcString, iLength );

    // 동기화 처리
    kUnlock( &pstWindow->stLock );
    return TRUE;
}

