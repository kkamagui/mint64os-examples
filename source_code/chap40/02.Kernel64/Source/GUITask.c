/**
 *  file    GUITask.c
 *  date    2009/10/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI 태스크에 관련된 함수를 정의한 소스 파일
 */
#include "GUITask.h"
#include "Window.h"

//------------------------------------------------------------------------------
//  기본 GUI 태스크
//------------------------------------------------------------------------------
/**
 *  기본 GUI 태스크의 코드
 *      GUI 태스크를 만들 때 복사하여 기본 코드로 사용
 */
void kBaseGUITask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;

    //--------------------------------------------------------------------------
    // 그래픽 모드 판단
    //--------------------------------------------------------------------------
    // MINT64 OS가 그래픽 모드로 시작했는지 확인
    if( kIsGraphicMode() == FALSE )
    {        
        // MINT64 OS가 그래픽 모드로 시작하지 않았다면 실패
        kPrintf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // 윈도우를 생성
    //--------------------------------------------------------------------------
    // 마우스의 현재 위치를 반환
    kGetCursorPosition( &iMouseX, &iMouseY );

    // 윈도우의 크기와 제목 설정
    iWindowWidth = 500;
    iWindowHeight = 200;
    
    // 윈도우 생성 함수 호출, 마우스가 있던 위치를 기준으로 생성
    qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, "Hello World Window" );
    // 윈도우를 생성하지 못했으면 실패
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }
    
    //--------------------------------------------------------------------------
    // GUI 태스크의 이벤트 처리 루프
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // 이벤트 큐에서 이벤트를 수신
        if( kReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // 수신된 이벤트를 타입에 따라 나누어 처리
        switch( stReceivedEvent.qwType )
        {
            // 마우스 이벤트 처리
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:            
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
            // 여기에 마우스 이벤트 처리 코드 넣기
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
            break;

            // 키 이벤트 처리
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // 여기에 키보드 이벤트 처리 코드 넣기
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            break;

            // 윈도우 이벤트 처리
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            // 여기에 윈도우 이벤트 처리 코드 넣기
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );

            //------------------------------------------------------------------
            // 윈도우 닫기 이벤트이면 윈도우를 삭제하고 루프를 빠져나가 태스크를 종료
            //------------------------------------------------------------------
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // 윈도우 삭제
                kDeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // 그 외 정보
        default:
            // 여기에 알 수 없는 이벤트 처리 코드 넣기
            break;
        }
    }
}

//------------------------------------------------------------------------------
//  Hello World GUI 태스크
//------------------------------------------------------------------------------
/**
 *  Hello World GUI 태스크
 */
