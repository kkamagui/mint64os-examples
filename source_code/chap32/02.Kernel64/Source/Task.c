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
static void kInitializeTCBPool( void )
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
static TCB* kAllocateTCB( void )
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
static void kFreeTCB( QWORD qwID )
{
    int i;
    
    // 태스크 ID의 하위 32비트가 인덱스 역할을 함
    i = GETTCBOFFSET( qwID );
    
    // TCB를 초기화하고 ID 설정
    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    
    gs_stTCBPoolManager.iUseCount--;
}

/**
 *  태스크를 생성
 *      태스크 ID에 따라서 스택 풀에서 스택 자동 할당
 *      프로세스 및 스레드 모두 생성 가능
 */
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress )
{
    TCB* pstTask, * pstProcess;
    void* pvStackAddress;
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    pstTask = kAllocateTCB();
    if( pstTask == NULL )
    {
        // 임계영역 끝
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        return NULL;
    }

    // 현재 프로세스 또는 스레드가 속한 프로세스를 검색
    pstProcess = kGetProcessByThread( kGetRunningTask() );
    // 만약 프로세스가 없다면 아무런 작업도 하지 않음
    if( pstProcess == NULL )
    {
        kFreeTCB( pstTask->stLink.qwID );
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        return NULL;
    }

    // 스레드를 생성하는 경우라면 내가 속한 프로세스의 자식 스레드 리스트에 연결함
    if( qwFlags & TASK_FLAGS_THREAD )
    {
        // 현재 스레드의 프로세스를 찾아서 생성할 스레드에 프로세스 정보를 상속
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
        pstTask->qwMemorySize = pstProcess->qwMemorySize;
        
        // 부모 프로세스의 자식 스레드 리스트에 추가
        kAddListToTail( &( pstProcess->stChildThreadList ), &( pstTask->stThreadLink ) );
    }
    // 프로세스는 파라미터로 넘어온 값을 그대로 설정
    else
    {
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pvMemoryAddress;
        pstTask->qwMemorySize = qwMemorySize;
    }
    
    // 스레드의 ID를 태스크 ID와 동일하게 설정
    pstTask->stThreadLink.qwID = pstTask->stLink.qwID;    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    // 태스크 ID로 스택 어드레스 계산, 하위 32비트가 스택 풀의 오프셋 역할 수행
    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * 
            GETTCBOFFSET( pstTask->stLink.qwID ) ) );
    
    // TCB를 설정한 후 준비 리스트에 삽입하여 스케줄링될 수 있도록 함
    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, 
            TASK_STACKSIZE );

    // 자식 스레드 리스트를 초기화
    kInitializeList( &( pstTask->stChildThreadList ) );
    
    // FPU 사용 여부를 사용하지 않은 것으로 초기화
    pstTask->bFPUUsed = FALSE;

    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    // 태스크를 준비 리스트에 삽입
    kAddTaskToReadyList( pstTask );
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    return pstTask;
}

/**
 *  파라미터를 이용해서 TCB를 설정
 */
static void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
                 void* pvStackAddress, QWORD qwStackSize )
{
    // 콘텍스트 초기화
    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );
    
    // 스택에 관련된 RSP, RBP 레지스터 설정
    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize - 8;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize - 8;
    
    // Return Address 영역에 kExitTask() 함수의 어드레스를 삽입하여 태스크의 엔트리
    // 포인트 함수를 빠져나감과 동시에 kExitTask() 함수로 이동하도록 함
    *( QWORD * ) ( ( QWORD ) pvStackAddress + qwStackSize - 8 ) = ( QWORD ) kExitTask;

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
    int i;
    TCB* pstTask;
    
    // 태스크 풀 초기화
    kInitializeTCBPool();

    // 준비 리스트와 우선 순위별 실행 횟수를 초기화하고 대기 리스트도 초기화
    for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
    {
        kInitializeList( &( gs_stScheduler.vstReadyList[ i ] ) );
        gs_stScheduler.viExecuteCount[ i ] = 0;
    }    
    kInitializeList( &( gs_stScheduler.stWaitList ) );
    
    // TCB를 할당 받아 부팅을 수행한 태스크를 커널 최초의 프로세스로 설정
    pstTask = kAllocateTCB();
    gs_stScheduler.pstRunningTask = pstTask;
    pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = ( void* ) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = ( void* ) 0x600000;
    pstTask->qwStackSize = 0x100000;
    
    // 프로세서 사용률을 계산하는데 사용하는 자료구조 초기화
    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;
    
    // FPU를 사용한 태스크 ID를 유효하지 않은 값으로 초기화
    gs_stScheduler.qwLastFPUUsedTaskID = TASK_INVALIDID;

    // 스핀락 초기화
    kInitializeSpinLock( &( gs_stScheduler.stSpinLock ) );
}

