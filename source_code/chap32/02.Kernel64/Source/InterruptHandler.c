/**
 *  file    InterruptHandler.c
 *  date    2009/01/24
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   인터럽트 및 예외 핸들러에 관련된 소스 파일
 */

#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"

// 인터럽트 핸들러 자료구조
static INTERRUPTMANAGER gs_stInterruptManager;

/**
 *  인터럽트 자료구조 초기화
 */
void kInitializeHandler( void )
{
    kMemSet( &gs_stInterruptManager, 0, sizeof( gs_stInterruptManager ) );
}

/**
 *  인터럽트 처리 모드를 설정
 */
void kSetSymmetricIOMode( BOOL bSymmetricIOMode )
{
    gs_stInterruptManager.bSymmetricIOMode = bSymmetricIOMode;
}

/**
 *  인터럽트 부하 분산 기능을 사용할지 여부를 설정
 */
void kSetInterruptLoadBalancing( BOOL bUseLoadBalancing )
{
    gs_stInterruptManager.bUseLoadBalancing = bUseLoadBalancing;
}

/**
 *  코어 별 인터럽트 처리 횟수를 증가
 */
void kIncreaseInterruptCount( int iIRQ )
{
    // 코어의 인터럽트 카운트를 증가
    gs_stInterruptManager.vvqwCoreInterruptCount[ kGetAPICID() ][ iIRQ ]++;
}

/**
 *  현재 인터럽트 모드에 맞추어 EOI를 전송
 */
void kSendEOI( int iIRQ )
{
    // 대칭 I/O 모드가 아니면 PIC 모드이므로, PIC 컨트롤러로 EOI를 전송해야 함
    if( gs_stInterruptManager.bSymmetricIOMode == FALSE )
    {
        kSendEOIToPIC( iIRQ );
    }
    // 대칭 I/O 모드이면 로컬 APIC로 EOI를 전송해야 함
    else
    {
        kSendEOIToLocalAPIC();
    }
}

/**
 *  인터럽트 핸들러 자료구조를 반환
 */
INTERRUPTMANAGER* kGetInterruptManager( void )
{
    return &gs_stInterruptManager;
}

/**
 *  인터럽트 부하 분산(Interrupt Load Balancing) 처리
 */
void kProcessLoadBalancing( int iIRQ )
{
    QWORD qwMinCount = 0xFFFFFFFFFFFFFFFF;
    int iMinCountCoreIndex;
    int iCoreCount;
    int i;
    BOOL bResetCount = FALSE;
    BYTE bAPICID;
    
    bAPICID = kGetAPICID();

    // 부하 분산 기능이 꺼져 있거나, 부하 분산을 처리할 시점이 아니면 할 필요가 없음
    if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] == 0 ) ||
        ( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] % 
            INTERRUPT_LOADBALANCINGDIVIDOR ) != 0 ) ||
        ( gs_stInterruptManager.bUseLoadBalancing == FALSE ) )
    {
        return ;
    }
    
    // 코어의 개수를 구해서 루프를 수행하며 인터럽트 처리 횟수가 가장 작은 코어를 
    // 선택
    iMinCountCoreIndex = 0;
    iCoreCount = kGetProcessorCount();
    for( i = 0 ; i < iCoreCount ; i++ )
    {
        if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] <
                qwMinCount ) )
        {
            qwMinCount = gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ];
            iMinCountCoreIndex = i;
        }
        // 전체 카운트가 거의 최대 값에 근접했다면 나중에 카운트를 모두 0으로 설정
        else if( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] >=
            0xFFFFFFFFFFFFFFFE )
        {
            bResetCount = TRUE;
        }
    }
    
    // I/O 리다이렉션 테이블을 변경하여 가장 인터럽트를 처리한 횟수가 작은 로컬 APIC로 전달
    kRoutingIRQToAPICID( iIRQ, iMinCountCoreIndex );
    
    // 처리한 코어의 카운트가 최댓값에 근접했다면 전체 카운트를 다시 0에서 시작하도록
    // 변경
    if( bResetCount == TRUE )
    {
        for( i = 0 ; i < iCoreCount ; i++ )
        {
            gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] = 0;
        }
    }
}

