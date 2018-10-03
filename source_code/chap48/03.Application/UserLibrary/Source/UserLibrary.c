/**
 *  file    UserLibrary.c
 *  date    2009/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   유저 레벨에서 사용하는 라이브러리에 관련된 소스 파일
 */

#include "Types.h"
#include "UserLibrary.h"
#include <stdarg.h>

//==============================================================================
//  화면 입출력과 표준 함수 관련
//==============================================================================
/** 
 *  메모리를 특정 값으로 채움
 */
void memset( void* pvDestination, BYTE bData, int iSize )
{
    int i;
    QWORD qwData;
    int iRemainByteStartOffset;
    
    // 8 바이트 데이터를 채움
    qwData = 0;
    for( i = 0 ; i < 8 ; i++ )
    {
        qwData = ( qwData << 8 ) | bData;
    }
    
    // 8 바이트씩 먼저 채움
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = qwData;
    }
    
    // 8 바이트씩 채우고 남은 부분을 마무리
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset++ ] = bData;
    }
}

/**
 *  메모리 복사
 */
int memcpy( void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    int iRemainByteStartOffset;
    
    // 8 바이트씩 먼저 복사
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = ( ( QWORD* ) pvSource )[ i ];
    }
    
    // 8 바이트씩 채우고 남은 부분을 마무리
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset ] = 
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        iRemainByteStartOffset++;
    }
    return iSize;
}

/**
 *  메모리 비교
 */
int memcmp( const void* pvDestination, const void* pvSource, int iSize )
{
    int i, j;
    int iRemainByteStartOffset;
    QWORD qwValue;
    char cValue;
    
    // 8 바이트씩 먼저 비교
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        qwValue = ( ( QWORD* ) pvDestination )[ i ] - ( ( QWORD* ) pvSource )[ i ];

        // 틀린 위치를 정확하게 찾아서 그 값을 반환
        if( qwValue != 0 )
        {
            for( i = 0 ; i < 8 ; i++ )
            {
                if( ( ( qwValue >> ( i * 8 ) ) & 0xFF ) != 0 )
                {
                    return ( qwValue >> ( i * 8 ) ) & 0xFF;
                }
            }
        }
    }
    
    // 8 바이트씩 채우고 남은 부분을 마무리
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        cValue = ( ( char* ) pvDestination )[ iRemainByteStartOffset ] -
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        if( cValue != 0 )
        {
            return cValue;
        }
        iRemainByteStartOffset++;
    }    
    return 0;
}

/**
 *  문자열을 복사
 */
int strcpy( char* pcDestination, const char* pcSource )
{
    int i;
    
    for( i = 0 ; pcSource[ i ] != 0 ; i++ )
    {
        pcDestination[ i ] = pcSource[ i ];
    }
    return i;
}

/**
 *  문자열을 비교
 */
int strcmp( char* pcDestination, const char* pcSource )
{
    int i;
    
    for( i = 0 ; ( pcDestination[ i ] != 0 ) && ( pcSource[ i ] != 0 ) ; i++ )
    {
        if( ( pcDestination[ i ] - pcSource[ i ] ) != 0 )
        {
            break;
        }
    }
    
    return ( pcDestination[ i ] - pcSource[ i ] );
}

/**
 *  문자열의 길이를 반환
 */
int strlen( const char* pcBuffer )
{
    int i;
    
    for( i = 0 ; ; i++ )
    {
        if( pcBuffer[ i ] == '\0' )
        {
            break;
        }
    }
    return i;
}

/**
 *  atoi() 함수의 내부 구현
 */
long atoi( const char* pcBuffer, int iRadix )
{
    long lReturn;
    
    switch( iRadix )
    {
        // 16진수
    case 16:
        lReturn = HexStringToQword( pcBuffer );
        break;
        
        // 10진수 또는 기타
    case 10:
    default:
        lReturn = DecimalStringToLong( pcBuffer );
        break;
    }
    return lReturn;
}

/**
 *  16진수 문자열을 QWORD로 변환 
 */
