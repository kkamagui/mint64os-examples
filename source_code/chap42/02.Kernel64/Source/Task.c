/**
 *  file    Task.c
 *  date    2009/02/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   태스크를 처리하는 함수에 관련된 파일
 */

#include "Task.h"
#include "Descriptor.h"
#include "MultiProcessor.h"

// 스케줄러 관련 자료구조
static SCHEDULER gs_vstScheduler[ MAXPROCESSORCOUNT ];
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
    
    // 스핀락 초기화
    kInitializeSpinLock( &gs_stTCBPoolManager.stSpinLock );
}

/**
 *  TCB를 할당 받음
 */
static TCB* kAllocateTCB( void )
{
    TCB* pstEmptyTCB;
    int i;
    
    // 동기화 처리
    kLockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
    
    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount )
    {
        // 동기화 처리
        kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
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

    // 동기화 처리
    kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );

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

    // 동기화 처리
    kLockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
    
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    
    gs_stTCBPoolManager.iUseCount--;

    // 동기화 처리
    kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
}

/**
 *  태스크를 생성
 *      태스크 ID에 따라서 스택 풀에서 스택 자동 할당
 *      프로세스 및 스레드 모두 생성 가능
 *      bAffinity에 태스크를 수행하고 싶은 코어의 ID를 설정 가능
 */
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress, BYTE bAffinity )
{
    TCB* pstTask, * pstProcess;
    void* pvStackAddress;
    BYTE bCurrentAPICID;
    
    // 현재 코어의 로컬 APIC ID를 확인
    bCurrentAPICID = kGetAPICID();
    
    // 태스크 자료구조 할당
    pstTask = kAllocateTCB();
    if( pstTask == NULL )
    {
        return NULL;
    }

    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    
    // 현재 프로세스 또는 스레드가 속한 프로세스를 검색
    pstProcess = kGetProcessByThread( kGetRunningTask( bCurrentAPICID ) );
    // 만약 프로세스가 없다면 아무런 작업도 하지 않음
    if( pstProcess == NULL )
    {
        kFreeTCB( pstTask->stLink.qwID );
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
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
    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    
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
    
    // 현재 코어의 로컬 APIC ID를 태스크에 설정
    pstTask->bAPICID = bCurrentAPICID;
    
    // 프로세서 친화도(Affinity)를 설정
    pstTask->bAffinity = bAffinity;

    // 부하 분산을 고려하여 스케줄러에 태스크를 추가 
    kAddTaskToSchedulerWithLoadBalancing( pstTask );
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
    int j;
    BYTE bCurrentAPICID;
    TCB* pstTask;
    
    // 현재 코어의 로컬 APIC ID 확인
    bCurrentAPICID = kGetAPICID();

    // Bootstrap Processor만 태스크 풀과 스케줄러 자료구조를 모두 초기화
    if( bCurrentAPICID == 0 )
    {
        // 태스크 풀 초기화
        kInitializeTCBPool();
        
        // 준비 리스트와 우선 순위별 실행 횟수를 초기화하고 대기 리스트와 스핀락을 초기화
        for( j = 0 ; j < MAXPROCESSORCOUNT ; j++ )
        {
            // 준비 리스트 초기화
            for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
            {
                kInitializeList( &( gs_vstScheduler[ j ].vstReadyList[ i ] ) );
                gs_vstScheduler[ j ].viExecuteCount[ i ] = 0;
            }    
            // 대기 리스트 초기화
            kInitializeList( &( gs_vstScheduler[ j ].stWaitList ) );

            // 스핀락 초기화
            kInitializeSpinLock( &( gs_vstScheduler[ j ].stSpinLock ) );
        }
    }    
    
    // TCB를 할당 받아 부팅을 수행한 태스크를 커널 최초의 프로세스로 설정
    pstTask = kAllocateTCB();
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstTask;
    
    // BSP의 콘솔 쉘이나 AP의 유휴 태스크(Idle Task)는 모두 현재 코어에서만 실행하도록
    // 로컬 APIC ID와 프로세서 친화도를 현재 코어의 로컬 APIC ID로 설정
    pstTask->bAPICID = bCurrentAPICID;
    pstTask->bAffinity = bCurrentAPICID;
    
    // Bootstrap Processor는 콘솔 셸을 실행
    if( bCurrentAPICID == 0 )
    {
        pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    }
    // Application Processor는 특별히 긴급한 태스크가 없으므로 유휴(Idle) 태스크를 실행
    else
    {
        pstTask->qwFlags = TASK_FLAGS_LOWEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE;
    }
    
    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = ( void* ) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = ( void* ) 0x600000;
    pstTask->qwStackSize = 0x100000;
    
    // 프로세서 사용률을 계산하는데 사용하는 자료구조 초기화
    gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask = 0;
    gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 0;
    
    // FPU를 사용한 태스크 ID를 유효하지 않은 값으로 초기화
    gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID = TASK_INVALIDID;
}

/**
 *  현재 수행 중인 태스크를 설정
 */
void kSetRunningTask( BYTE bAPICID, TCB* pstTask )
{
    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    gs_vstScheduler[ bAPICID ].pstRunningTask = pstTask;

    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
}

/**
 *  현재 수행 중인 태스크를 반환
 */
TCB* kGetRunningTask( BYTE bAPICID )
{
    TCB* pstRunningTask;
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    
    pstRunningTask = gs_vstScheduler[ bAPICID ].pstRunningTask;
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return pstRunningTask;
}

/**
 *  태스크 리스트에서 다음으로 실행할 태스크를 얻음
 */
static TCB* kGetNextTaskToRun( BYTE bAPICID )
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
            iTaskCount = kGetListCount( &( gs_vstScheduler[ bAPICID ].
                    vstReadyList[ i ] ) );
            
            // 만약 실행한 횟수보다 리스트의 태스크 수가 더 많으면 현재 우선 순위의
            // 태스크를 실행함
            if( gs_vstScheduler[ bAPICID ].viExecuteCount[ i ] < iTaskCount )
            {
                pstTarget = ( TCB* ) kRemoveListFromHeader( 
                    &( gs_vstScheduler[ bAPICID ].vstReadyList[ i ] ) );
                gs_vstScheduler[ bAPICID ].viExecuteCount[ i ]++;
                break;            
            }
            // 만약 실행한 횟수가 더 많으면 실행 횟수를 초기화하고 다음 우선 순위로 양보함
            else
            {
                gs_vstScheduler[ bAPICID ].viExecuteCount[ i ] = 0;
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
static BOOL kAddTaskToReadyList( BYTE bAPICID, TCB* pstTask )
{
    BYTE bPriority;
    
    bPriority = GETPRIORITY( pstTask->qwFlags );
    if( bPriority == TASK_FLAGS_WAIT )
    {
        kAddListToTail( &( gs_vstScheduler[ bAPICID ].stWaitList ), pstTask );
        return TRUE;
    }
    else if( bPriority >= TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    kAddListToTail( &( gs_vstScheduler[ bAPICID ].vstReadyList[ bPriority ] ), 
            pstTask );
    return TRUE;
}

/**
 *  준비 큐에서 태스크를 제거
 */
static TCB* kRemoveTaskFromReadyList( BYTE bAPICID, QWORD qwTaskID )
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

    pstTarget = kRemoveList( &( gs_vstScheduler[ bAPICID ].vstReadyList[ bPriority ]), 
                     qwTaskID );
    return pstTarget;
}

/**
 *  태스크가 포함된 스케줄러의 ID를 반환하고, 해당 스케줄러의 스핀락을 잠금
 */
static BOOL kFindSchedulerOfTaskAndLock( QWORD qwTaskID, BYTE* pbAPICID )
{
    TCB* pstTarget;
    BYTE bAPICID;
    
    while( 1 )
    {
        // 태스크 ID로 태스크 자료구조를 찾아서 어느 스케줄러에서 실행 중인지 확인
        pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
        if( ( pstTarget == NULL ) || ( pstTarget->stLink.qwID != qwTaskID ) )
        {
            return FALSE;
        }
    
        // 현재 태스크가 실행되는 코어의 ID를 확인
        bAPICID = pstTarget->bAPICID;
        
        // 임계 영역 시작
        kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        // 스핀락을 획득한 이후 다시 확인하여 같은 코어에서 실행되는지 확인
        // 태스크가 수행되는 코어를 찾은 후 정확하게 스핀락을 걸기 위해 2중으로 검사
        pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
        if( pstTarget->bAPICID == bAPICID )
        {
            break;
        }
        
        // 태스크 자료구조에 저장된 로컬 APIC ID의 값이 스핀락을 획득하기 전과 후가 
        // 다르다면, 스핀락을 획득하는 동안 태스크가 다른 코어로 옮겨간 것임
        // 따라서 다시 스핀락을 해제하고 옮겨진 코어의 스핀락을 획득해야 함
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    }  
    
    *pbAPICID = bAPICID;
    return TRUE;
}

/**
 *  태스크의 우선 순위를 변경
 */
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority )
{
    TCB* pstTarget;
    BYTE bAPICID;
    
    if( bPriority > TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    // 태스크가 포함된 코어의 로컬 APIC ID를 찾은 후, 스핀락을 잠금
    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE )
    {
        return FALSE;
    }
    
    // 실행중인 태스크이면 우선 순위만 변경
    // PIT 컨트롤러의 인터럽트(IRQ 0)가 발생하여 태스크 전환이 수행될 때 변경된 
    // 우선 순위의 리스트로 이동
    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        SETPRIORITY( pstTarget->qwFlags, bPriority );
    }
    // 실행중인 태스크가 아니면 준비 리스트에서 찾아서 해당 우선 순위의 리스트로 이동
    else
    {
        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 우선 순위를 설정
        pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
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
            kAddTaskToReadyList( bAPICID, pstTarget );
        }
    }
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    
    return TRUE;    
}

