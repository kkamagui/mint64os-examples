/**
 *  file    WindowManager.c
 *  date    2009/10/04
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   윈도우 매니저에 관련된 함수를 정의한 소스 파일
 */
 
#include "Types.h"
#include "Window.h"
#include "WindowManagerTask.h"
#include "VBE.h"
#include "Mouse.h"
#include "Task.h"

/**
 *  윈도우 매니저 태스크
 */
void kStartWindowManager( void )
{
    int iMouseX, iMouseY;
    BOOL bMouseDataResult;
    BOOL bKeyDataResult;
    BOOL bEventQueueResult;

    // GUI 시스템을 초기화
    kInitializeGUISystem();
    
    // 현재 마우스 위치에 커서를 출력
    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );
        
    // 윈도우 매니저 태스크 루프
    while( 1 )
    {
        // 마우스 데이터를 처리
        bMouseDataResult = kProcessMouseData();
        
        // 키 데이터를 처리
        bKeyDataResult = kProcessKeyData();

        // 윈도우 매니저의 이벤트 큐에 수신된 데이터를 처리. 수신된 모든 이벤트를 처리함
        bEventQueueResult = FALSE;
        while( kProcessEventQueueData() == TRUE )
        {
            bEventQueueResult = TRUE;
        }
        
        // 처리한 데이터가 하나도 없다면 Sleep()을 수행하여 프로세서를 양보함
        if( ( bMouseDataResult == FALSE ) &&
            ( bKeyDataResult == FALSE ) &&
            ( bEventQueueResult == FALSE ) )
        {
            kSleep( 0 );
        }
    }
}

/**
 *  수신된 마우스 데이터를 처리
 */
