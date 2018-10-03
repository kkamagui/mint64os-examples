/**
 *  file    Main.c
 *  date    2010/03/30
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C 언어로 작성된 응용프로그램의 엔트리 포인트 파일
 */

#include "MINTOSLibrary.h"
#include "Main.h"
#include "HangulInput.h"

/**
 *  응용프로그램의 C 언어 엔트리 포인트
 */
int Main( char* pcArgument )
{
    QWORD qwWindowID;
    int iX;
    int iY;
    int iWidth;
    int iHeight;
    EVENT stReceivedEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;
    RECT stScreenArea;
    BUFFERMANAGER stBufferManager;
    BOOL bHangulMode;
    
    //--------------------------------------------------------------------------
    // 그래픽 모드 판단
    //--------------------------------------------------------------------------
    // MINT64 OS가 그래픽 모드로 시작했는지 확인
    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS가 그래픽 모드로 시작하지 않았다면 실패
        printf( "This task can run only GUI mode~!!!\n" );
        return -1;
    }
    
    //--------------------------------------------------------------------------
    // 윈도우와 입력 위치를 나타내는 커서를 생성
    //--------------------------------------------------------------------------
    // 윈도우를 화면 가운데에 가로 X 세로 크기를 60문자 X 40 픽셀로 생성
    GetScreenArea( &stScreenArea );
    iWidth = MAXOUTPUTLENGTH * FONT_ENGLISHWIDTH + 5;
    iHeight = 40;
    iX = ( GetRectangleWidth( &stScreenArea ) - iWidth ) / 2;
    iY = ( GetRectangleHeight( &stScreenArea ) - iHeight ) / 2;
    qwWindowID = CreateWindow(iX, iY, iWidth, iHeight, WINDOW_FLAGS_DEFAULT,
                              "한 줄 메모장(한/영 전환은 Alt 키)" );
    
    // 버퍼의 정보를 초기화하고 영문 입력 모드로 설정
    memset( &stBufferManager, 0, sizeof( stBufferManager ) );
    bHangulMode = FALSE;

    // 커서를 메모 입력 영역 앞쪽에 세로로 길게 출력하고 윈도우를 다시 표시
    DrawRect( qwWindowID, 3, 4 + WINDOW_TITLEBAR_HEIGHT, 5,
        3 + WINDOW_TITLEBAR_HEIGHT + FONT_ENGLISHHEIGHT, RGB( 0, 250, 0 ), TRUE );
    ShowWindow( qwWindowID, TRUE );

    //--------------------------------------------------------------------------
    // GUI 태스크의 이벤트 처리 루프
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // 이벤트 큐에서 이벤트를 수신
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 10 );
            continue;
        }
        
        // 수신된 이벤트를 타입에 따라 나누어 처리
        switch( stReceivedEvent.qwType )
        {
            // 키 눌림 처리
        case EVENT_KEY_DOWN:
            // 키 이벤트를 추출
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );

            // 입력된 키로 한글을 조합하거나 영문을 출력
            switch( pstKeyEvent->bASCIICode )
            {
            //------------------------------------------------------------------
            // 한/영 모드 전환 처리
            //------------------------------------------------------------------
            case KEY_LALT:
                // 한글 입력 모드 중에 Alt 키가 눌러지면 한글 조합을 종료
                if( bHangulMode == TRUE )
                {
                    // 키 입력 버퍼를 초기화
                    stBufferManager.iInputBufferLength = 0;

                    if( ( stBufferManager.vcOutputBufferForProcessing[ 0 ] != '\0' ) &&
                        ( stBufferManager.iOutputBufferLength + 2 < MAXOUTPUTLENGTH ) )
                    {
                        // 조합 중인 한글을 윈도우 화면에 출력하는 버퍼로 복사
                        memcpy( stBufferManager.vcOutputBuffer +
                                stBufferManager.iOutputBufferLength,
                                stBufferManager.vcOutputBufferForProcessing, 2 );
                        stBufferManager.iOutputBufferLength += 2;

                        // 조합 중인 한글을 저장하는 버퍼를 초기화
                        stBufferManager.vcOutputBufferForProcessing[ 0 ] = '\0';
                    }
                }
                // 영문 입력 모드 중에 Alt 키가 눌러지면 한글 조합용 버퍼를 초기화
                else
                {
                    stBufferManager.iInputBufferLength = 0;
                    stBufferManager.vcOutputBufferForComplete[ 0 ] = '\0';
                    stBufferManager.vcOutputBufferForProcessing[ 0 ] = '\0';
                }
                bHangulMode = TRUE - bHangulMode;
                break;

            //------------------------------------------------------------------
            // 백스페이스 키 처리
            //------------------------------------------------------------------
            case KEY_BACKSPACE:
                // 한글을 조합하는 중이면 입력 버퍼의 내용을 삭제하고 남은
                // 키 입력 버퍼의 내용으로 한글을 조합
                if( ( bHangulMode == TRUE ) &&
                    ( stBufferManager.iInputBufferLength > 0 ) )
                {
                    // 키 입력 버퍼의 내용을 하나 제거하고 한글을 다시 조합
                    stBufferManager.iInputBufferLength--;
                    ComposeHangul( stBufferManager.vcInputBuffer,
                            &stBufferManager.iInputBufferLength,
                            stBufferManager.vcOutputBufferForProcessing,
                            stBufferManager.vcOutputBufferForComplete );
                }
                // 한글 조합 중이 아니면 윈도우 화면에 출력하는 버퍼의 내용을 삭제
                else
                {
                    if( stBufferManager.iOutputBufferLength > 0 )
                    {
                        // 화면 출력 버퍼에 들어있는 내용이 2바이트 이상이고 버퍼에
                        // 저장된 값에 최상위 비트가 켜져 있으면 한글로 간주하고
                        // 마지막 2바이트를 모두 삭제
                        if( ( stBufferManager.iOutputBufferLength >= 2 ) &&
                            ( stBufferManager.vcOutputBuffer[
                                  stBufferManager.iOutputBufferLength - 1 ] & 0x80 ) )
                        {
                            stBufferManager.iOutputBufferLength -= 2;
                            memset( stBufferManager.vcOutputBuffer +
                                    stBufferManager.iOutputBufferLength, 0, 2 );
                        }
                        // 한글이 아니면 마지막 1바이트만 삭제
                        else
                        {
                            stBufferManager.iOutputBufferLength--;
                            stBufferManager.vcOutputBuffer[
                                stBufferManager.iOutputBufferLength ] = '\0';
                        }
                    }
                }
                break;

            //------------------------------------------------------------------
            // 나머지 키들은 현재 입력 모드에 따라 한글을 조합하거나
            // 윈도우 화면에 출력하는 버퍼로 직접 삽입
            //------------------------------------------------------------------
            default:
                // 특수 키들은 모두 무시
                if( pstKeyEvent->bASCIICode & 0x80 )
                {
                    break;
                }

                // 한글 조합 중이면 한글 조합 처리(한글을 화면 출력 버퍼에 저장할
                // 공간이 충분한지도 확인)
                if( ( bHangulMode == TRUE ) &&
                    ( stBufferManager.iOutputBufferLength + 2 <= MAXOUTPUTLENGTH ) )
                {
                    // 자/모음이 시프트와 조합되는 경우를 대비하여 쌍자음이나
                    // 쌍모음을 제외한 나머지는 소문자로 변환
                    ConvertJaumMoumToLowerCharactor( &pstKeyEvent->bASCIICode );

                    // 입력 버퍼에 키 입력 값을 채우고 데이터의 길이를 증가
                    stBufferManager.vcInputBuffer[
                        stBufferManager.iInputBufferLength ] = pstKeyEvent->bASCIICode;
                    stBufferManager.iInputBufferLength++;

                    // 한글 조합에 필요한 버퍼를 넘겨줘서 한글을 조합
                    if( ComposeHangul( stBufferManager.vcInputBuffer,
                        &stBufferManager.iInputBufferLength,
                        stBufferManager.vcOutputBufferForProcessing,
                        stBufferManager.vcOutputBufferForComplete) == TRUE )
                    {
                        // 조합이 완료된 버퍼에 값이 있는가 확인하여 윈도우 화면에
                        // 출력할 버퍼로 복사
                        if( stBufferManager.vcOutputBufferForComplete[ 0 ] != '\0' )
                        {
                           memcpy( stBufferManager.vcOutputBuffer +
                                   stBufferManager.iOutputBufferLength,
                                   stBufferManager.vcOutputBufferForComplete, 2 );
                           stBufferManager.iOutputBufferLength += 2;

                           // 조합된 한글을 복사한 뒤에 현재 조합 중인 한글을 출력할
                           // 공간이 없다면 키 입력 버퍼와 조합 중인 한글 버퍼를 모두 초기화
                           if( stBufferManager.iOutputBufferLength + 2 > MAXOUTPUTLENGTH )
                           {
                               stBufferManager.iInputBufferLength = 0;
                               stBufferManager.vcOutputBufferForProcessing[ 0 ] = '\0';
                           }
                        }
                    }
                    // 조합에 실패하면 입력 버퍼에 마지막으로 입력된 값이 한글 자/모음이
                    // 아니므로 한글 조합이 완료된 버퍼의 값과 입력 버퍼에 있는 값을 모두
                    // 출력 버퍼로 복사
                    else
                    {
                        // 조합이 완료된 버퍼에 값이 있는가 확인하여 윈도우 화면에
                        // 출력할 버퍼로 복사
                        if( stBufferManager.vcOutputBufferForComplete[ 0 ] != '\0' )
                        {
                            // 완성된 한글을 출력 버퍼로 복사
                            memcpy( stBufferManager.vcOutputBuffer +
                                    stBufferManager.iOutputBufferLength,
                                    stBufferManager.vcOutputBufferForComplete, 2 );
                            stBufferManager.iOutputBufferLength += 2;
                        }

                        // 윈도우 화면에 출력하는 버퍼의 공간이 충분하면 키 입력 버퍼에
                        // 마지막으로 입력된 한글 자/모가 아닌 값을 복사
                        if( stBufferManager.iOutputBufferLength < MAXOUTPUTLENGTH )
                        {
                            stBufferManager.vcOutputBuffer[
                                stBufferManager.iOutputBufferLength ] =
                                        stBufferManager.vcInputBuffer[ 0 ];
                            stBufferManager.iOutputBufferLength++;
                        }

                        // 키 입력 버퍼를 비움
                        stBufferManager.iInputBufferLength = 0;
                    }
                }
                // 한글 입력 모드가 아닌 경우
                else if( ( bHangulMode == FALSE ) &&
                         ( stBufferManager.iOutputBufferLength + 1 <= MAXOUTPUTLENGTH ) )
                {
                    // 키 입력을 그대로 윈도우 화면에 출력하는 버퍼로 저장
                    stBufferManager.vcOutputBuffer[
                        stBufferManager.iOutputBufferLength ] = pstKeyEvent->bASCIICode;
                    stBufferManager.iOutputBufferLength++;
                }
                break;
            }

            //------------------------------------------------------------------
            // 화면 출력 버퍼에 있는 문자열과 조합 중인 한글을 출력하고 커서의
            // 위치를 갱신
            //------------------------------------------------------------------
            // 화면 출력 버퍼에 있는 문자열을 전부 출력
            DrawText( qwWindowID, 2, WINDOW_TITLEBAR_HEIGHT + 4, RGB( 0, 0, 0 ),
                      RGB( 255, 255, 255 ), stBufferManager.vcOutputBuffer,
                      MAXOUTPUTLENGTH );
            // 현재 조합 중인 한글이 있다면 화면 출력 버퍼의 내용이 출력된
            // 다음 위치에 조합중인 한글을 출력
            if( stBufferManager.vcOutputBufferForProcessing[ 0 ] != '\0' )
            {
                DrawText( qwWindowID, 2 + stBufferManager.iOutputBufferLength *
                          FONT_ENGLISHWIDTH, WINDOW_TITLEBAR_HEIGHT + 4,
                          RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
                          stBufferManager.vcOutputBufferForProcessing, 2 );
            }

            // 커서를 세로로 길게 출력
            DrawRect( qwWindowID, 3 + stBufferManager.iOutputBufferLength *
                      FONT_ENGLISHWIDTH, 4 + WINDOW_TITLEBAR_HEIGHT,
                      5 + stBufferManager.iOutputBufferLength * FONT_ENGLISHWIDTH,
                      3 + WINDOW_TITLEBAR_HEIGHT + FONT_ENGLISHHEIGHT,
                      RGB( 0, 250, 0 ), TRUE );

            ShowWindow( qwWindowID, TRUE );
            break;

            // 윈도우 닫기 버튼 처리
        case EVENT_WINDOW_CLOSE:
            // 윈도우를 삭제하고 메모리를 해제
            DeleteWindow( qwWindowID );
            return 0;
            break;
            
            // 그 외 정보
        default:
            break;
        }
    }
    
    return 0;
}

/**
 *  한글 자음과 모음 범위에서 쌍자음과 쌍모음을 제외한 나머지는 모두 소문자로 변환
 */
void ConvertJaumMoumToLowerCharactor( BYTE* pbInput )
{
    if( ( *pbInput < 'A' ) || ( *pbInput > 'Z' ) )
    {
        return ;
    }

    // 쌍자음 또는 쌍모음 여부 판별
    switch( *pbInput )
    {
    case 'Q':   // 'ㅃ'
    case 'W':   // 'ㅉ'
    case 'E':   // 'ㄸ'
    case 'R':   // 'ㄲ'
    case 'T':   // 'ㅆ'
    case 'O':   // 'ㅒ'
    case 'P':   // 'ㅖ'
        return ;
        break;
    }

    // 소문자로 변환
    *pbInput = TOLOWER( *pbInput );
}

