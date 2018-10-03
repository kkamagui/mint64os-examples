/**
 *  file    GUITask.c
 *  date    2009/10/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI 태스크에 관련된 함수를 정의한 소스 파일
 */
#include "GUITask.h"
#include "Window.h"
#include "MultiProcessor.h"
#include "Font.h"
#include "Console.h"
#include "Task.h"
#include "ConsoleShell.h"

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

//------------------------------------------------------------------------------
//  시스템 모니터 태스크
//------------------------------------------------------------------------------
/**
 *  시스템의 상태를 감시하여 화면에 표시하는 태스크
 */
void kSystemMonitorTask( void )
{
    QWORD qwWindowID;
    int i;
    int iWindowWidth;
    int iProcessorCount;
    DWORD vdwLastCPULoad[ MAXPROCESSORCOUNT ];
    int viLastTaskCount[ MAXPROCESSORCOUNT ];
    QWORD qwLastTickCount;
    EVENT stReceivedEvent;
    WINDOWEVENT* pstWindowEvent;
    BOOL bChanged;
    RECT stScreenArea;
    QWORD qwLastDynamicMemoryUsedSize;
    QWORD qwDynamicMemoryUsedSize;
    QWORD qwTemp;
    
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
    // 화면 영역의 크기를 반환
    kGetScreenArea( &stScreenArea );
    
    // 현재 프로세서의 개수로 윈도우의 너비를 계산
    iProcessorCount = kGetProcessorCount();
    iWindowWidth = iProcessorCount * ( SYSTEMMONITOR_PROCESSOR_WIDTH + 
            SYSTEMMONITOR_PROCESSOR_MARGIN ) + SYSTEMMONITOR_PROCESSOR_MARGIN;
    
    // 윈도우를 화면 가운데에 생성한 뒤 화면에 표시하지 않음. 프로세서 정보와 메모리 정보를 
    // 표시하는 영역을 그린 뒤 화면에 표시
    qwWindowID = kCreateWindow( ( stScreenArea.iX2 - iWindowWidth ) / 2, 
        ( stScreenArea.iY2 - SYSTEMMONITOR_WINDOW_HEIGHT ) / 2, 
        iWindowWidth, SYSTEMMONITOR_WINDOW_HEIGHT, WINDOW_FLAGS_DEFAULT & 
        ~WINDOW_FLAGS_SHOW, "System Monitor" );
    // 윈도우를 생성하지 못했으면 실패
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }

    // 프로세서 정보를 표시하는 영역을 3픽셀 두께로 표시하고 문자열을 출력
    kDrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 15, iWindowWidth - 5, 
            WINDOW_TITLEBAR_HEIGHT + 15, RGB( 0, 0, 0 ) );
    kDrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 16, iWindowWidth - 5, 
            WINDOW_TITLEBAR_HEIGHT + 16, RGB( 0, 0, 0 ) );
    kDrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 17, iWindowWidth - 5, 
            WINDOW_TITLEBAR_HEIGHT + 17, RGB( 0, 0, 0 ) );
    kDrawText( qwWindowID, 9, WINDOW_TITLEBAR_HEIGHT + 8, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, "Processor Information", 21 );


    // 메모리 정보를 표시하는 영역을 3픽셀 두께로 표시하고 문자열을 출력
    kDrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 50, 
            iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 50, 
            RGB( 0, 0, 0 ) );
    kDrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 51, 
                iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 51, 
                RGB( 0, 0, 0 ) );
    kDrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 52, 
                iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 52, 
                RGB( 0, 0, 0 ) );
    kDrawText( qwWindowID, 9, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 43, 
            RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, "Memory Information", 18 );
    // 윈도우를 화면에 표시
    kShowWindow( qwWindowID, TRUE );
    
    // 루프를 돌면서 시스템 정보를 감시하여 화면에 표시
    qwLastTickCount = 0;
    
    // 마지막으로 측정한 프로세서의 부하와 태스크 수, 그리고 메모리 사용량은 모두 0으로 설정
    kMemSet( vdwLastCPULoad, 0, sizeof( vdwLastCPULoad ) );
    kMemSet( viLastTaskCount, 0, sizeof( viLastTaskCount ) );
    qwLastDynamicMemoryUsedSize = 0;
    
    //--------------------------------------------------------------------------
    // GUI 태스크의 이벤트 처리 루프
    //--------------------------------------------------------------------------
    while( 1 )
    {
        //----------------------------------------------------------------------
        // 이벤트 큐의 이벤트 처리
        //----------------------------------------------------------------------
        // 이벤트 큐에서 이벤트를 수신
        if( kReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == TRUE )
        {
            // 수신된 이벤트를 타입에 따라 나누어 처리
            switch( stReceivedEvent.qwType )
            {
                // 윈도우 이벤트 처리
            case EVENT_WINDOW_CLOSE:
                //--------------------------------------------------------------
                // 윈도우 닫기 이벤트이면 윈도우를 삭제하고 루프를 빠져나가 태스크를 종료
                //--------------------------------------------------------------
                // 윈도우 삭제
                kDeleteWindow( qwWindowID );
                return ;
                break;
                
                // 그 외 정보
            default:
                break;
            }
        }
        
        // 0.5초마다 한번씩 시스템 상태를 확인
        if( ( kGetTickCount() - qwLastTickCount ) < 500 )
        {
            kSleep( 1 );
            continue;
        }

        // 마지막으로 측정한 시간을 최신으로 업데이트
        qwLastTickCount = kGetTickCount();

        //----------------------------------------------------------------------
        // 프로세서 정보 출력
        //----------------------------------------------------------------------
        // 프로세서 수만큼 부하와 태스크 수를 확인하여 달라진 점이 있으면 화면에 업데이트
        for( i = 0 ; i < iProcessorCount ; i++ )
        {
            bChanged = FALSE;
            
            // 프로세서 부하 검사
            if( vdwLastCPULoad[ i ] != kGetProcessorLoad( i ) )
            {
                vdwLastCPULoad [ i ] = kGetProcessorLoad( i );
                bChanged = TRUE;
            }
            // 태스크 수 검사
            else if( viLastTaskCount[ i ] != kGetTaskCount( i ) )
            {
                viLastTaskCount[ i ] = kGetTaskCount( i );
                bChanged = TRUE;
            }
            
            // 이전과 비교해서 변경 사항이 있으면 화면에 업데이트
            if( bChanged == TRUE )
            {
                // 화면에 현재 프로세서의 부하를 표시 
                kDrawProcessorInformation( qwWindowID, i * SYSTEMMONITOR_PROCESSOR_WIDTH + 
                    ( i + 1 ) * SYSTEMMONITOR_PROCESSOR_MARGIN, WINDOW_TITLEBAR_HEIGHT + 28,
                    i );
            }
        }
        
        //----------------------------------------------------------------------
        // 동적 메모리 정보 출력
        //----------------------------------------------------------------------
        // 동적 메모리의 정보를 반환
        kGetDynamicMemoryInformation( &qwTemp, &qwTemp, &qwTemp, 
                &qwDynamicMemoryUsedSize );
        
        // 현재 동적 할당 메모리 사용량이 이전과 다르다면 메모리 정보를 출력
        if( qwDynamicMemoryUsedSize != qwLastDynamicMemoryUsedSize )
        {
            qwLastDynamicMemoryUsedSize = qwDynamicMemoryUsedSize;
            
            // 메모리 정보를 출력
            kDrawMemoryInformation( qwWindowID, WINDOW_TITLEBAR_HEIGHT + 
                    SYSTEMMONITOR_PROCESSOR_HEIGHT + 60, iWindowWidth );
        }
    }
}

