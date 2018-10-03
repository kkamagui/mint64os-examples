/**
 *  file    Utility.h
 *  date    2009/01/17
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   OS에서 사용할 유틸리티 함수에 관련된 파일
 */

#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>


// PIT 컨트롤러가 발생한 횟수를 저장할 카운터
volatile QWORD g_qwTickCount = 0;

/**
 *  메모리를 특정 값으로 채움
 */
void kMemSet( void* pvDestination, BYTE bData, int iSize )
{
    int i;
    
    for( i = 0 ; i < iSize ; i++ )
    {
        ( ( char* ) pvDestination )[ i ] = bData;
    }
}

/**
 *  메모리 복사
 */
int kMemCpy( void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    
    for( i = 0 ; i < iSize ; i++ )
    {
        ( ( char* ) pvDestination )[ i ] = ( ( char* ) pvSource )[ i ];
    }
    
    return iSize;
}

/**
 *  메모리 비교
 */
int kMemCmp( const void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    char cTemp;
    
    for( i = 0 ; i < iSize ; i++ )
    {
        cTemp = ( ( char* ) pvDestination )[ i ] - ( ( char* ) pvSource )[ i ];
        if( cTemp != 0 )
        {
            return ( int ) cTemp;
        }
    }
    return 0;
}

/**
 *  RFLAGS 레지스터의 인터럽트 플래그를 변경하고 이전 인터럽트 플래그의 상태를 반환
 */
BOOL kSetInterruptFlag( BOOL bEnableInterrupt )
{
    QWORD qwRFLAGS;
    
    // 이전의 RFLAGS 레지스터 값을 읽은 뒤에 인터럽트 가능/불가 처리
    qwRFLAGS = kReadRFLAGS();    
    if( bEnableInterrupt == TRUE )
    {
        kEnableInterrupt();
    }
    else
    {
        kDisableInterrupt();
    }
    
    // 이전 RFLAGS 레지스터의 IF 비트(비트 9)를 확인하여 이전의 인터럽트 상태를 반환
    if( qwRFLAGS & 0x0200 )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  문자열의 길이를 반환
 */
int kStrLen( const char* pcBuffer )
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

// 램의 총 크기(Mbyte 단위)
static gs_qwTotalRAMMBSize = 0;

/**
 *  64Mbyte 이상의 위치부터 램 크기를 체크
 *      최초 부팅 과정에서 한번만 호출해야 함
 */
void kCheckTotalRAMSize( void )
{
    DWORD* pdwCurrentAddress;
    DWORD dwPreviousValue;
    
    // 64Mbyte(0x4000000)부터 4Mbyte단위로 검사 시작
    pdwCurrentAddress = ( DWORD* ) 0x4000000;
    while( 1 )
    {
        // 이전에 메모리에 있던 값을 저장
        dwPreviousValue = *pdwCurrentAddress;
        // 0x12345678을 써서 읽었을 때 문제가 없는 곳까지를 유효한 메모리 
        // 영역으로 인정
        *pdwCurrentAddress = 0x12345678;
        if( *pdwCurrentAddress != 0x12345678 )
        {
            break;
        }
        // 이전 메모리 값으로 복원
        *pdwCurrentAddress = dwPreviousValue;
        // 다음 4Mbyte 위치로 이동
        pdwCurrentAddress += ( 0x400000 / 4 );
    }
    // 체크가 성공한 어드레스를 1Mbyte로 나누어 Mbyte 단위로 계산
    gs_qwTotalRAMMBSize = ( QWORD ) pdwCurrentAddress / 0x100000;
}   

/**
 *  RAM 크기를 반환
 */
QWORD kGetTotalRAMSize( void )
{
    return gs_qwTotalRAMMBSize;
}

/**
 *  atoi() 함수의 내부 구현
 */
long kAToI( const char* pcBuffer, int iRadix )
{
    long lReturn;
    
    switch( iRadix )
    {
        // 16진수
    case 16:
        lReturn = kHexStringToQword( pcBuffer );
        break;
        
        // 10진수 또는 기타
    case 10:
    default:
        lReturn = kDecimalStringToLong( pcBuffer );
        break;
    }
    return lReturn;
}

/**
 *  16진수 문자열을 QWORD로 변환 
 */
QWORD kHexStringToQword( const char* pcBuffer )
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
long kDecimalStringToLong( const char* pcBuffer )
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
int kIToA( long lValue, char* pcBuffer, int iRadix )
{
    int iReturn;
    
    switch( iRadix )
    {
        // 16진수
    case 16:
        iReturn = kHexToString( lValue, pcBuffer );
        break;
        
        // 10진수 또는 기타
    case 10:
    default:
        iReturn = kDecimalToString( lValue, pcBuffer );
        break;
    }
    
    return iReturn;
}

/**
 *  16진수 값을 문자열로 변환
 */
int kHexToString( QWORD qwValue, char* pcBuffer )
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
    kReverseString( pcBuffer );
    return i;
}

