/**
 *  file    ConsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   콘솔 셸에 관련된 소스 파일
 */

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "SerialPort.h"
#include "MPConfigurationTable.h"
#include "LocalAPIC.h"
#include "MultiProcessor.h"
#include "IOAPIC.h"
#include "InterruptHandler.h"
#include "VBE.h"
#include "SystemCall.h"

// 커맨드 테이블 정의
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
        { "help", "Show Help", kHelp },
        { "cls", "Clear Screen", kCls },
        { "totalram", "Show Total RAM Size", kShowTotalRAMSize },
        { "shutdown", "Shutdown And Reboot OS", kShutdown },
        { "cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed },
        { "date", "Show Date And Time", kShowDateAndTime },
        { "changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)",
                kChangeTaskPriority },
        { "tasklist", "Show Task List", kShowTaskList },
        { "killtask", "End Task, ex)killtask 1(ID) or 0xffffffff(All Task)", kKillTask },
        { "cpuload", "Show Processor Load", kCPULoad },
        { "showmatrix", "Show Matrix Screen", kShowMatrix },
        { "dynamicmeminfo", "Show Dyanmic Memory Information", kShowDyanmicMemoryInformation },        
        { "hddinfo", "Show HDD Information", kShowHDDInformation },
        { "readsector", "Read HDD Sector, ex)readsector 0(LBA) 10(count)", 
                kReadSector },
        { "writesector", "Write HDD Sector, ex)writesector 0(LBA) 10(count)", 
                kWriteSector },
        { "mounthdd", "Mount HDD", kMountHDD },
        { "formathdd", "Format HDD", kFormatHDD },
        { "filesysteminfo", "Show File System Information", kShowFileSystemInformation },
        { "createfile", "Create File, ex)createfile a.txt", kCreateFileInRootDirectory },
        { "deletefile", "Delete File, ex)deletefile a.txt", kDeleteFileInRootDirectory },
        { "dir", "Show Directory", kShowRootDirectory },
        { "writefile", "Write Data To File, ex) writefile a.txt", kWriteDataToFile },
        { "readfile", "Read Data From File, ex) readfile a.txt", kReadDataFromFile },
        { "flush", "Flush File System Cache", kFlushCache },
        { "download", "Download Data From Serial, ex) download a.txt", kDownloadFile },
        { "showmpinfo", "Show MP Configuration Table Information", kShowMPConfigurationTable },
        { "showirqintinmap", "Show IRQ->INITIN Mapping Table", kShowIRQINTINMappingTable },
        { "showintproccount", "Show Interrupt Processing Count", kShowInterruptProcessingCount },
        { "changeaffinity", "Change Task Affinity, ex)changeaffinity 1(ID) 0xFF(Affinity)",
                kChangeTaskAffinity },
        { "vbemodeinfo", "Show VBE Mode Information", kShowVBEModeInfo },
        { "testsystemcall", "Test System Call Operation", kTestSystemCall },
};

//==============================================================================
//  실제 셸을 구성하는 코드
//==============================================================================
/**
 *  셸의 메인 루프
 */
void kStartConsoleShell( void )
{
    char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;
    CONSOLEMANAGER* pstConsoleManager;
    
    // 콘솔을 관리하는 자료구조를 반환
    pstConsoleManager = kGetConsoleManager();
    
    // 프롬프트 출력
    kPrintf( CONSOLESHELL_PROMPTMESSAGE );
    
    // 콘솔 셸 종료 플래그가 TRUE가 될 때까지 반복
    while( pstConsoleManager->bExit == FALSE )
    {
        bKey = kGetCh();

        // 콘솔 셸 종료 플래그가 설정된 경우 루프를 종료
        if( pstConsoleManager->bExit == TRUE )
        {
            break;
        }
        
        if( bKey == KEY_BACKSPACE )
        {
            if( iCommandBufferIndex > 0 )
            {
                // 현재 커서 위치를 얻어서 한 문자 앞으로 이동한 다음 공백을 출력하고 
                // 커맨드 버퍼에서 마지막 문자 삭제
                kGetCursor( &iCursorX, &iCursorY );
                kPrintStringXY( iCursorX - 1, iCursorY, " " );
                kSetCursor( iCursorX - 1, iCursorY );
                iCommandBufferIndex--;
            }
        }
        else if( bKey == KEY_ENTER )
        {
            kPrintf( "\n" );
            
            if( iCommandBufferIndex > 0 )
            {
                // 커맨드 버퍼에 있는 명령을 실행
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
                kExecuteCommand( vcCommandBuffer );
            }
            
            // 프롬프트 출력 및 커맨드 버퍼 초기화
            kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
            kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
        }
        // 시프트 키, CAPS Lock, NUM Lock, Scroll Lock은 무시
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
            ;
        }
        else if( bKey < 128 )
        {
            // TAB은 공백으로 전환
            if( bKey == KEY_TAB )
            {
                bKey = ' ';
            }
            
            // 버퍼가 남아있을 때만 가능
            if( iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
            {
                vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
                kPrintf( "%c", bKey );
            }
        }
    }
}

/*
 *  커맨드 버퍼에 있는 커맨드를 비교하여 해당 커맨드를 처리하는 함수를 수행
 */
void kExecuteCommand( const char* pcCommandBuffer )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    // 공백으로 구분된 커맨드를 추출
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    // 커맨드 테이블을 검사해서 동일한 이름의 커맨드가 있는지 확인
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        // 커맨드의 길이와 내용이 완전히 일치하는지 검사
        if( ( iCommandLength == iSpaceIndex ) &&
            ( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
                       iSpaceIndex ) == 0 ) )
        {
            gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
            break;
        }
    }

    // 리스트에서 찾을 수 없다면 에러 출력
    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
    }
}

/**
 *  파라미터 자료구조를 초기화
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  공백으로 구분된 파라미터의 내용과 길이를 반환
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // 더 이상 파라미터가 없으면 나감
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // 버퍼의 길이만큼 이동하면서 공백을 검색
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // 파라미터를 복사하고 길이를 반환
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // 파라미터의 위치 업데이트
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  커맨드를 처리하는 코드
//==============================================================================
/**
 *  셸 도움말을 출력
 */