void kHelloWorldGUITask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;
    int iY;
    char vcTempBuffer[ 50 ];
    static int s_iWindowCount = 0;
    // 이벤트 타입 문자열
    char* vpcEventString[] = { 
            "Unknown",
            "MOUSE_MOVE       ",
            "MOUSE_LBUTTONDOWN",
            "MOUSE_LBUTTONUP  ",
            "MOUSE_RBUTTONDOWN",
            "MOUSE_RBUTTONUP  ",
            "MOUSE_MBUTTONDOWN",
            "MOUSE_MBUTTONUP  ",
            "WINDOW_SELECT    ",
            "WINDOW_DESELECT  ",
            "WINDOW_MOVE      ",
            "WINDOW_RESIZE    ",
            "WINDOW_CLOSE     ",            
            "KEY_DOWN         ",
            "KEY_UP           " };
    RECT stButtonArea;
    QWORD qwFindWindowID;
    EVENT stSendEvent;
    int i;

    //--------------------------------------------------------------------------
    // 그래픽 모드 판단
    //--------------------------------------------------------------------------
    // MINT64 OS가 그래픽 모드로 시작했는지 확인
    if( kIsGraphicMode() == FALSE )
    {        
        // MINT64 OS가 그래픽 모드로 시작하지 않았다면 실패
        kPrintf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // 윈도우를 생성
    //--------------------------------------------------------------------------
    // 마우스의 현재 위치를 반환
    kGetCursorPosition( &iMouseX, &iMouseY );

    // 윈도우의 크기와 제목 설정
    iWindowWidth = 500;
    iWindowHeight = 200;
    
    // 윈도우 생성 함수 호출, 마우스가 있던 위치를 기준으로 생성하고 번호를 추가하여
    // 윈도우마다 개별적인 이름을 할당
    kSPrintf( vcTempBuffer, "Hello World Window %d", s_iWindowCount++ );
    qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, vcTempBuffer );
    // 윈도우를 생성하지 못했으면 실패
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }
    
    //--------------------------------------------------------------------------
    // 윈도우 매니저가 윈도우로 전송하는 이벤트를 표시하는 영역을 그림
    //--------------------------------------------------------------------------
    // 이벤트 정보를 출력할 위치 저장
    iY = WINDOW_TITLEBAR_HEIGHT + 10;
    
    // 이벤트 정보를 표시하는 영역의 테두리와 윈도우 ID를 표시
    kDrawRect( qwWindowID, 10, iY + 8, iWindowWidth - 10, iY + 70, RGB( 0, 0, 0 ),
            FALSE );
    kSPrintf( vcTempBuffer, "GUI Event Information[Window ID: 0x%Q]", qwWindowID );
    kDrawText( qwWindowID, 20, iY, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), 
               vcTempBuffer, kStrLen( vcTempBuffer ) );    
    
    //--------------------------------------------------------------------------
    // 화면 아래에 이벤트 전송 버튼을 그림
    //--------------------------------------------------------------------------
    // 버튼 영역을 설정
    kSetRectangleData( 10, iY + 80, iWindowWidth - 10, iWindowHeight - 10, 
            &stButtonArea );
    // 배경은 윈도우의 배경색으로 설정하고 문자는 검은색으로 설정하여 버튼을 그림
    kDrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, 
            "User Message Send Button(Up)", RGB( 0, 0, 0 ) );
    // 윈도우를 화면에 표시
    kShowWindow( qwWindowID, TRUE );
    
    //--------------------------------------------------------------------------
    // GUI 태스크의 이벤트 처리 루프
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // 이벤트 큐에서 이벤트를 수신
        if( kReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // 윈도우 이벤트 정보가 표시될 영역을 배경색으로 칠하여 이전에 표시한 데이터를
        // 모두 지움
        kDrawRect( qwWindowID, 11, iY + 20, iWindowWidth - 11, iY + 69, 
                   WINDOW_COLOR_BACKGROUND, TRUE );        
        
        // 수신된 이벤트를 타입에 따라 나누어 처리
        switch( stReceivedEvent.qwType )
        {
            // 마우스 이벤트 처리
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:            
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
            // 여기에 마우스 이벤트 처리 코드 넣기
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            // 마우스 이벤트의 타입을 출력
            kSPrintf( vcTempBuffer, "Mouse Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // 마우스 데이터를 출력
            kSPrintf( vcTempBuffer, "Data: X = %d, Y = %d, Button = %X", 
                     pstMouseEvent->stPoint.iX, pstMouseEvent->stPoint.iY,
                     pstMouseEvent->bButtonStatus );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );

            //------------------------------------------------------------------
            // 마우스 눌림 또는 떨어짐 이벤트이면 버튼의 색깔을 다시 그림
            //------------------------------------------------------------------
            // 마우스 왼쪽 버튼이 눌렸을 때 버튼 처리
            if( stReceivedEvent.qwType == EVENT_MOUSE_LBUTTONDOWN )
            {
                // 버튼 영역에 마우스 왼쪽 버튼이 눌렸는지를 판단
                if( kIsInRectangle( &stButtonArea, pstMouseEvent->stPoint.iX, 
                                    pstMouseEvent->stPoint.iY ) == TRUE )
                {
                    // 버튼의 배경을 밝은 녹색으로 변경하여 눌렸음을 표시
                    kDrawButton( qwWindowID, &stButtonArea, 
                                 RGB( 79, 204, 11 ), "User Message Send Button(Down)",
                                 RGB( 255, 255, 255 ) );
                    kUpdateScreenByID( qwWindowID );
                    
                    //----------------------------------------------------------
                    // 다른 윈도우로 유저 이벤트를 전송
                    //----------------------------------------------------------
                    // 생성된 모든 윈도우를 찾아서 이벤트를 전송
                    stSendEvent.qwType = EVENT_USER_TESTMESSAGE;
                    stSendEvent.vqwData[ 0 ] = qwWindowID;
                    stSendEvent.vqwData[ 1 ] = 0x1234;
                    stSendEvent.vqwData[ 2 ] = 0x5678;
                    
                    // 생성된 윈도우의 수 만큼 루프를 수행하면서 이벤트를 전송
                    for( i = 0 ; i < s_iWindowCount ; i++ )
                    {
                        // 윈도우 제목으로 윈도우를 검색
                        kSPrintf( vcTempBuffer, "Hello World Window %d", i );
                        qwFindWindowID = kFindWindowByTitle( vcTempBuffer );
                        // 윈도우가 존재하며 윈도우 자신이 아닌 경우는 이벤트를 전송
                        if( ( qwFindWindowID != WINDOW_INVALIDID ) &&
                            ( qwFindWindowID != qwWindowID ) )
                        {
                            // 윈도우로 이벤트 전송
                            kSendEventToWindow( qwFindWindowID, &stSendEvent );
                        }
                    }
                }
            }
            // 마우스 왼쪽 버튼이 떨어졌을 때 버튼 처리
            else if( stReceivedEvent.qwType == EVENT_MOUSE_LBUTTONUP )
            {
                // 버튼의 배경을 흰색으로 변경하여 눌리지 않았음을 표시
                kDrawButton( qwWindowID, &stButtonArea, 
                    WINDOW_COLOR_BACKGROUND, "User Message Send Button(Up)", 
                    RGB( 0, 0, 0 ) );
            }            
            break;

            // 키 이벤트 처리
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // 여기에 키보드 이벤트 처리 코드 넣기
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );

            // 키 이벤트의 타입을 출력
            kSPrintf( vcTempBuffer, "Key Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // 키 데이터를 출력
            kSPrintf( vcTempBuffer, "Data: Key = %c, Flag = %X", 
                    pstKeyEvent->bASCIICode, pstKeyEvent->bFlags );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            break;

            // 윈도우 이벤트 처리
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            // 여기에 윈도우 이벤트 처리 코드 넣기
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );

            // 윈도우 이벤트의 타입을 출력
            kSPrintf( vcTempBuffer, "Window Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // 윈도우 데이터를 출력
            kSPrintf( vcTempBuffer, "Data: X1 = %d, Y1 = %d, X2 = %d, Y2 = %d", 
                    pstWindowEvent->stArea.iX1, pstWindowEvent->stArea.iY1, 
                    pstWindowEvent->stArea.iX2, pstWindowEvent->stArea.iY2 );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            //------------------------------------------------------------------
            // 윈도우 닫기 이벤트이면 윈도우를 삭제하고 루프를 빠져나가 태스크를 종료
            //------------------------------------------------------------------
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // 윈도우 삭제
                kDeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // 그 외 정보
        default:
            // 여기에 알 수 없는 이벤트 처리 코드 넣기
            // 알 수 없는 이벤트를 출력
            kSPrintf( vcTempBuffer, "Unknown Event: 0x%X", stReceivedEvent.qwType );
            kDrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND,
                       vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // 데이터를 출력
            kSPrintf( vcTempBuffer, "Data0 = 0x%Q, Data1 = 0x%Q, Data2 = 0x%Q",
                      stReceivedEvent.vqwData[ 0 ], stReceivedEvent.vqwData[ 1 ], 
                      stReceivedEvent.vqwData[ 2 ] );
            kDrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            break;
        }

        // 윈도우를 화면에 업데이트
        kShowWindow( qwWindowID, TRUE );
    }
}