/**
 *  현재 수행 중인 태스크를 설정
 */
void kSetRunningTask( TCB* pstTask )
{
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    gs_stScheduler.pstRunningTask = pstTask;

    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
}

/**
 *  현재 수행 중인 태스크를 반환
 */
TCB* kGetRunningTask( void )
{
    TCB* pstRunningTask;
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    pstRunningTask = gs_stScheduler.pstRunningTask;
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    return pstRunningTask;
}

/**
 *  태스크 리스트에서 다음으로 실행할 태스크를 얻음
 */
static TCB* kGetNextTaskToRun( void )
{
    TCB* pstTarget = NULL;
    int iTaskCount, i, j;
    
    // 큐에 태스크가 있으나 모든 큐의 태스크가 1회씩 실행된 경우, 모든 큐가 프로세서를
    // 양보하여 태스크를 선택하지 못할 수 있으니 NULL일 경우 한번 더 수행
    for( j = 0 ; j < 2 ; j++ )
    {
        // 높은 우선 순위에서 낮은 우선 순위까지 리스트를 확인하여 스케줄링할 태스크를 선택
        for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
        {
            iTaskCount = kGetListCount( &( gs_stScheduler.vstReadyList[ i ] ) );
            
            // 만약 실행한 횟수보다 리스트의 태스크 수가 더 많으면 현재 우선 순위의
            // 태스크를 실행함
            if( gs_stScheduler.viExecuteCount[ i ] < iTaskCount )
            {
                pstTarget = ( TCB* ) kRemoveListFromHeader( 
                                        &( gs_stScheduler.vstReadyList[ i ] ) );
                gs_stScheduler.viExecuteCount[ i ]++;
                break;            
            }
            // 만약 실행한 횟수가 더 많으면 실행 횟수를 초기화하고 다음 우선 순위로 양보함
            else
            {
                gs_stScheduler.viExecuteCount[ i ] = 0;
            }
        }
        
        // 만약 수행할 태스크를 찾았으면 종료
        if( pstTarget != NULL )
        {
            break;
        }
    }    
    return pstTarget;
}

/**
 *  태스크를 스케줄러의 준비 리스트에 삽입
 */
static BOOL kAddTaskToReadyList( TCB* pstTask )
{
    BYTE bPriority;
    
    bPriority = GETPRIORITY( pstTask->qwFlags );
    if( bPriority == TASK_FLAGS_WAIT )
    {
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTask );
        return TRUE;
    }
    else if( bPriority >= TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    kAddListToTail( &( gs_stScheduler.vstReadyList[ bPriority ] ), pstTask );
    return TRUE;
}

/**
 *  준비 큐에서 태스크를 제거
 */
static TCB* kRemoveTaskFromReadyList( QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    
    // 태스크 ID가 유효하지 않으면 실패
    if( GETTCBOFFSET( qwTaskID ) >= TASK_MAXCOUNT )
    {
        return NULL;
    }
    
    // TCB 풀에서 해당 태스크의 TCB를 찾아 실제로 ID가 일치하는가 확인
    pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
    if( pstTarget->stLink.qwID != qwTaskID )
    {
        return NULL;
    }
    
    // 태스크가 존재하는 준비 리스트에서 태스크 제거
    bPriority = GETPRIORITY( pstTarget->qwFlags );
    if( bPriority >= TASK_MAXREADYLISTCOUNT )
    {
        return NULL;
    }    

    pstTarget = kRemoveList( &( gs_stScheduler.vstReadyList[ bPriority ]), 
                     qwTaskID );
    return pstTarget;
}

/**
 *  태스크의 우선 순위를 변경함
 */
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority )
{
    TCB* pstTarget;
    
    if( bPriority > TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    // 현재 실행중인 태스크이면 우선 순위만 변경
    // PIT 컨트롤러의 인터럽트(IRQ 0)가 발생하여 태스크 전환이 수행될 때 변경된 
    // 우선 순위의 리스트로 이동
    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        SETPRIORITY( pstTarget->qwFlags, bPriority );
    }
    // 실행중인 태스크가 아니면 준비 리스트에서 찾아서 해당 우선 순위의 리스트로 이동
    else
    {
        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 우선 순위를 설정
        pstTarget = kRemoveTaskFromReadyList( qwTaskID );
        if( pstTarget == NULL )
        {
            // 태스크 ID로 직접 찾아서 설정
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                // 우선 순위를 설정
                SETPRIORITY( pstTarget->qwFlags, bPriority );
            }
        }
        else
        {
            // 우선 순위를 설정하고 준비 리스트에 다시 삽입
            SETPRIORITY( pstTarget->qwFlags, bPriority );
            kAddTaskToReadyList( pstTarget );
        }
    }
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    return TRUE;    
}

