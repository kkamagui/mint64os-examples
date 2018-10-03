/**
 *  file    Main.c
 *  date    2009/01/02
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   C 언어로 작성된 커널의 엔트리 포인트 파일
 */

#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "SerialPort.h"
#include "MultiProcessor.h"
#include "VBE.h"
#include "2DGraphics.h"

// Application Processor를 위한 Main 함수
void MainForApplicationProcessor( void );
// 그래픽 모드를 테스트하는 함수
void kStartGraphicModeTest();

/**
 *  Bootstrap Processor용 C 언어 커널 엔트리 포인트
 *      아래 함수는 C 언어 커널의 시작 부분임
 */
void Main( void )
{
    int iCursorX, iCursorY;
    
    // 부트 로더에 있는 BSP 플래그를 읽어서 Application Processor이면 
    // 해당 코어용 초기화 함수로 이동
    if( *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) == 0 )
    {
        MainForApplicationProcessor();
    }
    
    // Bootstrap Processor가 부팅을 완료했으므로, 0x7C09에 있는 Bootstrap Processor를
    // 나타내는 플래그를 0으로 설정하여 Application Processor용으로 코드 실행 경로를 변경
    *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) = 0;

    // 콘솔을 먼저 초기화한 후, 다음 작업을 수행
    kInitializeConsole( 0, 10 );    
    kPrintf( "Switch To IA-32e Mode Success~!!\n" );
    kPrintf( "IA-32e C Language Kernel Start..............[Pass]\n" );
    kPrintf( "Initialize Console..........................[Pass]\n" );
    
    // 부팅 상황을 화면에 출력
    kGetCursor( &iCursorX, &iCursorY );
    kPrintf( "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "TSS Segment Load............................[    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "IDT Initialize..............................[    ]" );
    kInitializeIDTTables();    
    kLoadIDTR( IDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "Total RAM Size Check........................[    ]" );
    kCheckTotalRAMSize();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass], Size = %d MB\n", kGetTotalRAMSize() );
    
    kPrintf( "TCB Pool And Scheduler Initialize...........[Pass]\n" );
    iCursorY++;
    kInitializeScheduler();
    
    // 동적 메모리 초기화
    kPrintf( "Dynamic Memory Initialize...................[Pass]\n" );
    iCursorY++;
    kInitializeDynamicMemory();
    
    // 1ms당 한번씩 인터럽트가 발생하도록 설정
    kInitializePIT( MSTOCOUNT( 1 ), 1 );
    
    kPrintf( "Keyboard Activate And Queue Initialize......[    ]" );
    // 키보드를 활성화
    if( kInitializeKeyboard() == TRUE )
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Fail\n" );
        while( 1 ) ;
    }
    
    kPrintf( "PIC Controller And Interrupt Initialize.....[    ]" );
    // PIC 컨트롤러 초기화 및 모든 인터럽트 활성화
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    // 파일 시스템을 초기화
    kPrintf( "File System Initialize......................[    ]" );
    if( kInitializeFileSystem() == TRUE )
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Pass\n" );
    }
    else
    {
        kSetCursor( 45, iCursorY++ );
        kPrintf( "Fail\n" );
    }

    // 시리얼 포트를 초기화    
    kPrintf( "Serial Port Initialize......................[Pass]\n" );
    iCursorY++;
    kInitializeSerialPort();

    // 유휴 태스크를 시스템 스레드로 생성하고 셸을 시작
    kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, 
            ( QWORD ) kIdleTask, kGetAPICID() );

    // 그래픽 모드가 아니면 콘솔 셸 실행
    if( *( BYTE* ) VBE_STARTGRAPHICMODEFLAGADDRESS == 0 )
    {
        kStartConsoleShell();
    }
    // 그래픽 모드이면 그래픽 모드 테스트 함수 실행
    else
    {
        kStartGraphicModeTest();
    }
}

/**
 *  Application Processor용 C 언어 커널 엔트리 포인트
 *      대부분의 자료구조는 Bootstrap Processor가 생성해 놓았으므로 코어에 설정하는
 *      작업만 함
 */