/**
 *  다른 태스크를 찾아서 전환
 *      인터럽트나 예외가 발생했을 때 호출하면 안됨
 */
BOOL kSchedule( void )
{
    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousInterrupt;
    BYTE bCurrentAPICID;
    
    // 전환하는 도중 인터럽트가 발생하여 태스크 전환이 또 일어나면 곤란하므로 전환하는 
    // 동안 인터럽트가 발생하지 못하도록 설정
    bPreviousInterrupt = kSetInterruptFlag( FALSE );
    
    // 현재 로컬 APIC ID 확인
    bCurrentAPICID = kGetAPICID();
    
    // 전환할 태스크가 있어야 함
    if( kGetReadyTaskCount( bCurrentAPICID ) < 1 )
    {
        kSetInterruptFlag( bPreviousInterrupt );
        return FALSE;
    }
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    // 실행할 다음 태스크를 얻음
    pstNextTask = kGetNextTaskToRun( bCurrentAPICID );
    if( pstNextTask == NULL )
    {
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        kSetInterruptFlag( bPreviousInterrupt );
        return FALSE;
    }
    
    // 현재 수행중인 태스크의 정보를 수정한 뒤 콘텍스트 전환
    pstRunningTask = gs_vstScheduler[ bCurrentAPICID ].pstRunningTask; 
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환되었다면 사용한 프로세서 시간을 증가시킴
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask += 
            TASK_PROCESSORTIME - gs_vstScheduler[ bCurrentAPICID ].iProcessorTime;
    }

    // 다음에 수행할 태스크가 FPU를 쓴 태스크가 아니라면 TS 비트를 설정
    if( gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID != 
        pstNextTask->stLink.qwID )
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
        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), pstRunningTask );

        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

        // 태스크 전환
        kSwitchContext( NULL, &( pstNextTask->stContext ) );
    }
    else
    {
        kAddTaskToReadyList( bCurrentAPICID, pstRunningTask );
        
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

        // 태스크 전환
        kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );
    }

    // 프로세서 사용 시간을 업데이트
    gs_vstScheduler[ bCurrentAPICID ].iProcessorTime = TASK_PROCESSORTIME;
    
    // 인터럽트 플래그 복원
    kSetInterruptFlag( bPreviousInterrupt );
    return FALSE;
}