/**
 *  개별 프로세서의 정보를 화면에 표시
 */
static void kDrawProcessorInformation( QWORD qwWindowID, int iX, int iY, 
        BYTE bAPICID )
{
    char vcBuffer[ 100 ];
    RECT stArea;
    QWORD qwProcessorLoad;
    QWORD iUsageBarHeight;
    int iMiddleX;
    
    // 프로세서 ID를 표시
    kSPrintf( vcBuffer, "Processor ID: %d", bAPICID );
    kDrawText( qwWindowID, iX + 10, iY, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, 
            vcBuffer, kStrLen( vcBuffer ) );
    
    // 프로세서의 태스크 개수를 표시
    kSPrintf( vcBuffer, "Task Count: %d   ", kGetTaskCount( bAPICID ) );
    kDrawText( qwWindowID, iX + 10, iY + 18, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND,
            vcBuffer, kStrLen( vcBuffer ) );

    //--------------------------------------------------------------------------
    // 프로세서 부하를 나타내는 막대를 표시
    //--------------------------------------------------------------------------
    // 프로세서 부하를 표시
    qwProcessorLoad = kGetProcessorLoad( bAPICID );
    if( qwProcessorLoad > 100 )
    {
        qwProcessorLoad = 100;
    }
    
    // 부하를 표시하는 막대의 전체에 테두리를 표시
    kDrawRect( qwWindowID, iX, iY + 36, iX + SYSTEMMONITOR_PROCESSOR_WIDTH, 
            iY + SYSTEMMONITOR_PROCESSOR_HEIGHT, RGB( 0, 0, 0 ), FALSE );

    // 프로세서 사용량을 나타내는 막대의 길이, ( 막대 전체의 길이 * 프로세서 부하 / 100 ) 
    iUsageBarHeight = ( SYSTEMMONITOR_PROCESSOR_HEIGHT - 40 ) * qwProcessorLoad / 100;

    // 부하를 표시하는 영역의 막대 내부를 표시
    // 채워진 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠 
    kDrawRect( qwWindowID, iX + 2,
        iY + ( SYSTEMMONITOR_PROCESSOR_HEIGHT - iUsageBarHeight ) - 2, 
        iX + SYSTEMMONITOR_PROCESSOR_WIDTH - 2, 
        iY + SYSTEMMONITOR_PROCESSOR_HEIGHT - 2, SYSTEMMONITOR_BAR_COLOR, TRUE );
    // 빈 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠
    kDrawRect( qwWindowID, iX + 2, iY + 38, iX + SYSTEMMONITOR_PROCESSOR_WIDTH - 2,
            iY + ( SYSTEMMONITOR_PROCESSOR_HEIGHT - iUsageBarHeight ) - 1, 
            WINDOW_COLOR_BACKGROUND, TRUE );
    
    // 프로세서의 부하를 표시, 막대의 가운데에 부하가 표시되도록 함
    kSPrintf( vcBuffer, "Usage: %d%%", qwProcessorLoad );
    iMiddleX = ( SYSTEMMONITOR_PROCESSOR_WIDTH - 
            ( kStrLen( vcBuffer ) * FONT_ENGLISHWIDTH ) ) / 2;
    kDrawText( qwWindowID, iX + iMiddleX, iY + 80, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, vcBuffer, kStrLen( vcBuffer ) );
    
    // 프로세서 정보가 표시된 영역만 다시 화면에 업데이트
    kSetRectangleData( iX, iY, iX + SYSTEMMONITOR_PROCESSOR_WIDTH, 
            iY + SYSTEMMONITOR_PROCESSOR_HEIGHT, &stArea );    
    kUpdateScreenByWindowArea( qwWindowID, &stArea );
}