void MainForApplicationProcessor( void )
{
    QWORD qwTickCount;

    // GDT 테이블을 설정
    kLoadGDTR( GDTR_STARTADDRESS );

    // TSS 디스크립터를 설정. TSS 세그먼트와 디스크립터를 Application Processor의 
    // 수만큼 생성했으므로, APIC ID를 이용하여 TSS 디스크립터를 할당
    kLoadTR( GDT_TSSSEGMENT + ( kGetAPICID() * sizeof( GDTENTRY16 ) ) );

    // IDT 테이블을 설정
    kLoadIDTR( IDTR_STARTADDRESS );
    
    // 스케줄러 초기화
    kInitializeScheduler();
    
    // 현재 코어의 로컬 APIC를 활성화
    kEnableSoftwareLocalAPIC();

    // 모든 인터럽트를 수신할 수 있도록 태스크 우선 순위 레지스터를 0으로 설정
    kSetTaskPriority( 0 );

    // 로컬 APIC의 로컬 벡터 테이블을 초기화
    kInitializeLocalVectorTable();

    // 인터럽트를 활성화
    kEnableInterrupt();    

    // 대칭 I/O 모드 테스트를 위해 Application Processor가 시작한 후 한번만 출력
    kPrintf( "Application Processor[APIC ID: %d] Is Activated\n",
            kGetAPICID() );

    // 유휴 태스크 실행
    kIdleTask();
}

// x를 절대값으로 변환하는 매크로
#define ABS( x )    ( ( ( x ) >= 0 ) ? ( x ) : -( x ) )

/**
 *  임의의 X, Y 좌표를 반환
 */
void kGetRandomXY( int* piX, int* piY )
{
    int iRandom;
    
    // X좌표를 계산
    iRandom = kRandom();
    *piX = ABS( iRandom ) % 1000;
    
    // Y좌표를 계산
    iRandom = kRandom();
    *piY = ABS( iRandom ) % 700;
}

/**
 *  임의의 색을 반환
 */
COLOR kGetRandomColor( void )
{
    int iR, iG, iB;
    int iRandom;

    iRandom = kRandom();
    iR = ABS( iRandom ) % 256;

    iRandom = kRandom();
    iG = ABS( iRandom ) % 256;
    
    iRandom = kRandom();
    iB = ABS( iRandom ) % 256;
    
    return RGB( iR, iG, iB );
}

/**
 *  윈도우 프레임을 그림
 */