QWORD HexStringToQword( const char* pcBuffer )
{
    QWORD qwValue = 0;
    int i;
    
    // 문자열을 돌면서 차례로 변환
    for( i = 0 ; pcBuffer[ i ] != '\0' ; i++ )
    {
        qwValue *= 16;
        if( ( 'A' <= pcBuffer[ i ] )  && ( pcBuffer[ i ] <= 'Z' ) )
        {
            qwValue += ( pcBuffer[ i ] - 'A' ) + 10;
        }
        else if( ( 'a' <= pcBuffer[ i ] )  && ( pcBuffer[ i ] <= 'z' ) )
        {
            qwValue += ( pcBuffer[ i ] - 'a' ) + 10;
        }
        else 
        {
            qwValue += pcBuffer[ i ] - '0';
        }
    }
    return qwValue;
}

/**
 *  10진수 문자열을 long으로 변환
 */
long DecimalStringToLong( const char* pcBuffer )
{
    long lValue = 0;
    int i;
    
    // 음수이면 -를 제외하고 나머지를 먼저 long으로 변환
    if( pcBuffer[ 0 ] == '-' )
    {
        i = 1;
    }
    else
    {
        i = 0;
    }
    
    // 문자열을 돌면서 차례로 변환
    for( ; pcBuffer[ i ] != '\0' ; i++ )
    {
        lValue *= 10;
        lValue += pcBuffer[ i ] - '0';
    }
    
    // 음수이면 - 추가
    if( pcBuffer[ 0 ] == '-' )
    {
        lValue = -lValue;
    }
    return lValue;
}

/**
 *  itoa() 함수의 내부 구현
 */
int itoa( long lValue, char* pcBuffer, int iRadix )
{
    int iReturn;
    
    switch( iRadix )
    {
        // 16진수
    case 16:
        iReturn = HexToString( lValue, pcBuffer );
        break;
        
        // 10진수 또는 기타
    case 10:
    default:
        iReturn = DecimalToString( lValue, pcBuffer );
        break;
    }
    
    return iReturn;
}

/**
 *  16진수 값을 문자열로 변환
 */
int HexToString( QWORD qwValue, char* pcBuffer )
{
    QWORD i;
    QWORD qwCurrentValue;

    // 0이 들어오면 바로 처리
    if( qwValue == 0 )
    {
        pcBuffer[ 0 ] = '0';
        pcBuffer[ 1 ] = '\0';
        return 1;
    }
    
    // 버퍼에 1의 자리부터 16, 256, ...의 자리 순서로 숫자 삽입
    for( i = 0 ; qwValue > 0 ; i++ )
    {
        qwCurrentValue = qwValue % 16;
        if( qwCurrentValue >= 10 )
        {
            pcBuffer[ i ] = 'A' + ( qwCurrentValue - 10 );
        }
        else
        {
            pcBuffer[ i ] = '0' + qwCurrentValue;
        }
        
        qwValue = qwValue / 16;
    }
    pcBuffer[ i ] = '\0';
    
    // 버퍼에 들어있는 문자열을 뒤집어서 ... 256, 16, 1의 자리 순서로 변경
    ReverseString( pcBuffer );
    return i;
}

/**
 *  10진수 값을 문자열로 변환
 */
int DecimalToString( long lValue, char* pcBuffer )
{
    long i;

    // 0이 들어오면 바로 처리
    if( lValue == 0 )
    {
        pcBuffer[ 0 ] = '0';
        pcBuffer[ 1 ] = '\0';
        return 1;
    }
    
    // 만약 음수이면 출력 버퍼에 '-'를 추가하고 양수로 변환
    if( lValue < 0 )
    {
        i = 1;
        pcBuffer[ 0 ] = '-';
        lValue = -lValue;
    }
    else
    {
        i = 0;
    }

    // 버퍼에 1의 자리부터 10, 100, 1000 ...의 자리 순서로 숫자 삽입
    for( ; lValue > 0 ; i++ )
    {
        pcBuffer[ i ] = '0' + lValue % 10;        
        lValue = lValue / 10;
    }
    pcBuffer[ i ] = '\0';
    
    // 버퍼에 들어있는 문자열을 뒤집어서 ... 1000, 100, 10, 1의 자리 순서로 변경
    if( pcBuffer[ 0 ] == '-' )
    {
        // 음수인 경우는 부호를 제외하고 문자열을 뒤집음
        ReverseString( &( pcBuffer[ 1 ] ) );
    }
    else
    {
        ReverseString( pcBuffer );
    }
    
    return i;
}

/**
 *  문자열의 순서를 뒤집음
 */