/**
 *  메모리 정보를 출력
 */
static void kDrawMemoryInformation( QWORD qwWindowID, int iY, int iWindowWidth )
{
    char vcBuffer[ 100 ];
    QWORD qwTotalRAMKbyteSize;
    QWORD qwDynamicMemoryStartAddress;
    QWORD qwDynamicMemoryUsedSize;
    QWORD qwUsedPercent;
    QWORD qwTemp;
    int iUsageBarWidth;
    RECT stArea;
    int iMiddleX;
    
    // Mbyte 단위의 메모리를 Kbyte 단위로 변환
    qwTotalRAMKbyteSize = kGetTotalRAMSize() * 1024;
    
    // 메모리 정보를 표시
    kSPrintf( vcBuffer, "Total Size: %d KB        ", qwTotalRAMKbyteSize );
    kDrawText( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 10, iY + 3, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, vcBuffer, kStrLen( vcBuffer ) );
    
    // 동적 메모리의 정보를 반환
    kGetDynamicMemoryInformation( &qwDynamicMemoryStartAddress, &qwTemp, 
            &qwTemp, &qwDynamicMemoryUsedSize );
    
    kSPrintf( vcBuffer, "Used Size: %d KB        ", ( qwDynamicMemoryUsedSize + 
            qwDynamicMemoryStartAddress ) / 1024 );
    kDrawText( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 10, iY + 21, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, vcBuffer, kStrLen( vcBuffer ) );
    
    //--------------------------------------------------------------------------
    // 메모리 사용량을 나타내는 막대를 표시
    //--------------------------------------------------------------------------
    // 메모리 사용량을 표시하는 막대의 전체에 테두리를 표시
    kDrawRect( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN, iY + 40,
            iWindowWidth - SYSTEMMONITOR_PROCESSOR_MARGIN, 
            iY + SYSTEMMONITOR_MEMORY_HEIGHT - 32, RGB( 0, 0, 0 ), FALSE );
    // 메모리 사용량(%)을 계산
    qwUsedPercent = ( qwDynamicMemoryStartAddress + qwDynamicMemoryUsedSize ) * 
        100 / 1024 / qwTotalRAMKbyteSize;
    if( qwUsedPercent > 100 )
    {
        qwUsedPercent = 100;
    }
    
    // 메모리 사용량을 나타내는 막대의 길이, ( 막대 전체의 길이 * 메모리 사용량 / 100 )     
    iUsageBarWidth = ( iWindowWidth - 2 * SYSTEMMONITOR_PROCESSOR_MARGIN ) * 
        qwUsedPercent / 100;
    
    // 메모리 사용량을 표시하는 영역의 막대 내부를 표시
    // 색칠된 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠 
    kDrawRect( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 2, iY + 42, 
            SYSTEMMONITOR_PROCESSOR_MARGIN + 2 + iUsageBarWidth, 
            iY + SYSTEMMONITOR_MEMORY_HEIGHT - 34, SYSTEMMONITOR_BAR_COLOR, TRUE );  
    // 빈 막대를 표시, 테두리와 2픽셀 정도 여유 공간을 둠
    kDrawRect( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 2 + iUsageBarWidth, 
        iY + 42, iWindowWidth - SYSTEMMONITOR_PROCESSOR_MARGIN - 2,
        iY + SYSTEMMONITOR_MEMORY_HEIGHT - 34, WINDOW_COLOR_BACKGROUND, TRUE );    
    
    // 사용량을 나타내는 텍스트 표시, 막대의 가운데에 사용량이 표시되도록 함
    kSPrintf( vcBuffer, "Usage: %d%%", qwUsedPercent );
    iMiddleX = ( iWindowWidth - ( kStrLen( vcBuffer ) * FONT_ENGLISHWIDTH ) ) / 2;
    kDrawText( qwWindowID, iMiddleX, iY + 45, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, 
            vcBuffer, kStrLen( vcBuffer ) );
    
    // 메모리 정보가 표시된 영역만 화면에 다시 업데이트
    kSetRectangleData(0, iY, iWindowWidth, iY + SYSTEMMONITOR_MEMORY_HEIGHT, &stArea );
    kUpdateScreenByWindowArea( qwWindowID, &stArea );
}