void kDrawWindowFrame( int iX, int iY, int iWidth, int iHeight, const char* pcTitle )
{
    char* pcTestString1 = "This is MINT64 OS's window prototype~!!!";
    char* pcTestString2 = "Coming soon~!!!";
    VBEMODEINFOBLOCK* pstVBEMode;
    COLOR* pstVideoMemory;
    RECT stScreenArea;

    // VBE 모드 정보 블록을 반환
    pstVBEMode = kGetVBEModeInfoBlock();
    
    // 화면 영역 설정
    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;
    
    // 그래픽 메모리 어드레스 설정
    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );
    
    // 윈도우 프레임의 가장자리를 그림, 2 픽셀 두께
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX, iY, iX + iWidth, iY + iHeight, RGB( 109, 218, 22 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, RGB( 109, 218, 22 ),
            FALSE );

    // 제목 표시줄을 채움
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX, iY + 3, iX + iWidth - 1, iY + 21, RGB( 79, 204, 11 ), TRUE );

    // 윈도우 제목을 표시
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            iX + 6, iY + 3, RGB( 255, 255, 255 ), RGB( 79, 204, 11 ),
            pcTitle, kStrLen( pcTitle ) );
    
    // 제목 표시줄을 입체로 보이게 위쪽의 선을 그림, 2 픽셀 두께
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 1, iX + iWidth - 1, iY + 1, RGB( 183, 249, 171 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 2, iX + iWidth - 1, iY + 2, RGB( 150, 210, 140 ) );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 1, iY + 2, iX + 1, iY + 20, RGB( 183, 249, 171 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 2, iX + 2, iY + 20, RGB( 150, 210, 140 ) );
    
    // 제목 표시줄의 아래쪽에 선을 그림
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 19, iX + iWidth - 2, iY + 19, RGB( 46, 59, 30 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 20, iX + iWidth - 2, iY + 20, RGB( 46, 59, 30 ) );

    // 닫기 버튼을 그림, 오른쪽 상단에 표시
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2, iY + 19,
            RGB( 255, 255, 255 ), TRUE );

    // 닫기 버튼을 입체로 보이게 선을 그림, 2 픽셀 두께로 그림
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2, iY + 1, iX + iWidth - 2, iY + 19 - 1,
            RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 1, iY + 1, iX + iWidth - 2 - 1, iY + 19 - 1,
            RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 1, iY + 19, iX + iWidth - 2, iY + 19,
            RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 1, iY + 19 - 1, iX + iWidth - 2, iY + 19 - 1,
            RGB( 86, 86, 86 ), TRUE );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 1, iY + 1,
            RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1 + 1, iX + iWidth - 2 - 2, iY + 1 + 1,
            RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 18, iY + 19,
            RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 1, iY + 1, iX + iWidth - 2 - 18 + 1, iY + 19 - 1,
            RGB( 229, 229, 229 ) );
    
    // 대각선 X를 그림, 3 픽셀로 그림
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 4, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 5, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 5, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 1 + 5, iX + iWidth - 2 - 5, iY + 19 - 4, 
            RGB( 71, 199, 21 ) );
    
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 4, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 5, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 5, 
            RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 
            iX + iWidth - 2 - 18 + 4, iY + 19 - 5, iX + iWidth - 2 - 5, iY + 1 + 4, 
            RGB( 71, 199, 21 ) );


    // 내부를 그림
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 
            iX + 2, iY + 21, iX + iWidth - 2, iY + iHeight - 2, 
            RGB( 255, 255, 255 ), TRUE );
    
    // 테스트 문자 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            iX + 10, iY + 30, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTestString1,
            kStrLen( pcTestString1 ) );
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            iX + 10, iY + 50, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTestString2,
            kStrLen( pcTestString2 ) );
}

/**
 *  그래픽 모드를 테스트하는 함수
 */
