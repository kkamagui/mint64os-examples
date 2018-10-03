/**
 *  file    Main.c
 *  date    2010/01/03
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
        printf( "ex) exec textviwer.elf abc.txt\n" );
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
                              WINDOW_FLAGS_RESIZABLE, "Text Viewer" );
    
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


/**
 *  파일을 찾아서 파일의 크기만큼 버퍼를 할당하고 라인 별 파일 오프셋 버퍼를 할당한 뒤에
 *  파일의 내용을 읽어서 메모리에 저장
 */
BOOL ReadFileToBuffer( const char* pcFileName, TEXTINFO* pstInfo )
{
    DIR* pstDirectory;
    struct dirent* pstEntry;
    DWORD dwFileSize;
    FILE* pstFile;
    DWORD dwReadSize;
    
    //--------------------------------------------------------------------------
    // 루트 디렉터리를 열어서 파일을 검색
    //--------------------------------------------------------------------------
    pstDirectory = opendir( "/" );
    dwFileSize = 0;
    
    // 디렉터리에서 파일을 검색
    while( 1 )
    {
        // 디렉터리에서 엔트리 하나를 읽음
        pstEntry = readdir( pstDirectory );
        // 더 이상 파일이 없으면 나감
        if( pstEntry == NULL )
        {
            break;
        }
        
        // 파일 이름의 길이와 내용이 같은 것을 검색
        if( ( strlen( pstEntry->d_name ) == strlen( pcFileName ) ) &&
            ( memcmp( pstEntry->d_name, pcFileName, strlen( pcFileName ) ) 
                    == 0 ) )
        {
            dwFileSize = pstEntry->dwFileSize;
            break;
        }
    }
    // 디렉터리 핸들을 반환, 핸들을 반환하지 않으면 메모리가 해제되지 않고 남으므로
    // 꼭 해제해야 함
    closedir( pstDirectory );

    if( dwFileSize == 0 )
    {
        printf( "%s file doesn't exist or size is zero\n", 
                pcFileName );
        return FALSE;
    }
    
    // 파일 이름을 저장
    memcpy( &( pstInfo->vcFileName ), pcFileName, sizeof( pstInfo->vcFileName ) );
    pstInfo->vcFileName[ sizeof( pstInfo->vcFileName ) - 1 ] = '\0';
    
    //--------------------------------------------------------------------------
    // 파일의 전체를 읽을 수 있는 임시 버퍼와 라인 별 파일 오프셋을 저장할 버퍼를 
    // 할당 받아서 파일의 내용을 모두 저장
    //--------------------------------------------------------------------------
    // 라인 별로 파일 오프셋을 저장할 버퍼를 할당
    pstInfo->pdwFileOffsetOfLine = malloc( MAXLINECOUNT * sizeof( DWORD ) );
    if( pstInfo->pdwFileOffsetOfLine == NULL )
    {
        printf( "Memory allocation fail\n" );
        return FALSE;
    }
    
    // 파일의 내용을 저장할 버퍼를 할당
    pstInfo->pbFileBuffer = ( BYTE* ) malloc( dwFileSize );
    if( pstInfo->pbFileBuffer == NULL )
    {
        printf( "Memory %dbytes allocate fail\n", dwFileSize );
        free( pstInfo->pdwFileOffsetOfLine );
        return FALSE;
    }
    
    // 파일을 열어서 모두 메모리에 저장
    pstFile = fopen( pcFileName, "r" );
    if( ( pstFile != NULL ) && 
        ( fread( pstInfo->pbFileBuffer, 1, dwFileSize, pstFile ) == dwFileSize ) )
    {
        fclose( pstFile );
        printf( "%s file read success\n", pcFileName );
    }
    else
    {
        printf( "%s file read fail\n", pcFileName );
        free( pstInfo->pdwFileOffsetOfLine );
        free( pstInfo->pbFileBuffer );
        fclose( pstFile );
        return FALSE;
    }

    // 파일의 크기 저장
    pstInfo->dwFileSize = dwFileSize;
    return TRUE;
}