/**
 *  다른 태스크를 찾아서 전환
 *      인터럽트나 예외가 발생했을 때 호출하면 안됨
 */
void kSchedule( void )
{
    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousInterrupt;
    
    // 전환할 태스크가 있어야 함
    if( kGetReadyTaskCount() < 1 )
    {
        return ;
    }
    
    // 전환하는 도중 인터럽트가 발생하여 태스크 전환이 또 일어나면 곤란하므로 전환하는 
    // 동안 인터럽트가 발생하지 못하도록 설정
    bPreviousInterrupt = kSetInterruptFlag( FALSE );
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    // 실행할 다음 태스크를 얻음
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL )
    {
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        kSetInterruptFlag( bPreviousInterrupt );
        return ;
    }
    
    // 현재 수행중인 태스크의 정보를 수정한 뒤 콘텍스트 전환
    pstRunningTask = gs_stScheduler.pstRunningTask; 
    gs_stScheduler.pstRunningTask = pstNextTask;
    
    // 유휴 태스크에서 전환되었다면 사용한 프로세서 시간을 증가시킴
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += 
            TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;
    }

    // 다음에 수행할 태스크가 FPU를 쓴 태스크가 아니라면 TS 비트 설정
    if( gs_stScheduler.qwLastFPUUsedTaskID != pstNextTask->stLink.qwID )
    {
        kSetTS();
    }   
    else
    {
        kClearTS();
    }

    // 태스크 종료 플래그가 설정된 경우 콘텍스트를 저장할 필요가 없으므로, 대기 리스트에
    // 삽입하고 콘텍스트 전환
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstRunningTask );
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        
        // 태스크 전환
        kSwitchContext( NULL, &( pstNextTask->stContext ) );
    }
    else
    {
        kAddTaskToReadyList( pstRunningTask );
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        
        // 태스크 전환
        kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );
    }

    // 프로세서 사용 시간을 업데이트
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    // 인터럽트 플래그 복원
    kSetInterruptFlag( bPreviousInterrupt );
}

/**
 *  인터럽트가 발생했을 때, 다른 태스크를 찾아 전환
 *      반드시 인터럽트나 예외가 발생했을 때 호출해야 함
 */
BOOL kScheduleInInterrupt( void )
{
    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    // 전환할 태스크가 없으면 종료
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL )
    {
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        return FALSE;
    }
    
    //==========================================================================
    //  태스크 전환 처리   
    //      인터럽트 핸들러에서 저장한 콘텍스트를 다른 콘텍스트로 덮어쓰는 방법으로 처리
    //==========================================================================
    pcContextAddress = ( char* ) IST_STARTADDRESS + IST_SIZE - sizeof( CONTEXT );
    
    // 현재 수행중인 태스크의 정보를 수정한 뒤 콘텍스트 전환
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환되었다면 사용한 Tick Count를 증가시킴
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }    
    
    // 태스크 종료 플래그가 설정된 경우, 콘텍스트를 저장하지 않고 대기 리스트에만 삽입
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {    
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstRunningTask );
    }
    // 태스크가 종료되지 않으면 IST에 있는 콘텍스트를 복사하고, 현재 태스크를 준비 리스트로
    // 옮김
    else
    {
        kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
        kAddTaskToReadyList( pstRunningTask );
    }
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    // 다음에 수행할 태스크가 FPU를 쓴 태스크가 아니라면 TS 비트 설정
    if( gs_stScheduler.qwLastFPUUsedTaskID != pstNextTask->stLink.qwID )
    {
        kSetTS();
    }   
    else
    {
        kClearTS();
    }

    // 전환해서 실행할 태스크를 Running Task로 설정하고 콘텍스트를 IST에 복사해서
    // 자동으로 태스크 전환이 일어나도록 함
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

/**
 *  태스크를 종료
 */