void kStartGraphicModeTest()
{
    VBEMODEINFOBLOCK* pstVBEMode;
    int iX1, iY1, iX2, iY2;    
    COLOR stColor1, stColor2;
    int i;
    char* vpcString[] = { "Pixel", "Line", "Rectangle", "Circle", "MINT64 OS~!!!" };
    COLOR* pstVideoMemory;
    RECT stScreenArea;

    // VBE 모드 정보 블록을 반환
    pstVBEMode = kGetVBEModeInfoBlock();
    
    // 화면 영역 설정
    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;
    
    // 그래픽 메모리 어드레스 설정
    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );
    
    //==========================================================================
    // 점, 선, 사각형, 원, 그리고 문자를 간단히 출력
    //==========================================================================
    // (0, 0)에 Pixel이란 문자열을 검은색 바탕에 흰색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 0, RGB( 255, 255, 255), RGB( 0, 0, 0 ), vpcString[ 0 ], 
            kStrLen( vpcString[ 0 ] ) );
    // 픽셀을 (1, 20), (2, 20)에 흰색으로 출력
    kInternalDrawPixel( &stScreenArea, pstVideoMemory, 1, 20, RGB( 255, 255, 255 ) );
    kInternalDrawPixel( &stScreenArea, pstVideoMemory, 2, 20, RGB( 255, 255, 255 ) );
    
    // (0, 25)에 Line이란 문자열을 검은색 바탕에 빨간색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 25, RGB( 255, 0, 0), RGB( 0, 0, 0 ), vpcString[ 1 ], 
            kStrLen( vpcString[ 1 ] ) );
    // (20, 50)을 중심으로 (1000, 50) (1000, 100), (1000, 150), (1000, 200), 
    // (1000, 250)까지 빨간색으로 출력
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 50, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 100, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 150, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 200, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 250, RGB( 255, 0, 0 ) );
    
    // (0, 180)에 Rectangle이란 문자열을 검은색 바탕에 녹색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 180, RGB( 0, 255, 0), RGB( 0, 0, 0 ), vpcString[ 2 ], 
            kStrLen( vpcString[ 2 ] ) );
    // (20, 200)에서 시작하여 길이가 각각 50, 100, 150, 200인 사각형을 녹색으로 출력
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 20, 200, 70, 250, RGB( 0, 255, 0 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 120, 200, 220, 300, RGB( 0, 255, 0 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 270, 200, 420, 350, RGB( 0, 255, 0 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 470, 200, 670, 400, RGB( 0, 255, 0 ), TRUE );
    
    // (0, 550)에 Circle이란 문자열을 검은색 바탕에 파란색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 
            0, 550, RGB( 0, 0, 255), RGB( 0, 0, 0 ), vpcString[ 3 ], 
            kStrLen( vpcString[ 3 ] ) );
    // (45, 600)에서 시작하여 반지름이 25, 50, 75, 100인 원을 파란색으로 출력
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 45, 600, 25, RGB( 0, 0, 255 ), FALSE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 170, 600, 50, RGB( 0, 0, 255 ), TRUE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 345, 600, 75, RGB( 0, 0, 255 ), FALSE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 570, 600, 100, RGB( 0, 0, 255 ), TRUE ) ;
    
    // 키 입력 대기
    kGetCh();
    
    //==========================================================================
    // 점, 선, 사각형, 원, 그리고 문자를 무작위로 출력
    //==========================================================================
    // q 키가 입력될 때까지 아래를 반복
    do
    {
        // 점 그리기
        for( i = 0 ; i < 100 ; i++ )
        {
            // 임의의 X좌표와 색을 반환
            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();
            
            // 점 그리기
            kInternalDrawPixel( &stScreenArea, pstVideoMemory, iX1, iY1, stColor1 );
        }        
        
        // 선 그리기
        for( i = 0 ; i < 100 ; i++ )
        {
            // 임의의 X좌표와 색을 반환
            kGetRandomXY( &iX1, &iY1 );
            kGetRandomXY( &iX2, &iY2 );
            stColor1 = kGetRandomColor();
            
            // 선 그리기
            kInternalDrawLine( &stScreenArea, pstVideoMemory, iX1, iY1, iX2, iY2, stColor1 );
        }

        // 사각형 그리기
        for( i = 0 ; i < 20 ; i++ )
        {
            // 임의의 X좌표와 색을 반환
            kGetRandomXY( &iX1, &iY1 );
            kGetRandomXY( &iX2, &iY2 );
            stColor1 = kGetRandomColor();

            // 사각형 그리기
            kInternalDrawRect( &stScreenArea, pstVideoMemory, 
                    iX1, iY1, iX2, iY2, stColor1, kRandom() % 2 );
        }
        
        // 원 그리기
        for( i = 0 ; i < 100 ; i++ )
        {
            // 임의의 X좌표와 색을 반환
            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();

            // 원 그리기
            kInternalDrawCircle( &stScreenArea, pstVideoMemory, 
                    iX1, iY1, ABS( kRandom() % 50 + 1 ), stColor1, kRandom() % 2 );
        }
        
        // 텍스트 표시
        for( i = 0 ; i < 100 ; i++ )
        {
            // 임의의 X좌표와 색을 반환
            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();
            stColor2 = kGetRandomColor();
            
            // 텍스트 출력
            kInternalDrawText( &stScreenArea, pstVideoMemory, 
                    iX1, iY1, stColor1, stColor2, vpcString[ 4 ], 
                    kStrLen( vpcString[ 4 ] ) );
        }
    } while( kGetCh() != 'q' );
    
    //==========================================================================
    // 윈도우 프로토타입을 출력
    //==========================================================================
    // q 키를 눌러서 빠져 나왔다면 윈도우 프로토타입을 표시함
    while( 1 )
    {
        // 배경을 출력
        kInternalDrawRect( &stScreenArea, pstVideoMemory, 
                stScreenArea.iX1, stScreenArea.iY1, stScreenArea.iX2,
                stScreenArea.iY2, RGB( 232, 255, 232 ), TRUE );

        // 윈도우 프레임을 3개 그림
        for( i = 0 ; i < 3 ; i++ )
        {
            kGetRandomXY( &iX1, &iY1 );
            kDrawWindowFrame( iX1, iY1, 400, 200, "MINT64 OS Test Window" );
        }

        kGetCh();
    }
}