/**
 *  파일 버퍼의 내용을 분석하여 라인 별 파일 오프셋을 계산
 */
void CalculateFileOffsetOfLine( int iWidth, int iHeight, TEXTINFO* pstInfo )
{
    DWORD i;
    int iLineIndex;
    int iColumnIndex;
    
    // 여유 공간과 제목 표시줄의 높이를 고려해서 라인 별 문자 수와 출력할 수 있는 
    // 라인 수를 계산
    pstInfo->iColumnCount = ( iWidth - MARGIN * 2 ) / FONT_ENGLISHWIDTH;
    pstInfo->iRowCount = ( iHeight - ( WINDOW_TITLEBAR_HEIGHT * 2 ) - 
            ( MARGIN * 2 ) ) / FONT_ENGLISHHEIGHT;
    
    // 파일의 처음부터 끝까지 라인 번호를 계산해서 파일 오프셋을 저장
    iLineIndex = 0;
    iColumnIndex = 0;
    pstInfo->pdwFileOffsetOfLine[ 0 ] = 0;
    for( i = 0 ; i < pstInfo->dwFileSize ; i++ )
    {
        // 라인 피드 문자는 무시
        if( pstInfo->pbFileBuffer[ i ] == '\r' )
        {
            continue;
        }
        else if( pstInfo->pbFileBuffer[ i ] == '\t' )
        {
            // 탭 문자이면 탭 문자의 크기 단위로 출력할 오프셋을 변경
            iColumnIndex = iColumnIndex + TABSPACE;
            iColumnIndex -= iColumnIndex % TABSPACE;
        }
        else
        {
            iColumnIndex++;
        }
            
        // 출력할 위치가 라인 별 문자 수를 넘거나 탭 문자를 출력할 공간이 없는 경우, 
        // 또는 개행 문자가 검출되면 라인 변경
        if( ( iColumnIndex >= pstInfo->iColumnCount ) ||
            ( pstInfo->pbFileBuffer[ i ] == '\n' ) )
        {
            iLineIndex++;
            iColumnIndex = 0;
            
            // 라인 인덱스 버퍼에 오프셋 삽입
            if( i + 1 < pstInfo->dwFileSize )
            {
                pstInfo->pdwFileOffsetOfLine[ iLineIndex ] = i + 1;
            }
            
            // 텍스트 뷰어가 지원하는 최대 라인 수를 넘어서면 종료
            if( iLineIndex >= MAXLINECOUNT )
            {
                break;
            }
        }
    }
    
    // 가장 마지막 라인 번호를 저장
    pstInfo->iMaxLineCount = iLineIndex;
}

/**
 *  윈도우 화면 버퍼에 현재 라인부터 화면에 출력
 */