static void kHelp( const char* pcCommandBuffer )
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
    kPrintf( "=========================================================\n" );
    kPrintf( "                    MINT64 Shell Help                    \n" );
    kPrintf( "=========================================================\n" );
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

    // 가장 긴 커맨드의 길이를 계산
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // 도움말 출력
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );

        // 목록이 많을 경우 나눠서 보여줌
        if( ( i != 0 ) && ( ( i % 20 ) == 0 ) )
        {
            kPrintf( "Press any key to continue... ('q' is exit) : " );
            if( kGetCh() == 'q' )
            {
                kPrintf( "\n" );
                break;
            }        
            kPrintf( "\n" );
        }
    }
}

/**
 *  화면을 지움 
 */
static void kCls( const char* pcParameterBuffer )
{
    // 맨 윗줄은 디버깅 용으로 사용하므로 화면을 지운 후, 라인 1로 커서 이동
    kClearScreen();
    kSetCursor( 0, 1 );
}

/**
 *  총 메모리 크기를 출력
 */
static void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  PC를 재시작(Reboot)
 */
static void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // 파일 시스템 캐시에 들어있는 내용을 하드 디스크로 옮김
    kPrintf( "Cache Flush... ");
    if( kFlushFileSystemCache() == TRUE )
    {
        kPrintf( "Pass\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
    
    // 키보드 컨트롤러를 통해 PC를 재시작
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

/**
 *  프로세서의 속도를 측정
 */
static void kMeasureProcessorSpeed( const char* pcParameterBuffer )
{
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;
        
    kPrintf( "Now Measuring." );
    
    // 10초 동안 변화한 타임 스탬프 카운터를 이용하여 프로세서의 속도를 간접적으로 측정
    kDisableInterrupt();    
    for( i = 0 ; i < 200 ; i++ )
    {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT( MSTOCOUNT( 50 ) );
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf( "." );
    }
    // 타이머 복원
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );
    kEnableInterrupt();
    
    kPrintf( "\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000 );
}

/**
 *  RTC 컨트롤러에 저장된 일자 및 시간 정보를 표시
 */
static void kShowDateAndTime( const char* pcParameterBuffer )
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    // RTC 컨트롤러에서 시간 및 일자를 읽음
    kReadRTCTime( &bHour, &bMinute, &bSecond );
    kReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    kPrintf( "Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth,
             kConvertDayOfWeekToString( bDayOfWeek ) );
    kPrintf( "Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}

/**
 *  태스크의 우선 순위를 변경
 */
static void kChangeTaskPriority( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    char vcPriority[ 30 ];
    QWORD qwID;
    BYTE bPriority;
    
    // 파라미터를 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    kGetNextParameter( &stList, vcPriority );
    
    // 태스크의 우선 순위를 변경
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    
    bPriority = kAToI( vcPriority, 10 );
    
    kPrintf( "Change Task Priority ID [0x%q] Priority[%d] ", qwID, bPriority );
    if( kChangePriority( qwID, bPriority ) == TRUE )
    {
        kPrintf( "Success\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
}

/**
 *  현재 생성된 모든 태스크의 정보를 출력
 */
static void kShowTaskList( const char* pcParameterBuffer )
{
    int i;
    TCB* pstTCB;
    int iCount = 0;
    int iTotalTaskCount = 0;
    char vcBuffer[ 20 ];
    int iRemainLength;
    int iProcessorCount;
    
    // 코어 수만큼 루프를 돌면서 각 스케줄러에 있는 태스크의 수를 더함 
    iProcessorCount = kGetProcessorCount(); 
    
    for( i = 0 ; i < iProcessorCount ; i++ )
    {
        iTotalTaskCount += kGetTaskCount( i );
    }
    
    kPrintf( "================= Task Total Count [%d] =================\n", 
             iTotalTaskCount );
    
    // 코어가 2개 이상이면 각 스케줄러 별로 개수를 출력
    if( iProcessorCount > 1 )
    {
        // 각 스케줄러 별로 태스크의 개수를 출력
        for( i = 0 ; i < iProcessorCount ; i++ )
        {
            if( ( i != 0 ) && ( ( i % 4 ) == 0 ) )
            {
                kPrintf( "\n" );
            }
            
            kSPrintf( vcBuffer, "Core %d : %d", i, kGetTaskCount( i ) );
            kPrintf( vcBuffer );
            
            // 출력하고 남은 공간을 모두 스페이스바로 채움
            iRemainLength = 19 - kStrLen( vcBuffer );
            kMemSet( vcBuffer, ' ', iRemainLength );
            vcBuffer[ iRemainLength ] = '\0';
            kPrintf( vcBuffer );
        }
        
        kPrintf( "\nPress any key to continue... ('q' is exit) : " );
        if( kGetCh() == 'q' )
        {
            kPrintf( "\n" );
            return ;
        }
        kPrintf( "\n\n" );
    }
    
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        // TCB를 구해서 TCB가 사용 중이면 ID를 출력
        pstTCB = kGetTCBInTCBPool( i );
        if( ( pstTCB->stLink.qwID >> 32 ) != 0 )
        {
            // 태스크가 6개 출력될 때마다, 계속 태스크 정보를 표시할지 여부를 확인
            if( ( iCount != 0 ) && ( ( iCount % 6 ) == 0 ) )
            {
                kPrintf( "Press any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    kPrintf( "\n" );
                    break;
                }
                kPrintf( "\n" );
            }
            
            kPrintf( "[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n", 1 + iCount++,
                     pstTCB->stLink.qwID, GETPRIORITY( pstTCB->qwFlags ), 
                     pstTCB->qwFlags, kGetListCount( &( pstTCB->stChildThreadList ) ) );
            kPrintf( "    Core ID[0x%X] CPU Affinity[0x%X]\n", pstTCB->bAPICID,
                     pstTCB->bAffinity );
            kPrintf( "    Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n",
                    pstTCB->qwParentProcessID, pstTCB->pvMemoryAddress, pstTCB->qwMemorySize );
        }
    }
}

/**
 *  태스크를 종료
 */
static void kKillTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    QWORD qwID;
    TCB* pstTCB;
    int i;
    
    // 파라미터를 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    
    // 태스크를 종료
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    
    // 특정 ID만 종료하는 경우
    if( qwID != 0xFFFFFFFF )
    {
        pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
        qwID = pstTCB->stLink.qwID;

        // 시스템 테스트는 제외
        if( ( ( qwID >> 32 ) != 0 ) && ( ( pstTCB->qwFlags & TASK_FLAGS_SYSTEM ) == 0x00 ) )
        {
            kPrintf( "Kill Task ID [0x%q] ", qwID );
            if( kEndTask( qwID ) == TRUE )
            {
                kPrintf( "Success\n" );
            }
            else
            {
                kPrintf( "Fail\n" );
            }
        }
        else
        {
            kPrintf( "Task does not exist or task is system task\n" );
        }
    }
    // 콘솔 셸과 유휴 태스크를 제외하고 모든 태스크 종료
    else
    {
        for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
        {
            pstTCB = kGetTCBInTCBPool( i );
            qwID = pstTCB->stLink.qwID;

            // 시스템 테스트는 삭제 목록에서 제외
            if( ( ( qwID >> 32 ) != 0 ) && ( ( pstTCB->qwFlags & TASK_FLAGS_SYSTEM ) == 0x00 ) )
            {
                kPrintf( "Kill Task ID [0x%q] ", qwID );
                if( kEndTask( qwID ) == TRUE )
                {
                    kPrintf( "Success\n" );
                }
                else
                {
                    kPrintf( "Fail\n" );
                }
            }
        }
    }
}

/**
 *  프로세서의 사용률을 표시
 */
static void kCPULoad( const char* pcParameterBuffer )
{
    int i;
    char vcBuffer[ 50 ];
    int iRemainLength;
    
    kPrintf( "================= Processor Load =================\n" ); 
    
    // 각 코어 별로 부하를 출력
    for( i = 0 ; i < kGetProcessorCount() ; i++ )
    {
        if( ( i != 0 ) && ( ( i % 4 ) == 0 ) )
        {
            kPrintf( "\n" );
        }
        
        kSPrintf( vcBuffer, "Core %d : %d%%", i, kGetProcessorLoad( i ) );
        kPrintf( "%s", vcBuffer );
        
        // 출력하고 남은 공간을 모두 스페이스바로 채움
        iRemainLength = 19 - kStrLen( vcBuffer );
        kMemSet( vcBuffer, ' ', iRemainLength );
        vcBuffer[ iRemainLength ] = '\0';
        kPrintf( vcBuffer );
    }
    kPrintf( "\n" );
}

// 난수를 발생시키기 위한 변수
static volatile QWORD gs_qwRandomValue = 0;

/**
 *  임의의 난수를 반환
 */
QWORD kRandom( void )
{
    gs_qwRandomValue = ( gs_qwRandomValue * 412153 + 5571031 ) >> 16;
    return gs_qwRandomValue;
}

/**
 *  철자를 흘러내리게 하는 스레드
 */
static void kDropCharactorThread( void )
{
    int iX, iY;
    int i;
    char vcText[ 2 ] = { 0, };

    iX = kRandom() % CONSOLE_WIDTH;
    
    while( 1 )
    {
        // 잠시 대기함
        kSleep( kRandom() % 20 );
        
        if( ( kRandom() % 20 ) < 16 )
        {
            vcText[ 0 ] = ' ';
            for( i = 0 ; i < CONSOLE_HEIGHT - 1 ; i++ )
            {
                kPrintStringXY( iX, i , vcText );
                kSleep( 50 );
            }
        }        
        else
        {
            for( i = 0 ; i < CONSOLE_HEIGHT - 1 ; i++ )
            {
                vcText[ 0 ] = ( i + kRandom() ) % 128;
                kPrintStringXY( iX, i, vcText );
                kSleep( 50 );
            }
        }
    }
}

/**
 *  스레드를 생성하여 매트릭스 화면처럼 보여주는 프로세스
 */
static void kMatrixProcess( void )
{
    int i;
    
    for( i = 0 ; i < 300 ; i++ )
    {
        if( kCreateTask( TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0, 
            ( QWORD ) kDropCharactorThread, TASK_LOADBALANCINGID ) == NULL )
        {
            break;
        }
        
        kSleep( kRandom() % 5 + 5 );
    }
    
    kPrintf( "%d Thread is created\n", i );

    // 키가 입력되면 프로세스 종료
    kGetCh();
}

/**
 *  매트릭스 화면을 보여줌
 */
static void kShowMatrix( const char* pcParameterBuffer )
{
    TCB* pstProcess;
    
    pstProcess = kCreateTask( TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, ( void* ) 0xE00000, 0xE00000, 
                              ( QWORD ) kMatrixProcess, TASK_LOADBALANCINGID );
    if( pstProcess != NULL )
    {
        kPrintf( "Matrix Process [0x%Q] Create Success\n" );

        // 태스크가 종료 될 때까지 대기
        while( ( pstProcess->stLink.qwID >> 32 ) != 0 )
        {
            kSleep( 100 );
        }
    }
    else
    {
        kPrintf( "Matrix Process Create Fail\n" );
    }
}

/**
 *  동적 메모리 정보를 표시
 */
static void kShowDyanmicMemoryInformation( const char* pcParameterBuffer )
{
    QWORD qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize;
    
    kGetDynamicMemoryInformation( &qwStartAddress, &qwTotalSize, &qwMetaSize, 
            &qwUsedSize );

    kPrintf( "============ Dynamic Memory Information ============\n" );
    kPrintf( "Start Address: [0x%Q]\n", qwStartAddress );
    kPrintf( "Total Size:    [0x%Q]byte, [%d]MB\n", qwTotalSize, 
            qwTotalSize / 1024 / 1024 );
    kPrintf( "Meta Size:     [0x%Q]byte, [%d]KB\n", qwMetaSize, 
            qwMetaSize / 1024 );
    kPrintf( "Used Size:     [0x%Q]byte, [%d]KB\n", qwUsedSize, qwUsedSize / 1024 );
}

/**
 *  하드 디스크의 정보를 표시
 */
static void kShowHDDInformation( const char* pcParameterBuffer )
{
    HDDINFORMATION stHDD;
    char vcBuffer[ 100 ];
    
    // 하드 디스크의 정보를 읽음
    if( kGetHDDInformation( &stHDD ) == FALSE )
    {
        kPrintf( "HDD Information Read Fail\n" );
        return ;
    }        
    
    kPrintf( "============ Primary Master HDD Information ============\n" );
    
    // 모델 번호 출력
    kMemCpy( vcBuffer, stHDD.vwModelNumber, sizeof( stHDD.vwModelNumber ) );
    vcBuffer[ sizeof( stHDD.vwModelNumber ) - 1 ] = '\0';
    kPrintf( "Model Number:\t %s\n", vcBuffer );
    
    // 시리얼 번호 출력
    kMemCpy( vcBuffer, stHDD.vwSerialNumber, sizeof( stHDD.vwSerialNumber ) );
    vcBuffer[ sizeof( stHDD.vwSerialNumber ) - 1 ] = '\0';
    kPrintf( "Serial Number:\t %s\n", vcBuffer );

    // 헤드, 실린더, 실린더 당 섹터 수를 출력
    kPrintf( "Head Count:\t %d\n", stHDD.wNumberOfHead );
    kPrintf( "Cylinder Count:\t %d\n", stHDD.wNumberOfCylinder );
    kPrintf( "Sector Count:\t %d\n", stHDD.wNumberOfSectorPerCylinder );
    
    // 총 섹터 수 출력
    kPrintf( "Total Sector:\t %d Sector, %dMB\n", stHDD.dwTotalSectors, 
            stHDD.dwTotalSectors / 2 / 1024 );
}

/**
 *  하드 디스크에 파라미터로 넘어온 LBA 어드레스에서 섹터 수 만큼 읽음
 */
static void kReadSector( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcLBA[ 50 ], vcSectorCount[ 50 ];
    DWORD dwLBA;
    int iSectorCount;
    char* pcBuffer;
    int i, j;
    BYTE bData;
    BOOL bExit = FALSE;
    
    // 파라미터 리스트를 초기화하여 LBA 어드레스와 섹터 수 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    if( ( kGetNextParameter( &stList, vcLBA ) == 0 ) ||
        ( kGetNextParameter( &stList, vcSectorCount ) == 0 ) )
    {
        kPrintf( "ex) readsector 0(LBA) 10(count)\n" );
        return ;
    }
    dwLBA = kAToI( vcLBA, 10 );
    iSectorCount = kAToI( vcSectorCount, 10 );
    
    // 섹터 수만큼 메모리를 할당 받아 읽기 수행
    pcBuffer = kAllocateMemory( iSectorCount * 512 );
    if( kReadHDDSector( TRUE, TRUE, dwLBA, iSectorCount, pcBuffer ) == iSectorCount )
    {
        kPrintf( "LBA [%d], [%d] Sector Read Success~!!", dwLBA, iSectorCount );
        // 데이터 버퍼의 내용을 출력
        for( j = 0 ; j < iSectorCount ; j++ )
        {
            for( i = 0 ; i < 512 ; i++ )
            {
                if( !( ( j == 0 ) && ( i == 0 ) ) && ( ( i % 256 ) == 0 ) )
                {
                    kPrintf( "\nPress any key to continue... ('q' is exit) : " );
                    if( kGetCh() == 'q' )
                    {
                        bExit = TRUE;
                        break;
                    }
                }                

                if( ( i % 16 ) == 0 )
                {
                    kPrintf( "\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i ); 
                }

                // 모두 두 자리로 표시하려고 16보다 작은 경우 0을 추가해줌
                bData = pcBuffer[ j * 512 + i ] & 0xFF;
                if( bData < 16 )
                {
                    kPrintf( "0" );
                }
                kPrintf( "%X ", bData );
            }
            
            if( bExit == TRUE )
            {
                break;
            }
        }
        kPrintf( "\n" );
    }
    else
    {
        kPrintf( "Read Fail\n" );
    }
    
    kFreeMemory( pcBuffer );
}

/**
 *  하드 디스크에 파라미터로 넘어온 LBA 어드레스에서 섹터 수 만큼 씀
 */
static void kWriteSector( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcLBA[ 50 ], vcSectorCount[ 50 ];
    DWORD dwLBA;
    int iSectorCount;
    char* pcBuffer;
    int i, j;
    BOOL bExit = FALSE;
    BYTE bData;
    static DWORD s_dwWriteCount = 0;

    // 파라미터 리스트를 초기화하여 LBA 어드레스와 섹터 수 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    if( ( kGetNextParameter( &stList, vcLBA ) == 0 ) ||
        ( kGetNextParameter( &stList, vcSectorCount ) == 0 ) )
    {
        kPrintf( "ex) writesector 0(LBA) 10(count)\n" );
        return ;
    }
    dwLBA = kAToI( vcLBA, 10 );
    iSectorCount = kAToI( vcSectorCount, 10 );

    s_dwWriteCount++;
    
    // 버퍼를 할당 받아 데이터를 채움. 
    // 패턴은 4 바이트의 LBA 어드레스와 4 바이트의 쓰기가 수행된 횟수로 생성
    pcBuffer = kAllocateMemory( iSectorCount * 512 );
    for( j = 0 ; j < iSectorCount ; j++ )
    {
        for( i = 0 ; i < 512 ; i += 8 )
        {
            *( DWORD* ) &( pcBuffer[ j * 512 + i ] ) = dwLBA + j;
            *( DWORD* ) &( pcBuffer[ j * 512 + i + 4 ] ) = s_dwWriteCount;            
        }
    }
    
    // 쓰기 수행
    if( kWriteHDDSector( TRUE, TRUE, dwLBA, iSectorCount, pcBuffer ) != iSectorCount )
    {
        kPrintf( "Write Fail\n" );
        return ;
    }
    kPrintf( "LBA [%d], [%d] Sector Write Success~!!", dwLBA, iSectorCount );

    // 데이터 버퍼의 내용을 출력
    for( j = 0 ; j < iSectorCount ; j++ )
    {
        for( i = 0 ; i < 512 ; i++ )
        {
            if( !( ( j == 0 ) && ( i == 0 ) ) && ( ( i % 256 ) == 0 ) )
            {
                kPrintf( "\nPress any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    bExit = TRUE;
                    break;
                }
            }                

            if( ( i % 16 ) == 0 )
            {
                kPrintf( "\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i ); 
            }

            // 모두 두 자리로 표시하려고 16보다 작은 경우 0을 추가해줌
            bData = pcBuffer[ j * 512 + i ] & 0xFF;
            if( bData < 16 )
            {
                kPrintf( "0" );
            }
            kPrintf( "%X ", bData );
        }
        
        if( bExit == TRUE )
        {
            break;
        }
    }
    kPrintf( "\n" );    
    kFreeMemory( pcBuffer );    
}

/**
 *  하드 디스크를 연결
 */
static void kMountHDD( const char* pcParameterBuffer )
{
    if( kMount() == FALSE )
    {
        kPrintf( "HDD Mount Fail\n" );
        return ;
    }
    kPrintf( "HDD Mount Success\n" );
}

/**
 *  하드 디스크에 파일 시스템을 생성(포맷)
 */
static void kFormatHDD( const char* pcParameterBuffer )
{
    if( kFormat() == FALSE )
    {
        kPrintf( "HDD Format Fail\n" );
        return ;
    }
    kPrintf( "HDD Format Success\n" );
}

/**
 *  파일 시스템 정보를 표시
 */
static void kShowFileSystemInformation( const char* pcParameterBuffer )
{
    FILESYSTEMMANAGER stManager;
    
    kGetFileSystemInformation( &stManager );
    
    kPrintf( "================== File System Information ==================\n" );
    kPrintf( "Mouted:\t\t\t\t\t %d\n", stManager.bMounted );
    kPrintf( "Reserved Sector Count:\t\t\t %d Sector\n", stManager.dwReservedSectorCount );
    kPrintf( "Cluster Link Table Start Address:\t %d Sector\n", 
            stManager.dwClusterLinkAreaStartAddress );
    kPrintf( "Cluster Link Table Size:\t\t %d Sector\n", stManager.dwClusterLinkAreaSize );
    kPrintf( "Data Area Start Address:\t\t %d Sector\n", stManager.dwDataAreaStartAddress );
    kPrintf( "Total Cluster Count:\t\t\t %d Cluster\n", stManager.dwTotalClusterCount );
}

/**
 *  루트 디렉터리에 빈 파일을 생성
 */
static void kCreateFileInRootDirectory( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    DWORD dwCluster;
    int i;
    FILE* pstFile;
    
    // 파라미터 리스트를 초기화하여 파일 이름을 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }

    pstFile = fopen( vcFileName, "w" );
    if( pstFile == NULL )
    {
        kPrintf( "File Create Fail\n" );
        return;
    }
    fclose( pstFile );
    kPrintf( "File Create Success\n" );
}

/**
 *  루트 디렉터리에서 파일을 삭제
 */
static void kDeleteFileInRootDirectory( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    
    // 파라미터 리스트를 초기화하여 파일 이름을 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }

    if( remove( vcFileName ) != 0 )
    {
        kPrintf( "File Not Found or File Opened\n" );
        return ;
    }
    
    kPrintf( "File Delete Success\n" );
}

/**
 *  루트 디렉터리의 파일 목록을 표시
 */
static void kShowRootDirectory( const char* pcParameterBuffer )
{
    DIR* pstDirectory;
    int i, iCount, iTotalCount;
    struct dirent* pstEntry;
    char vcBuffer[ 400 ];
    char vcTempValue[ 50 ];
    DWORD dwTotalByte;
    DWORD dwUsedClusterCount;
    FILESYSTEMMANAGER stManager;
    
    // 파일 시스템 정보를 얻음
    kGetFileSystemInformation( &stManager );
     
    // 루트 디렉터리를 엶
    pstDirectory = opendir( "/" );
    if( pstDirectory == NULL )
    {
        kPrintf( "Root Directory Open Fail\n" );
        return ;
    }
    
    // 먼저 루프를 돌면서 디렉터리에 있는 파일의 개수와 전체 파일이 사용한 크기를 계산
    iTotalCount = 0;
    dwTotalByte = 0;
    dwUsedClusterCount = 0;
    while( 1 )
    {
        // 디렉터리에서 엔트리 하나를 읽음
        pstEntry = readdir( pstDirectory );
        // 더이상 파일이 없으면 나감
        if( pstEntry == NULL )
        {
            break;
        }
        iTotalCount++;
        dwTotalByte += pstEntry->dwFileSize;

        // 실제로 사용된 클러스터의 개수를 계산
        if( pstEntry->dwFileSize == 0 )
        {
            // 크기가 0이라도 클러스터 1개는 할당되어 있음
            dwUsedClusterCount++;
        }
        else
        {
            // 클러스터 개수를 올림하여 더함
            dwUsedClusterCount += ( pstEntry->dwFileSize + 
                ( FILESYSTEM_CLUSTERSIZE - 1 ) ) / FILESYSTEM_CLUSTERSIZE;
        }
    }
    
    // 실제 파일의 내용을 표시하는 루프
    rewinddir( pstDirectory );
    iCount = 0;
    while( 1 )
    {
        // 디렉터리에서 엔트리 하나를 읽음
        pstEntry = readdir( pstDirectory );
        // 더이상 파일이 없으면 나감
        if( pstEntry == NULL )
        {
            break;
        }
        
        // 전부 공백으로 초기화 한 후 각 위치에 값을 대입
        kMemSet( vcBuffer, ' ', sizeof( vcBuffer ) - 1 );
        vcBuffer[ sizeof( vcBuffer ) - 1 ] = '\0';
        
        // 파일 이름 삽입
        kMemCpy( vcBuffer, pstEntry->d_name, 
                 kStrLen( pstEntry->d_name ) );

        // 파일 길이 삽입
        kSPrintf( vcTempValue, "%d Byte", pstEntry->dwFileSize );
        kMemCpy( vcBuffer + 30, vcTempValue, kStrLen( vcTempValue ) );

        // 파일의 시작 클러스터 삽입
        kSPrintf( vcTempValue, "0x%X Cluster", pstEntry->dwStartClusterIndex );
        kMemCpy( vcBuffer + 55, vcTempValue, kStrLen( vcTempValue ) + 1 );
        kPrintf( "    %s\n", vcBuffer );

        if( ( iCount != 0 ) && ( ( iCount % 20 ) == 0 ) )
        {
            kPrintf( "Press any key to continue... ('q' is exit) : " );
            if( kGetCh() == 'q' )
            {
                kPrintf( "\n" );
                break;
            }        
        }
        iCount++;
    }
    
    // 총 파일의 개수와 파일의 총 크기를 출력
    kPrintf( "\t\tTotal File Count: %d\n", iTotalCount );
    kPrintf( "\t\tTotal File Size: %d KByte (%d Cluster)\n", dwTotalByte, 
             dwUsedClusterCount );
    
    // 남은 클러스터 수를 이용해서 여유 공간을 출력
    kPrintf( "\t\tFree Space: %d KByte (%d Cluster)\n", 
             ( stManager.dwTotalClusterCount - dwUsedClusterCount ) * 
             FILESYSTEM_CLUSTERSIZE / 1024, stManager.dwTotalClusterCount - 
             dwUsedClusterCount );
    
    // 디렉터리를 닫음
    closedir( pstDirectory );
}

/**
 *  파일을 생성하여 키보드로 입력된 데이터를 씀
 */
static void kWriteDataToFile( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    FILE* fp;
    int iEnterCount;
    BYTE bKey;
    
    // 파라미터 리스트를 초기화하여 파일 이름을 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }
    
    // 파일 생성
    fp = fopen( vcFileName, "w" );
    if( fp == NULL )
    {
        kPrintf( "%s File Open Fail\n", vcFileName );
        return ;
    }
    
    // 엔터 키가 연속으로 3번 눌러질 때까지 내용을 파일에 씀
    iEnterCount = 0;
    while( 1 )
    {
        bKey = kGetCh();
        // 엔터 키이면 연속 3번 눌러졌는가 확인하여 루프를 빠져 나감
        if( bKey == KEY_ENTER )
        {
            iEnterCount++;
            if( iEnterCount >= 3 )
            {
                break;
            }
        }
        // 엔터 키가 아니라면 엔터 키 입력 횟수를 초기화
        else
        {
            iEnterCount = 0;
        }
        
        kPrintf( "%c", bKey );
        if( fwrite( &bKey, 1, 1, fp ) != 1 )
        {
            kPrintf( "File Wirte Fail\n" );
            break;
        }
    }
    
    kPrintf( "File Create Success\n" );
    fclose( fp );
}

/**
 *  파일을 열어서 데이터를 읽음
 */
static void kReadDataFromFile( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iLength;
    FILE* fp;
    int iEnterCount;
    BYTE bKey;
    
    // 파라미터 리스트를 초기화하여 파일 이름을 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    iLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iLength ] = '\0';
    if( ( iLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || ( iLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        return ;
    }
    
    // 파일 생성
    fp = fopen( vcFileName, "r" );
    if( fp == NULL )
    {
        kPrintf( "%s File Open Fail\n", vcFileName );
        return ;
    }
    
    // 파일의 끝까지 출력하는 것을 반복
    iEnterCount = 0;
    while( 1 )
    {
        if( fread( &bKey, 1, 1, fp ) != 1 )
        {
            break;
        }
        kPrintf( "%c", bKey );
        
        // 만약 엔터 키이면 엔터 키 횟수를 증가시키고 20라인까지 출력했다면 
        // 더 출력할지 여부를 물어봄
        if( bKey == KEY_ENTER )
        {
            iEnterCount++;
            
            if( ( iEnterCount != 0 ) && ( ( iEnterCount % 20 ) == 0 ) )
            {
                kPrintf( "Press any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    kPrintf( "\n" );
                    break;
                }
                kPrintf( "\n" );
                iEnterCount = 0;
            }
        }
    }
    fclose( fp );
}

/**
 *  파일 시스템의 캐시 버퍼에 있는 데이터를 모두 하드 디스크에 씀 
 */
static void kFlushCache( const char* pcParameterBuffer )
{
    QWORD qwTickCount;
    
    qwTickCount = kGetTickCount();
    kPrintf( "Cache Flush... ");
    if( kFlushFileSystemCache() == TRUE )
    {
        kPrintf( "Pass\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
    kPrintf( "Total Time = %d ms\n", kGetTickCount() - qwTickCount );
}

/**
 *  시리얼 포트로부터 데이터를 수신하여 파일로 저장
 */
static void kDownloadFile( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcFileName[ 50 ];
    int iFileNameLength;
    DWORD dwDataLength;
    FILE* fp;
    DWORD dwReceivedSize;
    DWORD dwTempSize;
    BYTE vbDataBuffer[ SERIAL_FIFOMAXSIZE ];
    QWORD qwLastReceivedTickCount;
    
    // 파라미터 리스트를 초기화하여 파일 이름을 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    iFileNameLength = kGetNextParameter( &stList, vcFileName );
    vcFileName[ iFileNameLength ] = '\0';
    if( ( iFileNameLength > ( FILESYSTEM_MAXFILENAMELENGTH - 1 ) ) || 
        ( iFileNameLength == 0 ) )
    {
        kPrintf( "Too Long or Too Short File Name\n" );
        kPrintf( "ex)download a.txt\n" );
        return ;
    }
    
    // 시리얼 포트의 FIFO를 모두 비움
    kClearSerialFIFO();
    
    //==========================================================================
    // 데이터 길이가 수신될 때까지 기다린다는 메시지를 출력하고, 4 바이트를 수신한 뒤
    // Ack를 전송
    //==========================================================================
    kPrintf( "Waiting For Data Length....." );
    dwReceivedSize = 0;
    qwLastReceivedTickCount = kGetTickCount();
    while( dwReceivedSize < 4 )
    {
        // 남은 수만큼 데이터 수신
        dwTempSize = kReceiveSerialData( ( ( BYTE* ) &dwDataLength ) +
            dwReceivedSize, 4 - dwReceivedSize );
        dwReceivedSize += dwTempSize;
        
        // 수신된 데이터가 없다면 잠시 대기
        if( dwTempSize == 0 )
        {
            kSleep( 0 );
            
            // 대기한 시간이 30초 이상이라면 Time Out으로 중지
            if( ( kGetTickCount() - qwLastReceivedTickCount ) > 30000 )
            {
                kPrintf( "Time Out Occur~!!\n" );
                return ;
            }
        }
        else
        {
            // 마지막으로 데이터를 수신한 시간을 갱신
            qwLastReceivedTickCount = kGetTickCount();
        }
    }
    kPrintf( "[%d] Byte\n", dwDataLength );

    // 정상적으로 데이터 길이를 수신했으므로, Ack를 송신
    kSendSerialData( "A", 1 );

    //==========================================================================
    // 파일을 생성하고 시리얼로부터 데이터를 수신하여 파일에 저장
    //==========================================================================
    // 파일 생성
    fp = fopen( vcFileName, "w" );
    if( fp == NULL )
    {
        kPrintf( "%s File Open Fail\n", vcFileName );
        return ;
    }
    
    // 데이터 수신
    kPrintf( "Data Receive Start: " );
    dwReceivedSize = 0;
    qwLastReceivedTickCount = kGetTickCount();
    while( dwReceivedSize < dwDataLength )
    {
        // 버퍼에 담아서 데이터를 씀
        dwTempSize = kReceiveSerialData( vbDataBuffer, SERIAL_FIFOMAXSIZE );
        dwReceivedSize += dwTempSize;

        // 이번에 데이터가 수신된 것이 있다면 ACK 또는 파일 쓰기 수행
        if( dwTempSize != 0 ) 
        {
            // 수신하는 쪽은 데이터의 마지막까지 수신했거나 FIFO의 크기인 
            // 16 바이트마다 한번씩 Ack를 전송
            if( ( ( dwReceivedSize % SERIAL_FIFOMAXSIZE ) == 0 ) ||
                ( ( dwReceivedSize == dwDataLength ) ) )
            {
                kSendSerialData( "A", 1 );
                kPrintf( "#" );
            }
            
            // 쓰기 중에 문제가 생기면 바로 종료
            if( fwrite( vbDataBuffer, 1, dwTempSize, fp ) != dwTempSize )
            {
                kPrintf( "File Write Error Occur\n" );
                break;
            }
            
            // 마지막으로 데이터를 수신한 시간을 갱신
            qwLastReceivedTickCount = kGetTickCount();
        }
        // 이번에 수신된 데이터가 없다면 잠시 대기
        else
        {
            kSleep( 0 );
            
            // 대기한 시간이 10초 이상이라면 Time Out으로 중지
            if( ( kGetTickCount() - qwLastReceivedTickCount ) > 10000 )
            {
                kPrintf( "Time Out Occur~!!\n" );
                break;
            }
        }
    }   

    //==========================================================================
    // 전체 데이터의 크기와 실제로 수신 받은 데이터의 크기를 비교하여 성공 여부를
    // 출력한 뒤, 파일을 닫고 파일 시스템 캐시를 모두 비움
    //==========================================================================
    // 수신된 길이를 비교해서 문제가 발생했는지를 표시
    if( dwReceivedSize != dwDataLength )
    {
        kPrintf( "\nError Occur. Total Size [%d] Received Size [%d]\n", 
                 dwReceivedSize, dwDataLength );
    }
    else
    {
        kPrintf( "\nReceive Complete. Total Size [%d] Byte\n", dwReceivedSize );
    }
    
    // 파일을 닫고 파일 시스템 캐시를 내보냄
    fclose( fp );
    kFlushFileSystemCache();
}

/**
 *  MP 설정 테이블 정보를 출력
 */
static void kShowMPConfigurationTable( const char* pcParameterBuffer )
{
    kPrintMPConfigurationTable();
}

/**
 *  IRQ와 I/O APIC의 인터럽트 입력 핀(INTIN)의 관계를 저장한 테이블을 표시
 */
static void kShowIRQINTINMappingTable( const char* pcParameterBuffer )
{
    // I/O APIC를 관리하는 자료구조에 있는 출력 함수를 호출
    kPrintIRQToINTINMap();
}

/**
 *  코어 별로 인터럽트를 처리한 횟수를 출력
 */
static void kShowInterruptProcessingCount( const char* pcParameterBuffer )
{
    INTERRUPTMANAGER* pstInterruptManager;
    int i;
    int j;
    int iProcessCount;
    char vcBuffer[ 20 ];
    int iRemainLength;
    int iLineCount;
    
    kPrintf( "========================== Interrupt Count ==========================\n" );
    
    // MP 설정 테이블에 저장된 코어의 개수를 읽음
    iProcessCount = kGetProcessorCount();
    
    //==========================================================================
    //  Column 출력
    //==========================================================================
    // 프로세서의 수만큼 Column을 출력
    // 한 줄에 코어 4개씩 출력하고 한 Column당 15칸을 할당함
    for( i = 0 ; i < iProcessCount ; i++ )
    {
        if( i == 0 )
        {
            kPrintf( "IRQ Num\t\t" );
        }
        else if( ( i % 4 ) == 0 )
        {
            kPrintf( "\n       \t\t" );
        }
        kSPrintf( vcBuffer, "Core %d", i );
        kPrintf( vcBuffer );
        
        // 출력하고 남은 공간을 모두 스페이스로 채움
        iRemainLength = 15 - kStrLen( vcBuffer );
        kMemSet( vcBuffer, ' ', iRemainLength );
        vcBuffer[ iRemainLength ] = '\0';
        kPrintf( vcBuffer );
    }
    kPrintf( "\n" );

    //==========================================================================
    //  Row와 인터럽트 처리 횟수 출력
    //==========================================================================
    // 총 인터럽트 횟수와 코어 별 인터럽트 처리 횟수를 출력
    iLineCount = 0;
    pstInterruptManager = kGetInterruptManager();
    for( i = 0 ; i < INTERRUPT_MAXVECTORCOUNT ; i++ )
    {
        for( j = 0 ; j < iProcessCount ; j++ )
        {
            // Row를 출력, 한 줄에 코어 4개씩 출력하고 한 Column당 15칸을 할당
            if( j == 0 )
            {
                // 20 라인마다 화면 정지
                if( ( iLineCount != 0 ) && ( iLineCount > 10 ) )
                {
                    kPrintf( "\nPress any key to continue... ('q' is exit) : " );
                    if( kGetCh() == 'q' )
                    {
                        kPrintf( "\n" );
                        return ;
                    }
                    iLineCount = 0;
                    kPrintf( "\n" );
                }
                kPrintf( "---------------------------------------------------------------------\n" );
                kPrintf( "IRQ %d\t\t", i );
                iLineCount += 2;
            }
            else if( ( j % 4 ) == 0 )
            {
                kPrintf( "\n      \t\t" );
                iLineCount++;
            }
            
            kSPrintf( vcBuffer, "0x%Q", pstInterruptManager->vvqwCoreInterruptCount[ j ][ i ] );
            // 출력하고 남은 영역을 모두 스페이스로 채움
            kPrintf( vcBuffer );
            iRemainLength = 15 - kStrLen( vcBuffer );
            kMemSet( vcBuffer, ' ', iRemainLength );
            vcBuffer[ iRemainLength ] = '\0';
            kPrintf( vcBuffer );
        }
        kPrintf( "\n" );
    }
}

/**
 *  태스크의 프로세서 친화도를 변경
 */
static void kChangeTaskAffinity( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    char vcAffinity[ 30 ];
    QWORD qwID;
    BYTE bAffinity;
    
    // 파라미터를 추출
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    kGetNextParameter( &stList, vcAffinity );
    
    // 태스크 ID 필드 추출
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    
    // 프로세서 친화도(Affinity) 추출
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        bAffinity = kAToI( vcAffinity + 2, 16 );
    }
    else
    {
        bAffinity = kAToI( vcAffinity, 10 );
    }
    
    kPrintf( "Change Task Affinity ID [0x%q] Affinity[0x%x] ", qwID, bAffinity );
    if( kChangeProcessorAffinity( qwID, bAffinity ) == TRUE )
    {
        kPrintf( "Success\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
}

/**
 *  VBE 모드 정보 블록을 출력
 */
static void kShowVBEModeInfo( const char* pcParameterBuffer )
{
    VBEMODEINFOBLOCK* pstModeInfo;
    
    // VBE 모드 정보 블록을 반환
    pstModeInfo = kGetVBEModeInfoBlock();
    
    // 해상도와 색 정보를 위주로 출력
    kPrintf( "VESA %x\n", pstModeInfo->wWinGranulity );
    kPrintf( "X Resolution: %d\n", pstModeInfo->wXResolution );
    kPrintf( "Y Resolution: %d\n", pstModeInfo->wYResolution );
    kPrintf( "Bits Per Pixel: %d\n", pstModeInfo->bBitsPerPixel );
    
    kPrintf( "Red Mask Size: %d, Field Position: %d\n", pstModeInfo->bRedMaskSize, 
            pstModeInfo->bRedFieldPosition );
    kPrintf( "Green Mask Size: %d, Field Position: %d\n", pstModeInfo->bGreenMaskSize, 
            pstModeInfo->bGreenFieldPosition );
    kPrintf( "Blue Mask Size: %d, Field Position: %d\n", pstModeInfo->bBlueMaskSize, 
            pstModeInfo->bBlueFieldPosition );
    kPrintf( "Physical Base Pointer: 0x%X\n", pstModeInfo->dwPhysicalBasePointer );
    
    kPrintf( "Linear Red Mask Size: %d, Field Position: %d\n", 
            pstModeInfo->bLinearRedMaskSize, pstModeInfo->bLinearRedFieldPosition );
    kPrintf( "Linear Green Mask Size: %d, Field Position: %d\n", 
            pstModeInfo->bLinearGreenMaskSize, pstModeInfo->bLinearGreenFieldPosition );
    kPrintf( "Linear Blue Mask Size: %d, Field Position: %d\n", 
            pstModeInfo->bLinearBlueMaskSize, pstModeInfo->bLinearBlueFieldPosition );
}

/**
 *  시스템 콜을 테스트하는 유저 레벨 태스크를 생성
 */
static void kTestSystemCall( const char* pcParameterBuffer )
{
    BYTE* pbUserMemory;
    
    // 동적 할당 영역에 4Kbyte 메모리를 할당 받아 유저 레벨 태스크를 생성할 준비를 함
    pbUserMemory = kAllocateMemory( 0x1000 );
    if( pbUserMemory == NULL )
    {
        return ;
    }
    
    // 유저 레벨 태스크로 사용할 kSystemCallTestTask() 함수의 코드를 할당 받은 메모리에 복사
    kMemCpy( pbUserMemory, kSystemCallTestTask, 0x1000 );
    
    // 유저 레벨 태스크로 생성
    kCreateTask( TASK_FLAGS_USERLEVEL | TASK_FLAGS_PROCESS,
            pbUserMemory, 0x1000, ( QWORD ) pbUserMemory, TASK_LOADBALANCINGID );
}