BOOL kEndTask( QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    // 현재 실행중인 태스크이면 EndTask 비트를 설정하고 태스크를 전환
    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        
        kSchedule();

        // 태스크가 전환 되었으므로 아래 코드는 절대 실행되지 않음
        while( 1 ) ;
    }
    // 실행 중인 태스크가 아니면 준비 큐에서 직접 찾아서 대기 리스트에 연결
    else
    {
        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 태스크 종료 비트를
        // 설정
        pstTarget = kRemoveTaskFromReadyList( qwTaskID );
        if( pstTarget == NULL )
        {
            // 태스크 ID로 직접 찾아서 설정
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
            }
            // 임계 영역 끝
            kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
            return TRUE;
        }
        
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTarget );
    }
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    return TRUE;
}

/**
 *  태스크가 자신을 종료함
 */
void kExitTask( void )
{
    kEndTask( gs_stScheduler.pstRunningTask->stLink.qwID );
}

/**
 *  준비 큐에 있는 모든 태스크의 수를 반환
 */
int kGetReadyTaskCount( void )
{
    int iTotalCount = 0;
    int i;

    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    // 모든 준비 큐를 확인하여 태스크 개수를 구함
    for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
    {
        iTotalCount += kGetListCount( &( gs_stScheduler.vstReadyList[ i ] ) );
    }
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    return iTotalCount ;
}

/**
 *  전체 태스크의 수를 반환
 */ 
int kGetTaskCount( void )
{
    int iTotalCount;
    
    // 준비 큐의 태스크 수를 구한 후, 대기 큐의 태스크 수와 현재 수행 중인 태스크 수를 더함
    iTotalCount = kGetReadyTaskCount();
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    
    iTotalCount += kGetListCount( &( gs_stScheduler.stWaitList ) ) + 1;

    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
    return iTotalCount;
}

/**
 *  TCB 풀에서 해당 오프셋의 TCB를 반환
 */
TCB* kGetTCBInTCBPool( int iOffset )
{
    if( ( iOffset < -1 ) && ( iOffset > TASK_MAXCOUNT ) )
    {
        return NULL;
    }
    
    return &( gs_stTCBPoolManager.pstStartAddress[ iOffset ] );
}

/**
 *  태스크가 존재하는지 여부를 반환
 */