BOOL DrawTextBuffer( QWORD qwWindowID, TEXTINFO* pstInfo )
{
    DWORD i;
    DWORD j;
    DWORD dwBaseOffset;
    BYTE bTemp;
    int iXOffset;
    int iYOffset;
    int iLineCountToPrint;
    int iColumnCountToPrint;
    char vcBuffer[ 100 ];
    RECT stWindowArea;
    int iLength;
    int iWidth;
    int iColumnIndex;

    // 좌표의 기준값
    iXOffset = MARGIN;
    iYOffset = WINDOW_TITLEBAR_HEIGHT;
    GetWindowArea( qwWindowID, &stWindowArea );

    //--------------------------------------------------------------------------
    // 파일 정보 표시 영역에 정보를 출력
    //--------------------------------------------------------------------------
    // 파일 이름과 현재 라인, 그리고 전체 라인 수를 출력
    iWidth = GetRectangleWidth( &stWindowArea );
    DrawRect( qwWindowID, 2, iYOffset, iWidth - 3, WINDOW_TITLEBAR_HEIGHT * 2, 
            RGB( 55, 215, 47 ), TRUE );
    // 임시 버퍼에 정보를 저장
    sprintf( vcBuffer, "File: %s, Line: %d/%d\n", pstInfo->vcFileName, 
            pstInfo->iCurrentLineIndex + 1, pstInfo->iMaxLineCount );
    iLength = strlen( vcBuffer );
    // 저장된 정보를 파일 정보 표시 영역의 가운데에 출력
    DrawText( qwWindowID, ( iWidth - iLength * FONT_ENGLISHWIDTH ) / 2, 
            WINDOW_TITLEBAR_HEIGHT + 2, RGB( 255, 255, 255 ), RGB( 55, 215, 47 ), 
            vcBuffer, strlen( vcBuffer ) );
    
    //--------------------------------------------------------------------------
    // 파일 내용 표시 영역에 파일 내용을 출력
    //--------------------------------------------------------------------------
    // 데이터를 출력할 부분을 모두 흰색으로 덮어쓴 뒤에 라인을 출력
    iYOffset = ( WINDOW_TITLEBAR_HEIGHT * 2 ) + MARGIN;
    DrawRect( qwWindowID, iXOffset, iYOffset, iXOffset + FONT_ENGLISHWIDTH * 
            pstInfo->iColumnCount, iYOffset + FONT_ENGLISHHEIGHT * pstInfo->iRowCount, 
            RGB( 255, 255, 255 ), TRUE );
    
    //--------------------------------------------------------------------------
    // 루프를 수행하면서 라인 단위로 화면에 출력
    //--------------------------------------------------------------------------
    // 현재 라인에서 남은 라인 수와 한 화면에 출력할 수 있는 라인 수를 비교하여
    // 작은 것을 선택
    iLineCountToPrint = MIN( pstInfo->iRowCount, 
                 ( pstInfo->iMaxLineCount - pstInfo->iCurrentLineIndex ) );
    for( j = 0 ; j < iLineCountToPrint ; j++ )
    {
        // 출력할 라인의 파일 오프셋
        dwBaseOffset = pstInfo->pdwFileOffsetOfLine[ pstInfo->iCurrentLineIndex + j ];

        //----------------------------------------------------------------------
        // 루프를 수행하면서 현재 라인에 문자를 출력
        //----------------------------------------------------------------------
        // 현재 라인에서 남은 문자 수와 한 화면에 출력할 수 있는 문자 수를 비교하여
        // 작은 것을 선택
        iColumnCountToPrint = MIN( pstInfo->iColumnCount,
                                   ( pstInfo->dwFileSize - dwBaseOffset ) );
        iColumnIndex = 0;
        for( i = 0 ; ( i < iColumnCountToPrint ) && 
                     ( iColumnIndex < pstInfo->iColumnCount ) ; i++ )
        {
            bTemp = pstInfo->pbFileBuffer[ i + dwBaseOffset ];
            // 개행 문자가 보이면 종료
            if( bTemp == '\n' )
            {
                break;
            }
            // 탭 문자이면 탭 문자의 크기 단위로 출력할 오프셋을 변경
            else if( bTemp == '\t')
            {
                // 탭 문자가 차지하는 영역 단위로 출력할 오프셋을 정렬
                iColumnIndex = iColumnIndex + TABSPACE;
                iColumnIndex -= iColumnIndex % TABSPACE;
            }
            // 라인 피드 문자이면 무시
            else if( bTemp == '\r' )
            {
                // Nothing
            }
            // 기타 문자는 화면에 출력
            else
            {
                // 출력할 위치에 문자를 출력하고 다음 위치로 이동
                DrawText( qwWindowID, iColumnIndex * FONT_ENGLISHWIDTH + iXOffset,
                        iYOffset + ( j * FONT_ENGLISHHEIGHT ), RGB( 0, 0, 0 ),
                        RGB( 255, 255, 255 ), &bTemp, 1 );
                iColumnIndex++;
            }
        }
    }
    
    // 윈도우 전체를 갱신하여 변경된 화면을 업데이트
    ShowWindow( qwWindowID, TRUE );
    
    return TRUE;
}