//------------------------------------------------------------------------------
//  GUI 버전의 콘솔 셸 태스크
//------------------------------------------------------------------------------
// 이전 화면 버퍼의 값을 보관하는 영역
static CHARACTER gs_vstPreviousScreenBuffer[ CONSOLE_WIDTH * CONSOLE_HEIGHT ];

/**
 *  GUI 버전의 콘솔 셸 태스크
 */
void kGUIConsoleShellTask( void )
{
    static QWORD s_qwWindowID = WINDOW_INVALIDID;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    KEYEVENT* pstKeyEvent;
    RECT stScreenArea;
    KEYDATA stKeyData;
    TCB* pstConsoleShellTask;
    QWORD qwConsoleShellTaskID;

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
    
    // GUI 콘솔 셸 윈도우가 존재하면 생성된 윈도우를 최상위로 만들고 태스크 종료
    if( s_qwWindowID != WINDOW_INVALIDID )
    {
        kMoveWindowToTop( s_qwWindowID );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // 윈도우를 생성
    //--------------------------------------------------------------------------
    // 전체 화면 영역의 크기를 반환
    kGetScreenArea( &stScreenArea );
    
    // 윈도우의 크기 설정, 화면 버퍼에 들어가는 문자의 최대 너비와 높이를 고려해서 계산
    iWindowWidth = CONSOLE_WIDTH * FONT_ENGLISHWIDTH + 4;
    iWindowHeight = CONSOLE_HEIGHT * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT + 2;
    
    // 윈도우 생성 함수 호출, 화면 가운데에 생성
    s_qwWindowID = kCreateWindow( ( stScreenArea.iX2 - iWindowWidth ) / 2, 
        ( stScreenArea.iY2 - iWindowHeight ) / 2, iWindowWidth, iWindowHeight, 
        WINDOW_FLAGS_DEFAULT, "Console Shell for GUI" );
    // 윈도우를 생성하지 못했으면 실패
    if( s_qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }

    // 셸 커맨드를 처리하는 콘솔 셸 태스크를 생성
    kSetConsoleShellExitFlag( FALSE );
    pstConsoleShellTask = kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, 
        ( QWORD ) kStartConsoleShell, TASK_LOADBALANCINGID );
    if( pstConsoleShellTask == NULL )
    {
        // 콘솔 셸 태스크 생성에 실패하면 윈도우를 삭제하고 종료
        kDeleteWindow( s_qwWindowID );
        return ;
    }
    // 콘솔 셸 태스크의 ID를 저장
    qwConsoleShellTaskID = pstConsoleShellTask->stLink.qwID;

    // 이전 화면 버퍼를 초기화
    kMemSet( gs_vstPreviousScreenBuffer, 0xFF, sizeof( gs_vstPreviousScreenBuffer ) );
    
    //--------------------------------------------------------------------------
    // GUI 태스크의 이벤트 처리 루프
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // 화면 버퍼의 변경된 내용을 윈도우에 업데이트
        kProcessConsoleBuffer( s_qwWindowID );
        
        // 이벤트 큐에서 이벤트를 수신
        if( kReceiveEventFromWindowQueue( s_qwWindowID, &stReceivedEvent ) == FALSE )
        {
            kSleep( 0 );
            continue;
        }
        
        // 수신된 이벤트를 타입에 따라 나누어 처리
        switch( stReceivedEvent.qwType )
        {
            // 키 이벤트 처리
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // 여기에 키보드 이벤트 처리 코드 넣기
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            stKeyData.bASCIICode = pstKeyEvent->bASCIICode;
            stKeyData.bFlags = pstKeyEvent->bFlags;
            stKeyData.bScanCode = pstKeyEvent->bScanCode;

            // 키를 그래픽 모드용 키 큐로 삽입하여 셸 태스크의 입력으로 전달
            kPutKeyToGUIKeyQueue( &stKeyData );
            break;

            // 윈도우 이벤트 처리
        case EVENT_WINDOW_CLOSE:
            // 생성한 셸 태스크가 종료되도록 종료 플래그를 설정하고 종료될 때까지 대기
            kSetConsoleShellExitFlag( TRUE );
            while( kIsTaskExist( qwConsoleShellTaskID ) == TRUE )
            {
                kSleep( 1 );
            }
            
            // 윈도우를 삭제하고 윈도우 ID를 초기화
            kDeleteWindow( s_qwWindowID );
            s_qwWindowID = WINDOW_INVALIDID;            
            return ;
            
            break;
            
            // 그 외 정보
        default:
            // 여기에 알 수 없는 이벤트 처리 코드 넣기
            break;
        }
    }
}

