/**
 *  file    Task.c
 *  date    2009/02/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   태스크를 처리하는 함수에 관련된 파일
 */

#include "Task.h"
#include "Descriptor.h"

// 스케줄러 관련 자료구조
static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

//==============================================================================
//  태스크 풀과 태스크 관련
//==============================================================================
/**
 *  태스크 풀 초기화
 */
void kInitializeTCBPool( void )
{
    int i;
    
    kMemSet( &( gs_stTCBPoolManager ), 0, sizeof( gs_stTCBPoolManager ) );
    
    // 태스크 풀의 어드레스를 지정하고 초기화
    gs_stTCBPoolManager.pstStartAddress = ( TCB* ) TASK_TCBPOOLADDRESS;
    kMemSet( TASK_TCBPOOLADDRESS, 0, sizeof( TCB ) * TASK_MAXCOUNT );

    // TCB에 ID 할당
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    }
    
    // TCB의 최대 개수와 할당된 횟수를 초기화
    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;
}

/**
 *  TCB를 할당 받음
 */
TCB* kAllocateTCB( void )
{
    TCB* pstEmptyTCB;
    int i;
    
    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount )
    {
        return NULL;
    }

    for( i = 0 ; i < gs_stTCBPoolManager.iMaxCount ; i++ )
    {
        // ID의 상위 32비트가 0이면 할당되지 않은 TCB
        if( ( gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 )
        {
            pstEmptyTCB = &( gs_stTCBPoolManager.pstStartAddress[ i ] );
            break;
        }
    }

    // 상위 32비트를 0이 아닌 값으로 설정해서 할당된 TCB로 설정
    pstEmptyTCB->stLink.qwID = ( ( QWORD ) gs_stTCBPoolManager.iAllocatedCount << 32 ) | i;
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    if( gs_stTCBPoolManager.iAllocatedCount == 0 )
    {
        gs_stTCBPoolManager.iAllocatedCount = 1;
    }
    
    return pstEmptyTCB;
}

/**
 *  TCB를 해제함
 */
void kFreeTCB( QWORD qwID )
{
    int i;
    
    // 태스크 ID의 하위 32비트가 인덱스 역할을 함
    i = qwID & 0xFFFFFFFF;
    
    // TCB를 초기화하고 ID 설정
    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    
    gs_stTCBPoolManager.iUseCount--;
}

/**
 *  태스크를 생성
 *      태스크 ID에 따라서 스택 풀에서 스택 자동 할당
 */
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress )
{
    TCB* pstTask;
    void* pvStackAddress;
    
    pstTask = kAllocateTCB();
    if( pstTask == NULL )
    {
        return NULL;
    }
    
    // 태스크 ID로 스택 어드레스 계산, 하위 32비트가 스택 풀의 오프셋 역할 수행
    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * 
            ( pstTask->stLink.qwID & 0xFFFFFFFF ) ) );
    
    // TCB를 설정한 후 준비 리스트에 삽입하여 스케줄링될 수 있도록 함
    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, 
            TASK_STACKSIZE );
    kAddTaskToReadyList( pstTask );
    
    return pstTask;
}

/**
 *  파라미터를 이용해서 TCB를 설정
 */
void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
                 void* pvStackAddress, QWORD qwStackSize )
{
    // 콘텍스트 초기화
    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );
    
    // 스택에 관련된 RSP, RBP 레지스터 설정
    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize;

    // 세그먼트 셀렉터 설정
    pstTCB->stContext.vqRegister[ TASK_CSOFFSET ] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[ TASK_DSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_ESOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_FSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_GSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_SSOFFSET ] = GDT_KERNELDATASEGMENT;
    
    // RIP 레지스터와 인터럽트 플래그 설정
    pstTCB->stContext.vqRegister[ TASK_RIPOFFSET ] = qwEntryPointAddress;

    // RFLAGS 레지스터의 IF 비트(비트 9)를 1로 설정하여 인터럽트 활성화
    pstTCB->stContext.vqRegister[ TASK_RFLAGSOFFSET ] |= 0x0200;
    
    // 스택과 플래그 저장
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}

//==============================================================================
//  스케줄러 관련
//==============================================================================
/**
 *  스케줄러를 초기화
 *      스케줄러를 초기화하는데 필요한 TCB 풀과 init 태스크도 같이 초기화
 */