void ReverseString( char* pcBuffer )
{
   int iLength;
   int i;
   char cTemp;
   
   
   // 문자열의 가운데를 중심으로 좌/우를 바꿔서 순서를 뒤집음
   iLength = strlen( pcBuffer );
   for( i = 0 ; i < iLength / 2 ; i++ )
   {
       cTemp = pcBuffer[ i ];
       pcBuffer[ i ] = pcBuffer[ iLength - 1 - i ];
       pcBuffer[ iLength - 1 - i ] = cTemp;
   }
}

/**
 *  sprintf() 함수의 내부 구현
 */
int sprintf( char* pcBuffer, const char* pcFormatString, ... )
{
    va_list ap;
    int iReturn;
    
    // 가변 인자를 꺼내서 vsprintf() 함수에 넘겨줌
    va_start( ap, pcFormatString );
    iReturn = vsprintf( pcBuffer, pcFormatString, ap );
    va_end( ap );
    
    return iReturn;
}

/**
 *  vsprintf() 함수의 내부 구현
 *      버퍼에 포맷 문자열에 따라 데이터를 복사
 */
int vsprintf( char* pcBuffer, const char* pcFormatString, va_list ap )
{
    QWORD i, j, k;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char* pcCopyString;
    QWORD qwValue;
    int iValue;
    double dValue;
    
    // 포맷 문자열의 길이를 읽어서 문자열의 길이만큼 데이터를 출력 버퍼에 출력
    iFormatLength = strlen( pcFormatString );
    for( i = 0 ; i < iFormatLength ; i++ ) 
    {
        // %로 시작하면 데이터 타입 문자로 처리
        if( pcFormatString[ i ] == '%' ) 
        {
            // % 다음의 문자로 이동
            i++;
            switch( pcFormatString[ i ] ) 
            {
                // 문자열 출력  
            case 's':
                // 가변 인자에 들어있는 파라미터를 문자열 타입으로 변환
                pcCopyString = ( char* ) ( va_arg(ap, char* ));
                iCopyLength = strlen( pcCopyString );
                // 문자열의 길이만큼을 출력 버퍼로 복사하고 출력한 길이만큼 
                // 버퍼의 인덱스를 이동
                memcpy( pcBuffer + iBufferIndex, pcCopyString, iCopyLength );
                iBufferIndex += iCopyLength;
                break;
                
                // 문자 출력
            case 'c':
                // 가변 인자에 들어있는 파라미터를 문자 타입으로 변환하여 
                // 출력 버퍼에 복사하고 버퍼의 인덱스를 1만큼 이동
                pcBuffer[ iBufferIndex ] = ( char ) ( va_arg( ap, int ) );
                iBufferIndex++;
                break;

                // 정수 출력
            case 'd':
            case 'i':
                // 가변 인자에 들어있는 파라미터를 정수 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                iValue = ( int ) ( va_arg( ap, int ) );
                iBufferIndex += itoa( iValue, pcBuffer + iBufferIndex, 10 );
                break;
                
                // 4바이트 Hex 출력
            case 'x':
            case 'X':
                // 가변 인자에 들어있는 파라미터를 DWORD 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                qwValue = ( DWORD ) ( va_arg( ap, DWORD ) ) & 0xFFFFFFFF;
                iBufferIndex += itoa( qwValue, pcBuffer + iBufferIndex, 16 );
                break;

                // 8바이트 Hex 출력
            case 'q':
            case 'Q':
            case 'p':
                // 가변 인자에 들어있는 파라미터를 QWORD 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                qwValue = ( QWORD ) ( va_arg( ap, QWORD ) );
                iBufferIndex += itoa( qwValue, pcBuffer + iBufferIndex, 16 );
                break;
            
                // 소수점 둘째 자리까지 실수를 출력
            case 'f':
                dValue = ( double) ( va_arg( ap, double ) );
                // 셋째 자리에서 반올림 처리
                dValue += 0.005;
                // 소수점 둘째 자리부터 차례로 저장하여 버퍼를 뒤집음
                pcBuffer[ iBufferIndex ] = '0' + ( QWORD ) ( dValue * 100 ) % 10;
                pcBuffer[ iBufferIndex + 1 ] = '0' + ( QWORD ) ( dValue * 10 ) % 10;
                pcBuffer[ iBufferIndex + 2 ] = '.';
                for( k = 0 ; ; k++ )
                {
                    // 정수 부분이 0이면 종료
                    if( ( ( QWORD ) dValue == 0 ) && ( k != 0 ) )
                    {
                        break;
                    }
                    pcBuffer[ iBufferIndex + 3 + k ] = '0' + ( ( QWORD ) dValue % 10 );
                    dValue = dValue / 10;
                }
                pcBuffer[ iBufferIndex + 3 + k ] = '\0';
                // 값이 저장된 길이만큼 뒤집고 길이를 증가시킴
                ReverseString( pcBuffer + iBufferIndex );
                iBufferIndex += 3 + k;
                break;
                
                // 위에 해당하지 않으면 문자를 그대로 출력하고 버퍼의 인덱스를
                // 1만큼 이동
            default:
                pcBuffer[ iBufferIndex ] = pcFormatString[ i ];
                iBufferIndex++;
                break;
            }
        } 
        // 일반 문자열 처리
        else
        {
            // 문자를 그대로 출력하고 버퍼의 인덱스를 1만큼 이동
            pcBuffer[ iBufferIndex ] = pcFormatString[ i ];
            iBufferIndex++;
        }
    }
    
    // NULL을 추가하여 완전한 문자열로 만들고 출력한 문자의 길이를 반환
    pcBuffer[ iBufferIndex ] = '\0';
    return iBufferIndex;
}

