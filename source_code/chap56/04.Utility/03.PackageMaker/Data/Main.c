/**
 *  file    Main.c
 *  date    2010/03/23
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C 언어로 작성된 응용프로그램의 엔트리 포인트 파일
 */

#include "MINTOSLibrary.h"
#include "Main.h"

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
    TEXTINFO stInfo;
    int iMoveLine;
    EVENT stReceivedEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;
    DWORD dwFileOffset;
    RECT stScreenArea;
    
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
    // 파일의 내용을 파일 버퍼에 저장하고 라인 별 파일 오프셋 저장용 버퍼를 생성
    //--------------------------------------------------------------------------
    // 인자에 아무것도 전달되지 않으면 실패
    if( strlen( pcArgument ) == 0 )
    {
        printf( "ex) exec hanviwer.elf abc.txt\n" );
        return 0;
    }
        
    // 파일을 디렉터리에서 찾은 뒤에 파일의 크기만큼 메모리를 할당하여 파일을 저장
    // 라인 별 파일 오프셋을 저장할 버퍼도 같이 생성
    if( ReadFileToBuffer( pcArgument, &stInfo ) == FALSE )
    {
        printf( "%s file is not found\n", pcArgument );
        return 0;
    }

    //--------------------------------------------------------------------------
    // 윈도우와 라인 인덱스를 생성한 뒤 첫 번째 라인부터 화면에 출력
    //--------------------------------------------------------------------------
    // 윈도우를 화면 가운데에 가로 500 픽셀 X 세로 500 픽셀로 생성
    GetScreenArea( &stScreenArea );
    iWidth = 500;
    iHeight = 500;
    iX = ( GetRectangleWidth( &stScreenArea ) - iWidth ) / 2;
    iY = ( GetRectangleHeight( &stScreenArea ) - iHeight ) / 2;
    qwWindowID = CreateWindow(iX, iY, iWidth, iHeight, WINDOW_FLAGS_DEFAULT | 
                              WINDOW_FLAGS_RESIZABLE, "한글 뷰어(Hangul Viewer)" );
    
    // 라인 별 파일 오프셋을 계산하고 현재 화면에 출력하는 라인 인덱스를 0으로 설정
    CalculateFileOffsetOfLine( iWidth, iHeight, &stInfo );
    stInfo.iCurrentLineIndex = 0;
    
    // 현재 라인부터 화면 전체 크기만큼을 표시
    DrawTextBuffer( qwWindowID, &stInfo );
        
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
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            if( pstKeyEvent->bFlags & KEY_FLAGS_DOWN )
            {
                // 키 값에 따른 현재 라인 변경 값 설정
                switch( pstKeyEvent->bASCIICode )
                {
                    // Page Up 키와 Page Down 키는 화면에 출력 가능한 라인 단위로 이동
                case KEY_PAGEUP:
                    iMoveLine = -stInfo.iRowCount;
                    break;
                case KEY_PAGEDOWN:
                    iMoveLine = stInfo.iRowCount;
                    break;
                    // Up 키와 Down 키는 한 라인 단위로 이동
                case KEY_UP:
                    iMoveLine = -1;
                    break;
                case KEY_DOWN:
                    iMoveLine = 1;
                    break;
                    
                    // 기타 키이거나 현재 위치에서 움직일 필요가 없으면 종료
                default:
                    iMoveLine = 0;
                    break;
                }
                
                // 최대 최소 라인 범위를 벗어나면 현재 라인 인덱스를 조정
                if( stInfo.iCurrentLineIndex + iMoveLine < 0 )
                {
                    iMoveLine = -stInfo.iCurrentLineIndex;
                }
                else if( stInfo.iCurrentLineIndex + iMoveLine >= stInfo.iMaxLineCount )
                {
                    iMoveLine = stInfo.iMaxLineCount - stInfo.iCurrentLineIndex - 1;
                }
                                
                // 기타 키이거나 움직일 필요가 없으면 종료
                if( iMoveLine == 0 )
                {
                    break;
                }
                        
                // 현재 라인의 인덱스를 변경하고 화면에 출력
                stInfo.iCurrentLineIndex += iMoveLine;
                DrawTextBuffer( qwWindowID, &stInfo );
            }
            break;
            
            // 윈도우 크기 변경 처리
        case EVENT_WINDOW_RESIZE:
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );
            iWidth = GetRectangleWidth( &( pstWindowEvent->stArea ) );
            iHeight = GetRectangleHeight( &( pstWindowEvent->stArea ) );
            
            // 현재 라인이 있는 파일 오프셋을 저장
            dwFileOffset = stInfo.pdwFileOffsetOfLine[ stInfo.iCurrentLineIndex ];
            
            // 변경된 화면 크기로 다시 라인 수와 라인 당 문자 수, 그리고 파일 오프셋을 
            // 계산하고 이 값과 이전에 저장한 파일 오프셋을 이용하여 현재 라인을 다시 계산
            CalculateFileOffsetOfLine( iWidth, iHeight, &stInfo );
            stInfo.iCurrentLineIndex = dwFileOffset / stInfo.iColumnCount;
            
            // 현재 라인부터 화면에 출력
            DrawTextBuffer( qwWindowID, &stInfo );
            break;
            
            // 윈도우 닫기 버튼 처리
        case EVENT_WINDOW_CLOSE:
            // 윈도우를 삭제하고 메모리를 해제
            DeleteWindow( qwWindowID );
            free( stInfo.pbFileBuffer );
            free( stInfo.pdwFileOffsetOfLine );
            return 0;
            break;
            
            // 그 외 정보
        default:
            break;
        }
    }
    
    return 0;
}