void kInitializeScheduler( void )
{
    // 태스크 풀 초기화
    kInitializeTCBPool();

    // 준비 리스트 초기화
    kInitializeList( &( gs_stScheduler.stReadyList ) );

    // TCB를 할당 받아 실행 중인 태스크로 설정하여, 부팅을 수행한 태스크를 저장할 TCB를 준비
    gs_stScheduler.pstRunningTask = kAllocateTCB();
}

/**
 *  현재 수행 중인 태스크를 설정
 */
void kSetRunningTask( TCB* pstTask )
{
    gs_stScheduler.pstRunningTask = pstTask;
}

/**
 *  현재 수행 중인 태스크를 반환
 */
TCB* kGetRunningTask( void )
{
    return gs_stScheduler.pstRunningTask;
}

/**
 *  태스크 리스트에서 다음으로 실행할 태스크를 얻음
 */
TCB* kGetNextTaskToRun( void )
{
    if( kGetListCount( &( gs_stScheduler.stReadyList ) ) == 0 )
    {
        return NULL;
    }
    
    return ( TCB* ) kRemoveListFromHeader( &( gs_stScheduler.stReadyList ) );
}

/**
 *  태스크를 스케줄러의 준비 리스트에 삽입
 */
void kAddTaskToReadyList( TCB* pstTask )
{
    kAddListToTail( &( gs_stScheduler.stReadyList ), pstTask );
}

/**
 *  다른 태스크를 찾아서 전환
 *      인터럽트나 예외가 발생했을 때 호출하면 안됨
 */
void kSchedule( void )
{
    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousFlag;
    
    // 전환할 태스크가 있어야 함
    if( kGetListCount( &( gs_stScheduler.stReadyList ) ) == 0 )
    {
        return ;
    }
    
    // 전환하는 도중 인터럽트가 발생하여 태스크 전환이 또 일어나면 곤란하므로 전환하는 
    // 동안 인터럽트가 발생하지 못하도록 설정
    bPreviousFlag = kSetInterruptFlag( FALSE );
    // 실행할 다음 태스크를 얻음
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL )
    {
        kSetInterruptFlag( bPreviousFlag );
        return ;
    }
    
    pstRunningTask = gs_stScheduler.pstRunningTask; 
    kAddTaskToReadyList( pstRunningTask );
    
    // 다음 태스크를 현재 수행 중인 태스크로 설정한 후 콘텍스트 전환
    gs_stScheduler.pstRunningTask = pstNextTask;
    kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );

    // 프로세서 사용 시간을 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    kSetInterruptFlag( bPreviousFlag );
}

/**
 *  인터럽트가 발생했을 때, 다른 태스크를 찾아 전환
 *      반드시 인터럽트나 예외가 발생했을 때 호출해야 함
 */
BOOL kScheduleInInterrupt( void )
{
    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    
    // 전환할 태스크가 없으면 종료
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL )
    {
        return FALSE;
    }
    
    //==========================================================================
    //  태스크 전환 처리   
    //      인터럽트 핸들러에서 저장한 콘텍스트를 다른 콘텍스트로 덮어쓰는 방법으로 처리
    //==========================================================================
    pcContextAddress = ( char* ) IST_STARTADDRESS + IST_SIZE - sizeof( CONTEXT );
    
    // 현재 태스크를 얻어서 IST에 있는 콘텍스트를 복사하고, 현재 태스크를 준비 리스트로
    // 옮김
    pstRunningTask = gs_stScheduler.pstRunningTask;
    kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
    kAddTaskToReadyList( pstRunningTask );

    // 전환해서 실행할 태스크를 Running Task로 설정하고 콘텍스트를 IST에 복사해서
    // 자동으로 태스크 전환이 일어나도록 함
    gs_stScheduler.pstRunningTask = pstNextTask;
    kMemCpy( pcContextAddress, &( pstNextTask->stContext ), sizeof( CONTEXT ) );
    
    // 프로세서 사용 시간을 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    return TRUE;
}

/**
 *  프로세서를 사용할 수 있는 시간을 하나 줄임
 */
void kDecreaseProcessorTime( void )
{
    if( gs_stScheduler.iProcessorTime > 0 )
    {
        gs_stScheduler.iProcessorTime--;
    }
}

/**
 *  프로세서를 사용할 수 있는 시간이 다 되었는지 여부를 반환
 */
BOOL kIsProcessorTimeExpired( void )
{
    if( gs_stScheduler.iProcessorTime <= 0 )
    {
        return TRUE;
    }
    return FALSE;
}