/**
 *  printf 함수의 내부 구현
 */
void printf( const char* pcFormatString, ... )
{
    va_list ap;
    char vcBuffer[ 1024 ];
    int iNextPrintOffset;

    // 가변 인자 리스트를 사용해서 vsprintf()로 처리
    va_start( ap, pcFormatString );
    vsprintf( vcBuffer, pcFormatString, ap );
    va_end( ap );
    
    // 포맷 문자열을 화면에 출력
    iNextPrintOffset = ConsolePrintString( vcBuffer );
    
    // 커서의 위치를 업데이트
    SetCursor( iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH );
}

// 난수를 발생시키기 위한 변수
static volatile QWORD gs_qwRandomValue = 0;

/**
 *  난수의 초깃값(Seed) 설정
 */
void srand( QWORD qwSeed )
{
    gs_qwRandomValue = qwSeed;
}

/**
 *  임의의 난수를 반환
 */
QWORD rand( void )
{
    gs_qwRandomValue = ( gs_qwRandomValue * 412153 + 5571031 ) >> 16;
    return gs_qwRandomValue;
}

//==============================================================================
// GUI 시스템 관련
//==============================================================================
/**
 *  (x, y)가 사각형 영역 안에 있는지 여부를 반환
 */
BOOL IsInRectangle( const RECT* pstArea, int iX, int iY )
{
    // 화면에 표시되는 영역을 벗어났다면 그리지 않음
    if( ( iX <  pstArea->iX1 ) || ( pstArea->iX2 < iX ) ||
        ( iY <  pstArea->iY1 ) || ( pstArea->iY2 < iY ) )
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 *  사각형의 너비를 반환
 */
int GetRectangleWidth( const RECT* pstArea )
{
    int iWidth;
    
    iWidth = pstArea->iX2 - pstArea->iX1 + 1;
    
    if( iWidth < 0 )
    {
        return -iWidth;
    }
    
    return iWidth;
}

/**
 *  사각형의 높이를 반환
 */
int GetRectangleHeight( const RECT* pstArea )
{
    int iHeight;
    
    iHeight = pstArea->iY2 - pstArea->iY1 + 1;
    
    if( iHeight < 0 )
    {
        return -iHeight;
    }
    
    return iHeight;
}

/**
 *  영역 1과 영역 2의 겹치는 영역을 반환
 */
BOOL GetOverlappedRectangle( const RECT* pstArea1, const RECT* pstArea2,
        RECT* pstIntersection  )
{
    int iMaxX1;
    int iMinX2;
    int iMaxY1;
    int iMinY2;

    // X축의 시작점은 두 점 중에서 큰 것을 찾음
    iMaxX1 = MAX( pstArea1->iX1, pstArea2->iX1 );
    // X축의 끝점은 두 점 중에서 작은 것을 찾음
    iMinX2 = MIN( pstArea1->iX2, pstArea2->iX2 );
    // 계산한 시작점의 위치가 끝점의 위치보다 크다면 두 사각형은 겹치지 않음
    if( iMinX2 < iMaxX1 )
    {
        return FALSE;
    }

    // Y축의 시작점은 두 점 중에서 큰 것을 찾음
    iMaxY1 = MAX( pstArea1->iY1, pstArea2->iY1 );
    // Y축의 끝점은 두 점 중에서 작은 것을 찾음
    iMinY2 = MIN( pstArea1->iY2, pstArea2->iY2 );
    // 계산한 시작점의 위치가 끝점의 위치보다 크다면 두 사각형은 겹치지 않음
    if( iMinY2 < iMaxY1 )
    {
        return FALSE;
    }

    // 겹치는 영역의 정보 저장
    pstIntersection->iX1 = iMaxX1;
    pstIntersection->iY1 = iMaxY1;
    pstIntersection->iX2 = iMinX2;
    pstIntersection->iY2 = iMinY2;

    return TRUE;
}

/**
 *  전체 화면을 기준으로 한 X,Y 좌표를 윈도우 내부 좌표로 변환
 */
BOOL ConvertPointScreenToClient( QWORD qwWindowID, const POINT* pstXY, 
        POINT* pstXYInWindow )
{
    RECT stArea;
    
    // 윈도우 영역을 반환
    if( GetWindowArea( qwWindowID, &stArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstXYInWindow->iX = pstXY->iX - stArea.iX1;
    pstXYInWindow->iY = pstXY->iY - stArea.iY1;
    return TRUE;
}

/**
 *  윈도우 내부를 기준으로 한 X,Y 좌표를 화면 좌표로 변환
 */
BOOL ConvertPointClientToScreen( QWORD qwWindowID, const POINT* pstXY, 
        POINT* pstXYInScreen )
{
    RECT stArea;
    
    // 윈도우 영역을 반환
    if( GetWindowArea( qwWindowID, &stArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstXYInScreen->iX = pstXY->iX + stArea.iX1;
    pstXYInScreen->iY = pstXY->iY + stArea.iY1;
    return TRUE;
}

/**
 *  전체 화면을 기준으로 한 사각형 좌표를 윈도우 내부 좌표로 변환
 */
BOOL ConvertRectScreenToClient( QWORD qwWindowID, const RECT* pstArea, 
        RECT* pstAreaInWindow )
{
    RECT stWindowArea;
    
    // 윈도우 영역을 반환
    if( GetWindowArea( qwWindowID, &stWindowArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstAreaInWindow->iX1 = pstArea->iX1 - stWindowArea.iX1;
    pstAreaInWindow->iY1 = pstArea->iY1 - stWindowArea.iY1;
    pstAreaInWindow->iX2 = pstArea->iX2 - stWindowArea.iX1;
    pstAreaInWindow->iY2 = pstArea->iY2 - stWindowArea.iY1;
    return TRUE;
}

/**
 *  윈도우 내부를 기준으로 한 사각형 좌표를 화면 좌표로 변환
 */
BOOL ConvertRectClientToScreen( QWORD qwWindowID, const RECT* pstArea, 
        RECT* pstAreaInScreen )
{
    RECT stWindowArea;
    
    // 윈도우 영역을 반환
    if( GetWindowArea( qwWindowID, &stWindowArea ) == FALSE )
    {
        return FALSE;
    }
    
    pstAreaInScreen->iX1 = pstArea->iX1 + stWindowArea.iX1;
    pstAreaInScreen->iY1 = pstArea->iY1 + stWindowArea.iY1;
    pstAreaInScreen->iX2 = pstArea->iX2 + stWindowArea.iX1;
    pstAreaInScreen->iY2 = pstArea->iY2 + stWindowArea.iY1;
    return TRUE;
}

/**
 *  사각형 자료구조를 채움
 *      x1과 x2, y1과 y2를 비교해서 x1 < x2, y1 < y2가 되도록 저장
 */
void SetRectangleData( int iX1, int iY1, int iX2, int iY2, RECT* pstRect )
{
    // x1 < x2가 되도록 RECT 자료구조에 X좌표를 설정
    if( iX1 < iX2 )
    {
        pstRect->iX1 = iX1;
        pstRect->iX2 = iX2;
    }
    else
    {
        pstRect->iX1 = iX2;
        pstRect->iX2 = iX1;
    }
    
    // y1 < y2가 되도록 RECT 자료구조에 Y좌표를 설정
    if( iY1 < iY2 )
    {
        pstRect->iY1 = iY1;
        pstRect->iY2 = iY2;
    }
    else
    {
        pstRect->iY1 = iY2;
        pstRect->iY2 = iY1;
    }
}

/**
 *  마우스 이벤트 자료구조를 설정
 */
BOOL SetMouseEvent( QWORD qwWindowID, QWORD qwEventType, int iMouseX, int iMouseY, 
        BYTE bButtonStatus, EVENT* pstEvent )
{
    POINT stMouseXYInWindow;
    POINT stMouseXY;
    
    // 이벤트 종류를 확인하여 마우스 이벤트 생성
    switch( qwEventType )
    {
        // 마우스 이벤트 처리
    case EVENT_MOUSE_MOVE:
    case EVENT_MOUSE_LBUTTONDOWN:
    case EVENT_MOUSE_LBUTTONUP:            
    case EVENT_MOUSE_RBUTTONDOWN:
    case EVENT_MOUSE_RBUTTONUP:
    case EVENT_MOUSE_MBUTTONDOWN:
    case EVENT_MOUSE_MBUTTONUP:
        // 마우스의 X, Y좌표를 설정
        stMouseXY.iX = iMouseX;
        stMouseXY.iY = iMouseY;
        
        // 마우스 X, Y좌표를 윈도우 내부 좌표로 변환
        if( ConvertPointScreenToClient( qwWindowID, &stMouseXY, &stMouseXYInWindow ) 
                == FALSE )
        {
            return FALSE;
        }

        // 이벤트 타입 설정
        pstEvent->qwType = qwEventType;
        // 윈도우 ID 설정
        pstEvent->stMouseEvent.qwWindowID = qwWindowID;    
        // 마우스 버튼의 상태 설정
        pstEvent->stMouseEvent.bButtonStatus = bButtonStatus;
        // 마우스 커서의 좌표를 윈도우 내부 좌표로 변환한 값을 설정
        memcpy( &( pstEvent->stMouseEvent.stPoint ), &stMouseXYInWindow, 
                sizeof( POINT ) );
        break;
        
    default:
        return FALSE;
        break;
    }    
    return TRUE;
}

/**
 *  윈도우 이벤트 자료구조를 설정
 */
BOOL SetWindowEvent( QWORD qwWindowID, QWORD qwEventType, EVENT* pstEvent )
{
    RECT stArea;
    
    // 이벤트 종류를 확인하여 윈도우 이벤트 생성
    switch( qwEventType )
    {
        // 윈도우 이벤트 처리
    case EVENT_WINDOW_SELECT:
    case EVENT_WINDOW_DESELECT:
    case EVENT_WINDOW_MOVE:
    case EVENT_WINDOW_RESIZE:
    case EVENT_WINDOW_CLOSE:
        // 이벤트 타입 설정
        pstEvent->qwType = qwEventType;
        // 윈도우 ID 설정
        pstEvent->stWindowEvent.qwWindowID = qwWindowID;
        // 윈도우 영역을 반환
        if( GetWindowArea( qwWindowID, &stArea ) == FALSE )
        {
            return FALSE;
        }
        
        // 윈도우의 현재 좌표를 설정
        memcpy( &( pstEvent->stWindowEvent.stArea ), &stArea, sizeof( RECT ) );
        break;
        
    default:
        return FALSE;
        break;
    }    
    return TRUE;
}

/**
 *  키 이벤트 자료구조를 설정
 */
void SetKeyEvent( QWORD qwWindow, const KEYDATA* pstKeyData, EVENT* pstEvent )
{
    // 눌림 또는 떨어짐 처리
    if( pstKeyData->bFlags & KEY_FLAGS_DOWN )
    {
        pstEvent->qwType = EVENT_KEY_DOWN;
    }
    else
    {
        pstEvent->qwType = EVENT_KEY_UP;
    }
    
    // 키의 각 정보를 설정
    pstEvent->stKeyEvent.bASCIICode = pstKeyData->bASCIICode;
    pstEvent->stKeyEvent.bScanCode = pstKeyData->bScanCode;
    pstEvent->stKeyEvent.bFlags = pstKeyData->bFlags;
}