/**
 *  인터럽트가 발생했을 때, 다른 태스크를 찾아 전환
 *      반드시 인터럽트나 예외가 발생했을 때 호출해야 함
 */
BOOL kScheduleInInterrupt( void )
{
    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    BYTE bCurrentAPICID;
    QWORD qwISTStartAddress;
        
    // 현재 로컬 APIC ID 확인
    bCurrentAPICID = kGetAPICID();
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        
    // 전환할 태스크가 없으면 종료
    pstNextTask = kGetNextTaskToRun( bCurrentAPICID );
    if( pstNextTask == NULL )
    {
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        return FALSE;
    }
    
    //==========================================================================
    //  태스크 전환 처리   
    //      인터럽트 핸들러에서 저장한 콘텍스트를 다른 콘텍스트로 덮어쓰는 방법으로 처리
    //==========================================================================
    // IST의 끝부분부터 코어 0 -> 코어 15 순으로 64Kbyte씩 쓰고 있으므로, 로컬 APIC ID를
    // 이용해서 IST 어드레스를 계산
    qwISTStartAddress = IST_STARTADDRESS + IST_SIZE - 
                        ( IST_SIZE / MAXPROCESSORCOUNT * bCurrentAPICID );
    pcContextAddress = ( char* ) qwISTStartAddress - sizeof( CONTEXT );
    
    pstRunningTask = gs_vstScheduler[ bCurrentAPICID ].pstRunningTask;
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstNextTask;

    // 유휴 태스크에서 전환되었다면 사용한 Tick Count를 증가시킴
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }
    
    // 태스크 종료 플래그가 설정된 경우, 콘텍스트를 저장하지 않고 대기 리스트에만 삽입
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {    
        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), 
                        pstRunningTask );
    }
    // 태스크가 종료되지 않으면 IST에 있는 콘텍스트를 복사하고, 현재 태스크를 준비 리스트로
    // 옮김
    else
    {
        kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
    }
    
    // 다음에 수행할 태스크가 FPU를 쓴 태스크가 아니라면 TS bit 설정
    if( gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID != 
        pstNextTask->stLink.qwID )
    {
        kSetTS();
    }
    else
    {
        kClearTS();
    }
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    
    // 전환해서 실행할 태스크를 Running Task로 설정하고 콘텍스트를 IST에 복사해서
    // 자동으로 태스크 전환이 일어나도록 함
    kMemCpy( pcContextAddress, &( pstNextTask->stContext ), sizeof( CONTEXT ) );
    
    // 종료하는 태스크가 아니면 스케줄러에 태스크 추가
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) != TASK_FLAGS_ENDTASK )
    {
        // 스케줄러에 태스크를 추가, 부하 분산을 고려함
        kAddTaskToSchedulerWithLoadBalancing( pstRunningTask );
    }    
    
    // 프로세서 사용 시간을 업데이트
    gs_vstScheduler[ bCurrentAPICID ].iProcessorTime = TASK_PROCESSORTIME;
    
    return TRUE;
}