/**
 *  10진수 값을 문자열로 변환
 */
int kDecimalToString( long lValue, char* pcBuffer )
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
        kReverseString( &( pcBuffer[ 1 ] ) );
    }
    else
    {
        kReverseString( pcBuffer );
    }
    
    return i;
}

/**
 *  문자열의 순서를 뒤집음
 */
void kReverseString( char* pcBuffer )
{
   int iLength;
   int i;
   char cTemp;
   
   
   // 문자열의 가운데를 중심으로 좌/우를 바꿔서 순서를 뒤집음
   iLength = kStrLen( pcBuffer );
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
int kSPrintf( char* pcBuffer, const char* pcFormatString, ... )
{
    va_list ap;
    int iReturn;
    
    // 가변 인자를 꺼내서 vsprintf() 함수에 넘겨줌
    va_start( ap, pcFormatString );
    iReturn = kVSPrintf( pcBuffer, pcFormatString, ap );
    va_end( ap );
    
    return iReturn;
}

/**
 *  vsprintf() 함수의 내부 구현
 *      버퍼에 포맷 문자열에 따라 데이터를 복사
 */
int kVSPrintf( char* pcBuffer, const char* pcFormatString, va_list ap )
{
    QWORD i, j;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char* pcCopyString;
    QWORD qwValue;
    int iValue;
    
    // 포맷 문자열의 길이를 읽어서 문자열의 길이만큼 데이터를 출력 버퍼에 출력
    iFormatLength = kStrLen( pcFormatString );
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
                iCopyLength = kStrLen( pcCopyString );
                // 문자열의 길이만큼을 출력 버퍼로 복사하고 출력한 길이만큼 
                // 버퍼의 인덱스를 이동
                kMemCpy( pcBuffer + iBufferIndex, pcCopyString, iCopyLength );
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
                iBufferIndex += kIToA( iValue, pcBuffer + iBufferIndex, 10 );
                break;
                
                // 4바이트 Hex 출력
            case 'x':
            case 'X':
                // 가변 인자에 들어있는 파라미터를 DWORD 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                qwValue = ( DWORD ) ( va_arg( ap, DWORD ) ) & 0xFFFFFFFF;
                iBufferIndex += kIToA( qwValue, pcBuffer + iBufferIndex, 16 );
                break;

                // 8바이트 Hex 출력
            case 'q':
            case 'Q':
            case 'p':
                // 가변 인자에 들어있는 파라미터를 QWORD 타입으로 변환하여
                // 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
                qwValue = ( QWORD ) ( va_arg( ap, QWORD ) );
                iBufferIndex += kIToA( qwValue, pcBuffer + iBufferIndex, 16 );
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
 *  Tick Count를 반환
 */
QWORD kGetTickCount( void )
{
    return g_qwTickCount;
}

/**
 *  밀리세컨드(milisecond) 동안 대기
 */
void kSleep( QWORD qwMillisecond )
{
    QWORD qwLastTickCount;
    
    qwLastTickCount = g_qwTickCount;
    
    while( ( g_qwTickCount - qwLastTickCount ) <= qwMillisecond )
    {
        kSchedule();
    }
}