/**
 *  화면 버퍼의 변경된 내용을 GUI 콘솔 셸 윈도우 화면에 업데이트
 */
static void kProcessConsoleBuffer( QWORD qwWindowID )
{
    int i;
    int j;
    CONSOLEMANAGER* pstManager;
    CHARACTER* pstScreenBuffer;
    CHARACTER* pstPreviousScreenBuffer;
    RECT stLineArea;
    BOOL bChanged;
    static QWORD s_qwLastTickCount = 0;
    BOOL bFullRedraw;
    
    // 콘솔을 관리하는 자료구조를 반환 받아 화면 버퍼의 어드레스를 저장하고 
    // 이전 화면 버퍼의 어드레스도 저장
    pstManager = kGetConsoleManager();
    pstScreenBuffer = pstManager->pstScreenBuffer;
    pstPreviousScreenBuffer = gs_vstPreviousScreenBuffer;
    
    // 화면을 전체를 업데이트 한 지 5초가 지났으면 무조건 화면 전체를 다시 그림
    if( kGetTickCount() - s_qwLastTickCount > 5000 )
    {
        s_qwLastTickCount = kGetTickCount();
        bFullRedraw = TRUE;
    }
    else
    {
        bFullRedraw = FALSE;
    }

    // 화면 버퍼의 높이만큼 반복
    for( j = 0 ; j < CONSOLE_HEIGHT ; j++ )
    {
        // 처음에는 변경되지 않은 것으로 플래그 설정
        bChanged = FALSE;
        
        // 현재 라인에 변화가 있는지 비교하여 변경 여부 플래그를 처리
        for( i = 0 ; i < CONSOLE_WIDTH ; i++ )
        {
            // 문자를 비교하여 다르거나 전체를 새로 그려야 하면 이전 화면 버퍼에
            // 업데이트하고 변경 여부 플래그를 설정
            if( ( pstScreenBuffer->bCharactor != pstPreviousScreenBuffer->bCharactor ) ||
                ( bFullRedraw == TRUE ) )
            {
                // 문자를 화면에 출력
                kDrawText( qwWindowID, i * FONT_ENGLISHWIDTH + 2, 
                           j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT, 
                           RGB( 0, 255, 0 ), RGB( 0, 0, 0 ), 
                           &( pstScreenBuffer->bCharactor ), 1);
                
                // 이전 화면 버퍼로 값을 옮겨 놓고 현재 라인에 이전과
                // 다른 데이터가 있음을 표시
                kMemCpy( pstPreviousScreenBuffer, pstScreenBuffer, sizeof( CHARACTER ) );
                bChanged = TRUE;
            }

            pstScreenBuffer++;
            pstPreviousScreenBuffer++;
        }
        
        // 현재 라인에서 변경된 데이터가 있다면 현재 라인만 화면에 업데이트
        if( bChanged == TRUE )
        {
            // 현재 라인의 영역을 계산
            kSetRectangleData( 2, j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT,
                5 + FONT_ENGLISHWIDTH * CONSOLE_WIDTH, ( j + 1 ) * FONT_ENGLISHHEIGHT + 
                WINDOW_TITLEBAR_HEIGHT - 1, &stLineArea );
            // 윈도우의 일부만 화면 업데이트
            kUpdateScreenByWindowArea( qwWindowID, &stLineArea );
        }
    }
}