BOOL kProcessMouseData( void )
{
    QWORD qwWindowIDUnderMouse;
    BYTE bButtonStatus;
    int iRelativeX, iRelativeY;
    int iMouseX, iMouseY;
    int iPreviousMouseX, iPreviousMouseY;
    BYTE bChangedButton;
    RECT stWindowArea;
    EVENT stEvent;
    WINDOWMANAGER* pstWindowManager;
    char vcTempTitle[ WINDOW_TITLEMAXLENGTH ];
    static int iWindowCount = 0;
    QWORD qwWindowID;

    // 마우스 데이터가 수신되기를 기다림
    if( kGetMouseDataFromMouseQueue( &bButtonStatus, &iRelativeX, &iRelativeY ) ==
        FALSE )
    {
        return FALSE;
    }
    
    // 윈도우 매니저를 반환
    pstWindowManager = kGetWindowManager();

    // 현재 마우스 커서 위치를 반환
    kGetCursorPosition( &iMouseX, &iMouseY );
    
    // 움직이기 이전의 좌표를 저장
    iPreviousMouseX = iMouseX;
    iPreviousMouseY = iMouseY;
    
    // 마우스가 움직인 거리를 이전 커서 위치에 더해서 현재 좌표를 계산
    iMouseX += iRelativeX;
    iMouseY += iRelativeY;
    
    // 새로운 위치로 마우스 커서를 이동하고 다시 현재 커서의 위치를 반환.
    // 마우스 커서가 화면을 벗어나지 않도록 처리된 커서 좌표를 사용하여 화면을 벗어난
    // 커서로 인해 발생하는 문제를 방지
    kMoveCursor( iMouseX, iMouseY );
    kGetCursorPosition( &iMouseX, &iMouseY );
    
    // 현재 마우스 커서 아래에 있는 윈도우를 검색
    qwWindowIDUnderMouse = kFindWindowByPoint( iMouseX, iMouseY );
    
    //--------------------------------------------------------------------------
    // 버튼 상태가 변했는지 확인하고 버튼 상태에 따라 마우스 메시지와 윈도우 메시지를
    // 전송
    //--------------------------------------------------------------------------
    // 버튼 상태는 이전 버튼 상태와 현재 버튼 상태를 XOR하여 1로 설정됐는지를 확인
    bChangedButton = pstWindowManager->bPreviousButtonStatus ^ bButtonStatus;
    
    // 마우스 왼쪽 버튼에 변화가 생긴 경우 처리
    if( bChangedButton & MOUSE_LBUTTONDOWN )
    {
        // 왼쪽 버튼이 눌린 경우 처리
        if( bButtonStatus & MOUSE_LBUTTONDOWN )
        {
            // 마우스로 윈도우를 선택한 것이므로, 마우스 아래에 있는 윈도우가
            // 배경 윈도우가 아닌 경우 최상위로 올려줌
            if( qwWindowIDUnderMouse != pstWindowManager->qwBackgoundWindowID )
            {
                // 선택된 윈도우를 최상위로 만듦
                // 윈도우를 최상위로 만들면서 동시에 윈도우 선택과 선택 해제 이벤트도
                // 같이 전송
                kMoveWindowToTop( qwWindowIDUnderMouse );
            }
            
            //------------------------------------------------------------------
            // 왼쪽 버튼이 눌린 위치가 제목 표시줄의 위치이면 윈도우 이동인지 또는
            // 닫기 버튼 위치에서 눌렸는지 확인하여 처리
            //------------------------------------------------------------------
            if( kIsInTitleBar( qwWindowIDUnderMouse, iMouseX, iMouseY ) == TRUE )
            {
                // 닫기 버튼에서 눌러졌으면 윈도우에 닫기 전송
                if( kIsInCloseButton( qwWindowIDUnderMouse, iMouseX, iMouseY ) 
                        == TRUE )
                {
                    // 윈도우 닫기 이벤트를 전송
                    kSetWindowEvent( qwWindowIDUnderMouse, EVENT_WINDOW_CLOSE,
                            &stEvent );
                    kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
                    
                    //----------------------------------------------------------
                    // 테스트를 위해 일시적으로 추가된 부분
                    //----------------------------------------------------------
                    kDeleteWindow( qwWindowIDUnderMouse );                        
                }
                // 닫기 버튼이 아니면 윈도우 이동 모드로 변경
                else
                {
                    // 윈도우 이동 모드 설정
                    pstWindowManager->bWindowMoveMode = TRUE;
                    
                    // 현재 윈도우를 이동하는 윈도우로 설정
                    pstWindowManager->qwMovingWindowID = qwWindowIDUnderMouse;
                }
            }
            // 윈도우 내부에서 눌린 것이면 왼쪽 버튼이 눌러졌음을 전송
            else
            {
                // 왼쪽 버튼 눌림 이벤트를 전송
                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONDOWN,
                        iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            }
        }
        // 왼쪽 버튼이 떨어진 경우 처리
        else
        {
            // 윈도우가 이동 중이었으면 모드만 해제
            if( pstWindowManager->bWindowMoveMode == TRUE )
            {
                // 이동 중이라는 플래그를 해제
                pstWindowManager->bWindowMoveMode = FALSE;
                pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;
            }
            // 윈도우가 이동 중이 아니었으면 윈도우로 왼쪽 버튼 떨어짐 이벤트를 전송
            else
            {
                // 왼쪽 버튼 떨어짐 이벤트를 전송
                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONUP,
                        iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            }
        }
    }            
    // 마우스 오른쪽 버튼에 변화가 생긴 경우 처리
    else if( bChangedButton & MOUSE_RBUTTONDOWN )
    {
        // 오른쪽 버튼이 눌린 경우 처리
        if( bButtonStatus & MOUSE_RBUTTONDOWN )
        {
            // 오른쪽 버튼 눌림 이벤트를 전송
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONDOWN,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
            
            //------------------------------------------------------------------
            // 테스트를 위해 일시적으로 추가된 부분
            //------------------------------------------------------------------
            // 테스트를 위해 오른쪽 버튼이 눌리면 윈도우 생성
            kSPrintf( vcTempTitle, "MINT64 OS Test Window %d", iWindowCount++ );
            qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
                 400, 200, WINDOW_FLAGS_DRAWFRAME | WINDOW_FLAGS_DRAWTITLE, vcTempTitle );                    
            
            // 윈도우 내부에 텍스트를 출력하고 윈도우를 화면에 나타냄
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 10, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, "This is real window~!!", 22 );
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 30, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, "Now you can move and select window~!!", 
                    37 );
            kShowWindow( qwWindowID, TRUE );
        }
        else
        {
            // 오른쪽 버튼 떨어짐 이벤트를 전송
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONUP,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
    }
    // 마우스 가운데 버튼에 변화가 생긴 경우 처리
    else if( bChangedButton & MOUSE_MBUTTONDOWN )
    {
        // 가운데 버튼이 눌린 경우 처리
        if( bButtonStatus & MOUSE_MBUTTONDOWN )
        {
            // 가운데 버튼 눌림 이벤트를 전송
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONDOWN,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
        else
        {
            // 가운데 버튼 떨어짐 이벤트를 전송
            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONUP,
                    iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
        }
    }        
    // 마우스 버튼이 변경되지 않았으면 마우스 이동 처리만 수행
    else
    {
        // 마우스 이동 이벤트를 전송
        kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MOVE, iMouseX, iMouseY, 
                    bButtonStatus, &stEvent );
        kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );
    }

    // 윈도우가 이동 중이었으면 윈도우 이동 처리
    if( pstWindowManager->bWindowMoveMode == TRUE )
    {
        // 윈도우의 위치를 얻음
        if( kGetWindowArea( pstWindowManager->qwMovingWindowID, &stWindowArea )
                == TRUE )
        {
            // 윈도우의 현재 위치를 얻어서 마우스가 이동한 만큼 옮겨줌
            // 윈도우 이동 이벤트는 함수 내부에서 전달
            kMoveWindow( pstWindowManager->qwMovingWindowID, 
                         stWindowArea.iX1 + iMouseX - iPreviousMouseX, 
                         stWindowArea.iY1 + iMouseY - iPreviousMouseY ); 
        }
        // 윈도우의 위치를 얻을 수 없으면 윈도우가 존재하지 않는 것이므로
        // 윈도우 이동 모드 해제
        else
        {
            // 이동 중이라는 플래그를 해제
            pstWindowManager->bWindowMoveMode = FALSE;
            pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;
        }
    }
    
    // 다음 처리에 사용하려고 현재 버튼 상태를 저장함
    pstWindowManager->bPreviousButtonStatus = bButtonStatus;
    return TRUE;
}