/**
 *  공통으로 사용하는 예외 핸들러
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "              Vector:           Core ID:            " );
    // 예외 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
    kPrintStringXY( 21, 2, vcBuffer );
    kSPrintf( vcBuffer, "0x%X", kGetAPICID() );
    kPrintStringXY( 40, 2, vcBuffer );
    kPrintStringXY( 0, 3, "====================================================" );

    while( 1 ) ;
}

/**
 *  공통으로 사용하는 인터럽트 핸들러 
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;
    int iIRQ;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // 인터럽트 벡터에서 IRQ 번호 추출
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    kSendEOI( iIRQ );
    
    // 인터럽트 발생 횟수를 업데이트
    kIncreaseInterruptCount( iIRQ );
    
    // 부하 분산(Load Balancing) 처리
    kProcessLoadBalancing( iIRQ );
}

/**
 *  키보드 인터럽트의 핸들러
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;
    
    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // 키보드 컨트롤러에서 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }
    
    // 인터럽트 벡터에서 IRQ 번호 추출
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    kSendEOI( iIRQ );
    
    // 인터럽트 발생 횟수를 업데이트
    kIncreaseInterruptCount( iIRQ );
    
    // 부하 분산(Load Balancing) 처리
    kProcessLoadBalancing( iIRQ );
}

/**
 *  타이머 인터럽트의 핸들러
 */
void kTimerHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;
    int iIRQ;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // 인터럽트 벡터에서 IRQ 번호 추출
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    kSendEOI( iIRQ );
    
    // 인터럽트 발생 횟수를 업데이트
    kIncreaseInterruptCount( iIRQ );

    // IRQ 0 인터럽트 처리는 Bootstrap Processor만 처리
    if( kGetAPICID() == 0 )
    {
        // 타이머 발생 횟수를 증가
        g_qwTickCount++;
    
        // 태스크가 사용한 프로세서의 시간을 줄임
        kDecreaseProcessorTime();
        // 프로세서가 사용할 수 있는 시간을 다 썼다면 태스크 전환 수행
        if( kIsProcessorTimeExpired() == TRUE )
        {
            kScheduleInInterrupt();
        }
    }
}

/**
 *  Device Not Available 예외의 핸들러
 */
void kDeviceNotAvailableHandler( int iVectorNumber )
{
    TCB* pstFPUTask, * pstCurrentTask;
    QWORD qwLastFPUTaskID;

    //=========================================================================
    // FPU 예외가 발생했음을 알리려고 메시지를 출력하는 부분
    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    // 예외 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );    
    //=========================================================================
    
    // CR0 컨트롤 레지스터의 TS 비트를 0으로 설정
    kClearTS();

    // 이전에 FPU를 사용한 태스크가 있는지 확인하여, 있다면 FPU의 상태를 태스크에 저장
    qwLastFPUTaskID = kGetLastFPUUsedTaskID();
    pstCurrentTask = kGetRunningTask();
    
    // 이전에 FPU를 사용한 것이 자신이면 아무것도 안 함
    if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID )
    {
        return ;
    }
    // FPU를 사용한 태스크가 있으면 FPU 상태를 저장
    else if( qwLastFPUTaskID != TASK_INVALIDID )
    {
        pstFPUTask = kGetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );
        if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) )
        {
            kSaveFPUContext( pstFPUTask->vqwFPUContext );
        }
    }
    
    // 현재 태스크가 FPU를 사용한 적이 있는 지 확인하여 FPU를 사용한 적이 없다면 
    // 초기화하고, 사용한적이 있다면 저장된 FPU 콘텍스트를 복원
    if( pstCurrentTask->bFPUUsed == FALSE )
    {
        kInitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    else
    {
        kLoadFPUContext( pstCurrentTask->vqwFPUContext );
    }
    
    // FPU를 사용한 태스크 ID를 현재 태스크로 변경
    kSetLastFPUUsedTaskID( pstCurrentTask->stLink.qwID );
}

/**
 *  하드 디스크에서 발생하는 인터럽트의 핸들러
 */
void kHDDHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;
    // 왼쪽 위에 있는 메시지와 겹치지 않도록 (10, 0)에 출력
    kPrintStringXY( 10, 0, vcBuffer );
    //=========================================================================

    // 첫 번째 PATA 포트의 인터럽트 발생 여부를 TRUE로 설정
    kSetHDDInterruptFlag( TRUE, TRUE );
    
    // 인터럽트 벡터에서 IRQ 번호 추출
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI 전송
    kSendEOI( iIRQ );
    
    // 인터럽트 발생 횟수를 업데이트
    kIncreaseInterruptCount( iIRQ );
    
    // 부하 분산(Load Balancing) 처리
    kProcessLoadBalancing( iIRQ );
}