BOOL kIsTaskExist( QWORD qwID )
{
    TCB* pstTCB;
    
    // ID로 TCB를 반환
    pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
    // TCB가 없거나 ID가 일치하지 않으면 존재하지 않는 것임
    if( ( pstTCB == NULL ) || ( pstTCB->stLink.qwID != qwID ) )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  프로세서의 사용률을 반환
 */
QWORD kGetProcessorLoad( void )
{
    return gs_stScheduler.qwProcessorLoad;
}

/**
 *  스레드가 소속된 프로세스를 반환
 */
static TCB* kGetProcessByThread( TCB* pstThread )
{
    TCB* pstProcess;
    
    // 만약 내가 프로세스이면 자신을 반환
    if( pstThread->qwFlags & TASK_FLAGS_PROCESS )
    {
        return pstThread;
    }
    
    // 내가 프로세스가 아니라면, 부모 프로세스로 설정된 태스크 ID를 통해 
    // TCB 풀에서 태스크 자료구조 추출
    pstProcess = kGetTCBInTCBPool( GETTCBOFFSET( pstThread->qwParentProcessID ) );

    // 만약 프로세스가 없거나, 태스크 ID가 일치하지 않는다면 NULL을 반환
    if( ( pstProcess == NULL ) || ( pstProcess->stLink.qwID != pstThread->qwParentProcessID ) )
    {
        return NULL;
    }
    
    return pstProcess;
}

//==============================================================================
//  유휴 태스크 관련
//==============================================================================
/**
 *  유휴 태스크
 *      대기 큐에 삭제 대기중인 태스크를 정리
 */
void kIdleTask( void )
{
    TCB* pstTask, * pstChildThread, * pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    int i, iCount;
    QWORD qwTaskID;
    void* pstThreadLink;
    
    // 프로세서 사용량 계산을 위해 기준 정보를 저장
    qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();
    
    while( 1 )
    {
        // 현재 상태를 저장
        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
        
        // 프로세서 사용량을 계산
        // 100 - ( 유휴 태스크가 사용한 프로세서 시간 ) * 100 / ( 시스템 전체에서 
        // 사용한 프로세서 시간 )
        if( qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0 )
        {
            gs_stScheduler.qwProcessorLoad = 0;
        }
        else
        {
            gs_stScheduler.qwProcessorLoad = 100 - 
                ( qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask ) * 
                100 /( qwCurrentMeasureTickCount - qwLastMeasureTickCount );
        }
        
        // 현재 상태를 이전 상태에 보관
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        // 프로세서의 부하에 따라 쉬게 함
        kHaltProcessorByLoad();
        
        // 대기 큐에 대기중인 태스크가 있으면 태스크를 종료함
        if( kGetListCount( &( gs_stScheduler.stWaitList ) ) >= 0 )
        {
            while( 1 )
            {
                // 임계 영역 시작
                kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
                pstTask = kRemoveListFromHeader( &( gs_stScheduler.stWaitList ) );
                if( pstTask == NULL )
                {
                    // 임계 영역 끝
                    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
                    break;
                }
                
                if( pstTask->qwFlags & TASK_FLAGS_PROCESS )
                {
                    // 프로세스를 종료할 때 자식 스레드가 존재하면 스레드를 모두 
                    // 종료하고, 다시 자식 스레드 리스트에 삽입
                    iCount = kGetListCount( &( pstTask->stChildThreadList ) );
                    for( i = 0 ; i < iCount ; i++ )
                    {
                        // 스레드 링크의 어드레스에서 꺼내 스레드를 종료시킴
                        pstThreadLink = ( TCB* ) kRemoveListFromHeader( 
                                &( pstTask->stChildThreadList ) );
                        if( pstThreadLink == NULL )
                        {
                            break;
                        }
                        
                        // 자식 스레드 리스트에 연결된 정보는 태스크 자료구조에 있는 
                        // stThreadLink의 시작 어드레스이므로, 태스크 자료구조의 시작
                        // 어드레스를 구하려면 별도의 계산이 필요함
                        pstChildThread = GETTCBFROMTHREADLINK( pstThreadLink );

                        // 다시 자식 스레드 리스트에 삽입하여 해당 스레드가 종료될 때
                        // 자식 스레드가 프로세스를 찾아 스스로 리스트에서 제거하도록 함
                        kAddListToTail( &( pstTask->stChildThreadList ),
                                &( pstChildThread->stThreadLink ) );

                        // 자식 스레드를 찾아서 종료
                        kEndTask( pstChildThread->stLink.qwID );
                    }
                    
                    // 아직 자식 스레드가 남아있다면 자식 스레드가 다 종료될 때까지
                    // 기다려야 하므로 다시 대기 리스트에 삽입
                    if( kGetListCount( &( pstTask->stChildThreadList ) ) > 0 )
                    {
                        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTask );

                        // 임계 영역 끝
                        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
                        continue;
                    }
                    // 프로세스를 종료해야 하므로 할당 받은 메모리 영역을 삭제
                    else
                    {
                        // TODO: 추후에 코드 삽입
                    }
                }                
                else if( pstTask->qwFlags & TASK_FLAGS_THREAD )
                {
                    // 스레드라면 프로세스의 자식 스레드 리스트에서 제거
                    pstProcess = kGetProcessByThread( pstTask );
                    if( pstProcess != NULL )
                    {
                        kRemoveList( &( pstProcess->stChildThreadList ), pstTask->stLink.qwID );
                    }
                }
                
                qwTaskID = pstTask->stLink.qwID;
                kFreeTCB( qwTaskID );
                // 임계 영역 끝
                kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
                
                kPrintf( "IDLE: Task ID[0x%q] is completely ended.\n", 
                        qwTaskID );
            }
        }
        
        kSchedule();
    }
}

/**
 *  측정된 프로세서 부하에 따라 프로세서를 쉬게 함
 */
void kHaltProcessorByLoad( void )
{
    if( gs_stScheduler.qwProcessorLoad < 40 )
    {
        kHlt();
        kHlt();
        kHlt();
    }
    else if( gs_stScheduler.qwProcessorLoad < 80 )
    {
        kHlt();
        kHlt();
    }
    else if( gs_stScheduler.qwProcessorLoad < 95 )
    {
        kHlt();
    }
}


//==============================================================================
//  FPU 관련
//==============================================================================
/**
 *  마지막으로 FPU를 사용한 태스크 ID를 반환
 */
QWORD kGetLastFPUUsedTaskID( void )
{
    return gs_stScheduler.qwLastFPUUsedTaskID;
}

/**
 *  마지막으로 FPU를 사용한 태스크 ID를 설정
 */
void kSetLastFPUUsedTaskID( QWORD qwTaskID )
{
    gs_stScheduler.qwLastFPUUsedTaskID = qwTaskID;
}