/**
 *  프로세서를 사용할 수 있는 시간을 하나 줄임
 */
void kDecreaseProcessorTime( BYTE bAPICID )
{
    gs_vstScheduler[ bAPICID ].iProcessorTime--;
}

/**
 *  프로세서를 사용할 수 있는 시간이 다 되었는지 여부를 반환
 */
BOOL kIsProcessorTimeExpired( BYTE bAPICID )
{
    if( gs_vstScheduler[ bAPICID ].iProcessorTime <= 0 )
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
    BYTE bAPICID;
    
    // 태스크가 포함된 코어의 로컬 APIC ID를 찾은 후, 스핀락을 잠금
    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE )
    {
        return FALSE;
    }
    
    // 현재 실행중인 태스크이면 EndTask 비트를 설정하고 태스크를 전환
    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        
        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
        
        // 현재 스케줄러에서 실행중인 태스크의 경우만 아래를 적용
        if( kGetAPICID() == bAPICID )
        {
            kSchedule();
            
            // 태스크가 전환 되었으므로 아래 코드는 절대 실행되지 않음
            while( 1 ) 
            {
                ;
            }
        }
        
        return TRUE;
    }
    
    // 실행 중인 태스크가 아니면 준비 큐에서 직접 찾아서 대기 리스트에 연결
    // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 태스크 종료 비트를
    // 설정
    pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
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
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
        return TRUE;
    }
    
    pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
    SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
    kAddListToTail( &( gs_vstScheduler[ bAPICID ].stWaitList ), pstTarget );
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    return TRUE;
}

/**
 *  태스크가 자신을 종료함
 */
void kExitTask( void )
{
    kEndTask( gs_vstScheduler[ kGetAPICID() ].pstRunningTask->stLink.qwID );
}

/**
 *  준비 큐에 있는 모든 태스크의 수를 반환
 */