/**
 *  수신된 키 데이터를 처리
 */
BOOL kProcessKeyData( void )
{
    KEYDATA stKeyData;
    EVENT stEvent;
    QWORD qwAcitveWindowID;
    
    // 키보드 데이터가 수신되기를 기다림
    if( kGetKeyFromKeyQueue( &stKeyData ) == FALSE )
    {
        return FALSE;
    }

    // 최상위 윈도우, 즉 선택된 윈도우로 메시지를 전송
    qwAcitveWindowID = kGetTopWindowID();
    kSetKeyEvent( qwAcitveWindowID, &stKeyData, &stEvent );
    return kSendEventToWindow( qwAcitveWindowID, &stEvent );
}    

/**
 *  이벤트 큐에 수신된 이벤트 처리
 */
BOOL kProcessEventQueueData( void )
{
    EVENT stEvent;
    WINDOWEVENT* pstWindowEvent;
    QWORD qwWindowID;
    RECT stArea;
    
    // 윈도우 매니저의 이벤트 큐에 이벤트가 수신되기를 기다림
    if( kReceiveEventFromWindowManagerQueue( &stEvent ) == FALSE )
    {
        return FALSE;
    }
    
    pstWindowEvent = &( stEvent.stWindowEvent );
    
    // 타입 별로 처리
    switch( stEvent.qwType )
    {
        // 현재 윈도우가 있는 영역을 화면에 업데이트
    case EVENT_WINDOWMANAGER_UPDATESCREENBYID:
        if( kGetWindowArea( pstWindowEvent->qwWindowID, &stArea ) == TRUE )
        {
            kRedrawWindowByArea( &stArea );        
        }
        break;

        // 윈도우의 내부 영역을 화면에 업데이트
    case EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA:
        // 윈도우를 기준으로 한 좌표를 화면 좌표로 변환하여 업데이트 처리
        if( kConvertRectClientToScreen( pstWindowEvent->qwWindowID, 
                &( pstWindowEvent->stArea ), &stArea ) == TRUE )
        {
            kRedrawWindowByArea( &stArea );
        }
        break;

        // 화면 좌표로 전달된 영역을 화면에 업데이트
    case EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA:
        kRedrawWindowByArea( &( pstWindowEvent->stArea ) );
        break;

    default:
        break;
    }
    
    return TRUE;
}