int kGetReadyTaskCount( BYTE bAPICID )
{
    int iTotalCount = 0;
    int i;

    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    // 모든 준비 큐를 확인하여 태스크 개수를 구함
    for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
    {
        iTotalCount += kGetListCount( &( gs_vstScheduler[ bAPICID ].
                vstReadyList[ i ] ) );
    }
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    return iTotalCount ;
}

/**
 *  전체 태스크의 수를 반환
 */ 
int kGetTaskCount( BYTE bAPICID )
{
    int iTotalCount;
    
    // 준비 큐의 태스크 수를 구한 후, 대기 큐의 태스크 수와 현재 수행 중인 태스크 수를 더함
    iTotalCount = kGetReadyTaskCount( bAPICID );
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    
    iTotalCount += kGetListCount( &( gs_vstScheduler[ bAPICID ].stWaitList ) ) + 1;

    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
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
QWORD kGetProcessorLoad( BYTE bAPICID )
{
    return gs_vstScheduler[ bAPICID ].qwProcessorLoad;
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

/**
 *  각 스케줄러의 태스크 수를 이용하여 적절한 스케줄러에 태스크 추가
 *      부하 분산 기능을 사용하지 않는 경우 현재 코어에 삽입
 *      부하 분산을 사용하지 않는 경우, 태스크가 현재 수행되는 코어에서 계속 수행하므로
 *      pstTask에는 적어도 APIC ID가 설정되어 있어야 함
 */
void kAddTaskToSchedulerWithLoadBalancing( TCB* pstTask )
{
    BYTE bCurrentAPICID;
    BYTE bTargetAPICID;
    
    // 태스크가 동작하던 코어의 APIC를 확인
    bCurrentAPICID = pstTask->bAPICID;
    
    // 부하 분산 기능을 사용하고, 프로세서 친화도(Affinity)가 모든 코어(0xFF)로 
    // 설정되었으면 부하 분산 수행
    if( ( gs_vstScheduler[ bCurrentAPICID ].bUseLoadBalancing == TRUE ) &&
        ( pstTask->bAffinity == TASK_LOADBALANCINGID ) )
    {
        // 태스크를 추가할 스케줄러를 선택
        bTargetAPICID = kFindSchedulerOfMinumumTaskCount( pstTask );
    }
    // 태스크 부하 분산 기능과 관계 없이 프로세서 친화도 필드에 다른 코어의 APIC ID가 
    // 들어있으면 해당 스케줄러로 옮겨줌
    else if( ( pstTask->bAffinity != bCurrentAPICID ) &&
             ( pstTask->bAffinity != TASK_LOADBALANCINGID ) )
    {
        bTargetAPICID = pstTask->bAffinity;
    }
    // 부하 분산 기능을 사용하지 않는 경우는 현재 스케줄러에 다시 삽입
    else
    {
        bTargetAPICID = bCurrentAPICID;
    }
    
    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
    // 태스크를 추가할 스케줄러가 현재 스케줄러와 다르다면 태스크를 이동함.
    // FPU는 공유되지 않으므로 현재 태스크가 FPU를 마지막으로 썼다면 FPU 콘텍스트를 
    // 메모리에 저장해야 함
    if( ( bCurrentAPICID != bTargetAPICID ) &&
        ( pstTask->stLink.qwID == 
            gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID ) )
    {
        // FPU를 저장하기 전에 TS bit를 끄지 않으면, 예외 7(Device Not Available)이
        // 발생하므로 주의해야 함
        kClearTS();
        kSaveFPUContext( pstTask->vqwFPUContext );
        gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID = TASK_INVALIDID;
    }
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    // 임계 영역 시작
    kLockForSpinLock( &( gs_vstScheduler[ bTargetAPICID ].stSpinLock ) );    
 
    // 태스크를 수행할 코어의 APIC ID를 설정하고, 해당 스케줄러에 태스크 삽입
    pstTask->bAPICID = bTargetAPICID;
    kAddTaskToReadyList( bTargetAPICID, pstTask );
    
    // 임계 영역 끝
    kUnlockForSpinLock( &( gs_vstScheduler[ bTargetAPICID ].stSpinLock ) );
}

/**
 *  태스크를 추가할 스케줄러의 ID를 반환
 *      파라미터로 전달된 태스크 자료구조에는 적어도 플래그와 프로세서 친화도(Affinity) 필드가
 *      채워져있어야 함
 */
static BYTE kFindSchedulerOfMinumumTaskCount( const TCB* pstTask )
{
    BYTE bPriority;
    BYTE i;
    int iCurrentTaskCount;
    int iMinTaskCount;
    BYTE bMinCoreIndex;
    int iTempTaskCount;
    int iProcessorCount;
    
    // 코어의 개수를 확인
    iProcessorCount = kGetProcessorCount();
    
    // 코어가 하나라면 현재 코어에서 계속 수행
    if( iProcessorCount == 1 )
    {
        return pstTask->bAPICID;
    }
    
    // 우선 순위 추출
    bPriority = GETPRIORITY( pstTask->qwFlags );

    // 태스크가 포함된 스케줄러에서 태스크와 같은 우선 순위의 태스크 수를 확인
    iCurrentTaskCount = kGetListCount( &( gs_vstScheduler[ pstTask->bAPICID ].
            vstReadyList[ bPriority ] ) );
    
    // 나머지 코어에서 같은 현재 태스크와 같은 레벨을 검사
    // 자신과 태스크의 수가 적어도 2 이상 차이 나는 것 중에서 가장 태스크 수가 작은
    // 스케줄러의 ID를 반환
    iMinTaskCount = TASK_MAXCOUNT;
    bMinCoreIndex = pstTask->bAPICID;
    for( i = 0 ; i < iProcessorCount ; i++ )
    {
        if( i == pstTask->bAPICID )
        {
            continue;
        }
        
        // 모든 스케줄러를 돌면서 확인
        iTempTaskCount = kGetListCount( &( gs_vstScheduler[ i ].vstReadyList[ 
            bPriority ] ) );
        
        // 현재 코어와 태스크 수가 2개 이상 차이가 나고 이전까지 태스크 수가 가장 작았던
        // 코어보다 더 작다면 정보를 갱신함
        if( ( iTempTaskCount + 2 <= iCurrentTaskCount ) &&
            ( iTempTaskCount < iMinTaskCount ) )
        {
            bMinCoreIndex = i;
            iMinTaskCount = iTempTaskCount;
        }
    }
    
    return bMinCoreIndex;
}

/**
 *  파라미터로 전달된 코어에 태스크 부하 분산 기능 사용 여부를 설정
 */
BYTE kSetTaskLoadBalancing( BYTE bAPICID, BOOL bUseLoadBalancing )
{
    gs_vstScheduler[ bAPICID ].bUseLoadBalancing = bUseLoadBalancing;
}

/**
 *  프로세서 친화도를 변경
 */
BOOL kChangeProcessorAffinity( QWORD qwTaskID, BYTE bAffinity )
{
    TCB* pstTarget;
    BYTE bAPICID;
    
    // 태스크가 포함된 코어의 로컬 APIC ID를 찾은 후, 스핀락을 잠금
    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE )
    {
        return FALSE;
    }
    
    // 현재 실행중인 태스크이면 프로세서 친화도만 변경. 실제 태스크가 옮겨지는 시점은
    // 태스크 전환이 수행될 때임
    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        // 프로세서 친화도 변경
        pstTarget->bAffinity = bAffinity;

        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );
    }
    // 실행중인 태스크가 아니면 준비 리스트에서 찾아서 즉시 이동
    else
    {
        // 준비 리스트에서 태스크를 찾지 못하면 직접 태스크를 찾아서 친화도를 설정
        pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
        if( pstTarget == NULL )
        {
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                // 프로세서 친화도 변경
                pstTarget->bAffinity = bAffinity;
            }
        }
        else
        {
            // 프로세서 친화도 변경
            pstTarget->bAffinity = bAffinity;
        }

        // 임계 영역 끝
        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        // 프로세서 부하 분산을 고려해서 스케줄러에 등록
        kAddTaskToSchedulerWithLoadBalancing( pstTarget );
    }
    
    return TRUE;
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
    QWORD qwTaskID, qwChildThreadID;
    int i, iCount;
    void* pstThreadLink;
    BYTE bCurrentAPICID;
    BYTE bProcessAPICID;
    
    // 현재 코어의 로컬 APIC ID를 확인
    bCurrentAPICID = kGetAPICID();
    
    // 프로세서 사용량 계산을 위해 기준 정보를 저장
    qwLastSpendTickInIdleTask = 
        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();
    
    while( 1 )
    {
        // 현재 상태를 저장
        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickInIdleTask = 
            gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask;
        
        // 프로세서 사용량을 계산
        // 100 - ( 유휴 태스크가 사용한 프로세서 시간 ) * 100 / ( 시스템 전체에서 
        // 사용한 프로세서 시간 )
        if( qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0 )
        {
            gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 0;
        }
        else
        {
            gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 100 - 
                ( qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask ) * 
                100 /( qwCurrentMeasureTickCount - qwLastMeasureTickCount );
        }
        
        // 현재 상태를 이전 상태에 보관
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        // 프로세서의 부하에 따라 쉬게 함
        kHaltProcessorByLoad( bCurrentAPICID );
        
        // 대기 큐에 대기중인 태스크가 있으면 태스크를 종료함
        if( kGetListCount( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ) ) 
                > 0 )
        {
            while( 1 )
            {
                // 임계 영역 시작
                kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                pstTask = kRemoveListFromHeader( 
                    &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ) );
                // 임계 영역 끝
                kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                
                if( pstTask == NULL )
                {
                    break;
                }
                
                if( pstTask->qwFlags & TASK_FLAGS_PROCESS )
                {
                    // 프로세스를 종료할 때 자식 스레드가 존재하면 스레드를 모두 
                    // 종료하고, 다시 자식 스레드 리스트에 삽입
                    iCount = kGetListCount( &( pstTask->stChildThreadList ) );
                    for( i = 0 ; i < iCount ; i++ )
                    {
                        // 임계 영역 시작
                        kLockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        // 스레드 링크의 어드레스에서 꺼내 스레드를 종료시킴
                        pstThreadLink = ( TCB* ) kRemoveListFromHeader( 
                                &( pstTask->stChildThreadList ) );
                        if( pstThreadLink == NULL )
                        {
                            // 임계 영역 끝
                            kUnlockForSpinLock( 
                                &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
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
                        qwChildThreadID = pstChildThread->stLink.qwID;
                        // 임계 영역 끝
                        kUnlockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        
                        // 자식 스레드를 찾아서 종료
                        kEndTask( qwChildThreadID );
                    }
                    
                    // 아직 자식 스레드가 남아있다면 자식 스레드가 다 종료될 때까지
                    // 기다려야 하므로 다시 대기 리스트에 삽입
                    if( kGetListCount( &( pstTask->stChildThreadList ) ) > 0 )
                    {
                        // 임계 영역 시작
                        kLockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        kAddListToTail( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), 
                                        pstTask );
                        // 임계 영역 끝

                        kUnlockForSpinLock( 
                            &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
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
                        // 프로세스 ID로 프로세스가 속한 스케줄러의 ID를 찾고 스핀락 잠금
                        if( kFindSchedulerOfTaskAndLock( pstProcess->stLink.qwID, 
                                &bProcessAPICID ) == TRUE )
                        {
                            kRemoveList( &( pstProcess->stChildThreadList ), 
                                         pstTask->stLink.qwID );
                            kUnlockForSpinLock( &( gs_vstScheduler[ 
                                bProcessAPICID ].stSpinLock ) );
                        }
                    }
                }
                
                // 여기까지오면 태스크가 정상적으로 종료된 것이므로, 태스크 자료구조(TCB)를
                // 반환
                qwTaskID = pstTask->stLink.qwID;
                kFreeTCB( qwTaskID );                
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
void kHaltProcessorByLoad( BYTE bAPICID )
{
    if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 40 )
    {
        kHlt();
        kHlt();
        kHlt();
    }
    else if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 80 )
    {
        kHlt();
        kHlt();
    }
    else if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 95 )
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
QWORD kGetLastFPUUsedTaskID( BYTE bAPICID )
{
    return gs_vstScheduler[ bAPICID ].qwLastFPUUsedTaskID;
}

/**
 *  마지막으로 FPU를 사용한 태스크 ID를 설정
 */
void kSetLastFPUUsedTaskID( BYTE bAPICID, QWORD qwTaskID )
{
    gs_vstScheduler[ bAPICID ].qwLastFPUUsedTaskID = qwTaskID;
}


