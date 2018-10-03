/**
 *  file    SystemCallLibrary.c
 *  date    2009/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   MINT64 OS에서 지원하는 시스템 콜에 관련된 소스 파일
 */

#include "SystemCallLibrary.h"

//==============================================================================
//  콘솔 I/O 관련
//==============================================================================
/**
 *  콘솔에 문자열을 출력
 *      printf() 함수 내부에서 사용 
 *      \n, \t와 같은 문자가 포함된 문자열을 출력한 후, 화면상의 다음 출력할 위치를 
 *      반환
 */
int ConsolePrintString( const char* pcBuffer )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pcBuffer;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_CONSOLEPRINTSTRING, &stParameter );
}

/**
 *  커서의 위치를 설정
 *      문자를 출력할 위치도 같이 설정
 */
BOOL SetCursor( int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_SETCURSOR, &stParameter );
}

/**
 *  현재 커서의 위치를 반환
 */
BOOL GetCursor( int *piX, int *piY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) piX;
    PARAM( 1 ) = ( QWORD ) piY;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETCURSOR, &stParameter );
}

/**
 *  전체 화면을 삭제
 */
BOOL ClearScreen( void )
{
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_CLEARSCREEN, NULL );
}

/**
 *  getch() 함수의 구현
 */
BYTE getch( void )
{
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETCH, NULL );
}

//==============================================================================
// 동적 메모리 관련
//==============================================================================
/**
 *  메모리를 할당
 */
void* malloc( QWORD qwSize )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwSize;

    // 시스템 콜 호출
    return ( void* ) ExecuteSystemCall( SYSCALL_MALLOC, &stParameter );
}

/**
 *  할당 받은 메모리를 해제
 */
BOOL free( void* pvAddress )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pvAddress;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_FREE, &stParameter );    
}

//==============================================================================
// 파일과 디렉터리 I/O 관련
//==============================================================================
/**
 *  파일을 열거나 생성 
 */
FILE* fopen( const char* pcFileName, const char* pcMode )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pcFileName;
    PARAM( 1 ) = ( QWORD ) pcMode;

    // 시스템 콜 호출
    return ( FILE* ) ExecuteSystemCall( SYSCALL_FOPEN, &stParameter );      
}

/**
 *  파일을 읽어 버퍼로 복사
 */
DWORD fread( void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pvBuffer;
    PARAM( 1 ) = ( QWORD ) dwSize;
    PARAM( 2 ) = ( QWORD ) dwCount;
    PARAM( 3 ) = ( QWORD ) pstFile;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_FREAD, &stParameter );      
}

/**
 *  버퍼의 데이터를 파일에 씀
 */
DWORD fwrite( const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pvBuffer;
    PARAM( 1 ) = ( QWORD ) dwSize;
    PARAM( 2 ) = ( QWORD ) dwCount;
    PARAM( 3 ) = ( QWORD ) pstFile;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_FWRITE, &stParameter );    
}

/**
 *  파일 포인터의 위치를 이동
 */
int fseek( FILE* pstFile, int iOffset, int iOrigin )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstFile;
    PARAM( 1 ) = ( QWORD ) iOffset;
    PARAM( 2 ) = ( QWORD ) iOrigin;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_FSEEK, &stParameter );     
}

/**
 *  파일을 닫음
 */
int fclose( FILE* pstFile )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstFile;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_FCLOSE, &stParameter );      
}

/**
 *  파일을 삭제
 */
int remove( const char* pcFileName )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pcFileName;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_REMOVE, &stParameter );          
}

/**
 *  디렉터리를 엶
 */
DIR* opendir( const char* pcDirectoryName )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pcDirectoryName;

    // 시스템 콜 호출
    return ( DIR* ) ExecuteSystemCall( SYSCALL_OPENDIR, &stParameter );         
}

/**
 *  디렉터리 엔트리를 반환하고 다음으로 이동
 */
struct dirent* readdir( DIR* pstDirectory )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstDirectory;

    // 시스템 콜 호출
    return ( struct dirent* ) ExecuteSystemCall( SYSCALL_READDIR, 
                                                               &stParameter );       
}

/**
 *  디렉터리 포인터를 디렉터리의 처음으로 이동
 */
BOOL rewinddir( DIR* pstDirectory )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstDirectory;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_REWINDDIR, &stParameter );          
}

/**
 *  열린 디렉터리를 닫음
 */
int closedir( DIR* pstDirectory )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstDirectory;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_CLOSEDIR, &stParameter );       
}

/**
 *  핸들 풀을 검사하여 파일이 열려있는지를 확인
 */
BOOL IsFileOpened( const struct dirent* pstEntry )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstEntry;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISFILEOPENED, &stParameter );    
}

//==============================================================================
// 하드 디스크 I/O 관련
//==============================================================================
/**
 *  하드 디스크의 섹터를 읽음
 *      최대 256개의 섹터를 읽을 수 있음
 *      실제로 읽은 섹터 수를 반환
 */
int ReadHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) bPrimary;
    PARAM( 1 ) = ( QWORD ) bMaster;
    PARAM( 2 ) = ( QWORD ) dwLBA;
    PARAM( 3 ) = ( QWORD ) iSectorCount;
    PARAM( 4 ) = ( QWORD ) pcBuffer;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_READHDDSECTOR, &stParameter );     
}

/**
 *  하드 디스크에 섹터를 씀
 *      최대 256개의 섹터를 쓸 수 있음
 *      실제로 쓴 섹터 수를 반환
 */
int WriteHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) bPrimary;
    PARAM( 1 ) = ( QWORD ) bMaster;
    PARAM( 2 ) = ( QWORD ) dwLBA;
    PARAM( 3 ) = ( QWORD ) iSectorCount;
    PARAM( 4 ) = ( QWORD ) pcBuffer;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_WRITEHDDSECTOR, &stParameter );      
}

//==============================================================================
// 태스크와 스케줄러 관련
//==============================================================================
/**
 *  태스크를 생성
 */
QWORD CreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress, BYTE bAffinity )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) qwFlags;
    PARAM( 1 ) = ( QWORD ) pvMemoryAddress;
    PARAM( 2 ) = ( QWORD ) qwMemorySize;
    PARAM( 3 ) = ( QWORD ) qwEntryPointAddress;
    PARAM( 4 ) = ( QWORD ) bAffinity;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_CREATETASK, &stParameter );     
}

/**
 *  다른 태스크를 찾아서 전환
 */
BOOL Schedule( void )
{
    // 시스템 콜 호출
    return ( BOOL) ExecuteSystemCall( SYSCALL_SCHEDULE, NULL );
}

/**
 *  태스크의 우선 순위를 변경
 */
BOOL ChangePriority( QWORD qwID, BYTE bPriority, BOOL bExecutedInInterrupt )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwID;
    PARAM( 1 ) = ( QWORD ) bPriority;
    PARAM( 2 ) = ( QWORD ) bExecutedInInterrupt;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_CHANGEPRIORITY, &stParameter );        
}

/**
 *  태스크를 종료
 */
BOOL EndTask( QWORD qwTaskID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwTaskID;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ENDTASK, &stParameter );     
}

/**
 *  태스크가 자신을 종료함
 */
void exit( int iExitValue )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) iExitValue;

    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_EXITTASK, &stParameter );      
}

/**
 *  전체 태스크의 수를 반환
 */ 
int GetTaskCount( BYTE bAPICID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) bAPICID;

    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_GETTASKCOUNT, &stParameter );     
}

/**
 *  태스크가 존재하는지 여부를 반환
 */
BOOL IsTaskExist( QWORD qwID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwID;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISTASKEXIST, &stParameter );       
}

/**
 *  프로세서의 사용률을 반환
 */
QWORD GetProcessorLoad( BYTE bAPICID )
{
    PARAMETERTABLE stParameter;
     
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) bAPICID;

    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETPROCESSORLOAD, &stParameter );   
}

/**
 *  프로세서 친화도를 변경
 */
BOOL ChangeProcessorAffinity( QWORD qwTaskID, BYTE bAffinity )
{
    PARAMETERTABLE stParameter;
     
    // 파라미터 삽입
    PARAM( 0 ) = qwTaskID;
    PARAM( 1 ) = ( QWORD ) bAffinity;

    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_CHANGEPROCESSORAFFINITY, &stParameter );      
}

//==============================================================================
// GUI 시스템 관련
//==============================================================================
/**
 *  배경 윈도우의 ID를 반환
 */
QWORD GetBackgroundWindowID( void )
{
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETBACKGROUNDWINDOWID, NULL );         
}

/**
 *  화면 영역의 크기를 반환
 */
void GetScreenArea( RECT* pstScreenArea )
{
    PARAMETERTABLE stParameter;
     
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstScreenArea;

    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_GETSCREENAREA, &stParameter );     
}

/**
 *  윈도우를 생성
 */
QWORD CreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags,
        const char* pcTitle )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    PARAM( 2 ) = ( QWORD ) iWidth;
    PARAM( 3 ) = ( QWORD ) iHeight;
    PARAM( 4 ) = ( QWORD ) dwFlags;
    PARAM( 5 ) = ( QWORD ) pcTitle;
    
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_CREATEWINDOW, &stParameter );
}

/**
 *  윈도우를 삭제
 */
BOOL DeleteWindow( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DELETEWINDOW, &stParameter );    
}

/**
 *  윈도우를 화면에 나타내거나 숨김
 */
BOOL ShowWindow( QWORD qwWindowID, BOOL bShow )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) bShow;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_SHOWWINDOW, &stParameter );    
}

/**
 *  특정 위치를 포함하는 윈도우 중에서 가장 위에 있는 윈도우를 반환
 */
QWORD FindWindowByPoint( int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_FINDWINDOWBYPOINT, &stParameter );    
}

/**
 *  윈도우 제목이 일치하는 윈도우를 반환
 */
QWORD FindWindowByTitle( const char* pcTitle )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pcTitle;
    
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_FINDWINDOWBYTITLE, &stParameter );
}

/**
 *  윈도우가 존재하는지 여부를 반환
 */
BOOL IsWindowExist( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISWINDOWEXIST, &stParameter );    
}

/**
 *  최상위 윈도우의 ID를 반환
 */
QWORD GetTopWindowID( void )
{
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETTOPWINDOWID, NULL );     
}

/**
 *  윈도우의 Z 순서를 최상위로 만듦
 */
BOOL MoveWindowToTop( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_MOVEWINDOWTOTOP, &stParameter );      
}

/**
 *  X, Y좌표가 윈도우의 제목 표시줄 위치에 있는지를 반환
 */
BOOL IsInTitleBar( QWORD qwWindowID, int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISINTITLEBAR, &stParameter );     
}

/**
 *  X, Y좌표가 윈도우의 닫기 버튼 위치에 있는지를 반환
 */
BOOL IsInCloseButton( QWORD qwWindowID, int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISINCLOSEBUTTON, &stParameter );     
}

/**
 *  윈도우를 해당 위치로 이동
 */
BOOL MoveWindow( QWORD qwWindowID, int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_MOVEWINDOW, &stParameter );      
}

/**
 *  윈도우의 크기를 변경
 */
BOOL ResizeWindow( QWORD qwWindowID, int iX, int iY, int iWidth, int iHeight )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) iWidth;
    PARAM( 4 ) = ( QWORD ) iHeight;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_RESIZEWINDOW, &stParameter );    
}

/**
 *  윈도우 영역을 반환
 */
BOOL GetWindowArea( QWORD qwWindowID, RECT* pstArea )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstArea;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_GETWINDOWAREA, &stParameter );    
}

/**
 *  윈도우를 화면에 업데이트
 *      태스크에서 사용하는 함수
 */
BOOL UpdateScreenByID( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYID, &stParameter );      
}

/**
 *  윈도우의 내부를 화면에 업데이트
 *      태스크에서 사용하는 함수
 */
BOOL UpdateScreenByWindowArea( QWORD qwWindowID, const RECT* pstArea )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstArea;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYWINDOWAREA, &stParameter );     
}

/**
 *  화면 좌표로 화면을 업데이트
 *      태스크에서 사용하는 함수
 */
BOOL UpdateScreenByScreenArea( const RECT* pstArea )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pstArea;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYSCREENAREA, &stParameter );      
}

/**
 *  윈도우로 이벤트를 전송
 */
BOOL SendEventToWindow( QWORD qwWindowID, const EVENT* pstEvent )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstEvent;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_SENDEVENTTOWINDOW, &stParameter );      
}

/**
 *  윈도우의 이벤트 큐에 저장된 이벤트를 수신
 */
BOOL ReceiveEventFromWindowQueue( QWORD qwWindowID, EVENT* pstEvent )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstEvent;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_RECEIVEEVENTFROMWINDOWQUEUE, &stParameter );         
}

/**
 *  윈도우 화면 버퍼에 윈도우 테두리 그리기
 */
BOOL DrawWindowFrame( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWFRAME, &stParameter );      
}

/**
 *  윈도우 화면 버퍼에 배경 그리기
 */
BOOL DrawWindowBackground( QWORD qwWindowID )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWBACKGROUND, &stParameter );     
}

/**
 *  윈도우 화면 버퍼에 윈도우 제목 표시줄 그리기
 */
BOOL DrawWindowTitle( QWORD qwWindowID, const char* pcTitle, BOOL bSelectedTitle )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pcTitle;
    PARAM( 2 ) = ( QWORD ) bSelectedTitle;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWTITLE, &stParameter );        
}

/**
 *  윈도우 내부에 버튼 그리기
 */
BOOL DrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor,
        const char* pcText, COLOR stTextColor )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstButtonArea;
    PARAM( 2 ) = ( QWORD ) stBackgroundColor;
    PARAM( 3 ) = ( QWORD ) pcText;
    PARAM( 4 ) = ( QWORD ) stTextColor;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWBUTTON, &stParameter );      
}

/**
 *  윈도우 내부에 점 그리기
 */
BOOL DrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) stColor;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWPIXEL, &stParameter );     
}

/**
 *  윈도우 내부에 직선 그리기
 */
BOOL DrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX1;
    PARAM( 2 ) = ( QWORD ) iY1;
    PARAM( 3 ) = ( QWORD ) iX2;
    PARAM( 4 ) = ( QWORD ) iY2;
    PARAM( 5 ) = ( QWORD ) stColor;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWLINE, &stParameter );     
}

/**
 *  윈도우 내부에 사각형 그리기
 */
BOOL DrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2,
        COLOR stColor, BOOL bFill )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX1;
    PARAM( 2 ) = ( QWORD ) iY1;
    PARAM( 3 ) = ( QWORD ) iX2;
    PARAM( 4 ) = ( QWORD ) iY2;
    PARAM( 5 ) = ( QWORD ) stColor;
    PARAM( 6 ) = ( QWORD ) bFill;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWRECT, &stParameter );       
}

/**
 *  윈도우 내부에 원 그리기
 */
BOOL DrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor,
        BOOL bFill )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) iRadius;
    PARAM( 4 ) = ( QWORD ) stColor;
    PARAM( 5 ) = ( QWORD ) bFill;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWCIRCLE, &stParameter );      
}

/**
 *  윈도우 내부에 문자 출력
 */
BOOL DrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor,
        COLOR stBackgroundColor, const char* pcString, int iLength )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) stTextColor;
    PARAM( 4 ) = ( QWORD ) stBackgroundColor;
    PARAM( 5 ) = ( QWORD ) pcString;
    PARAM( 6 ) = ( QWORD ) iLength;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWTEXT, &stParameter );     
}

/**
 *  마우스 커서를 해당 위치로 이동해서 그려줌
 */
void MoveCursor( int iX, int iY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    
    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_MOVECURSOR, &stParameter );        
}

/**
 *  현재 마우스 커서의 위치를 반환
 */
void GetCursorPosition( int* piX, int* piY )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) piX;
    PARAM( 1 ) = ( QWORD ) piY;
    
    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_GETCURSORPOSITION, &stParameter );       
}

/**
 *  윈도우 화면 버퍼에 버퍼의 내용을 한번에 전송
 *      X, Y 좌표는 윈도우 내부 버퍼 기준
 */
BOOL BitBlt( QWORD qwWindowID, int iX, int iY, COLOR* pstBuffer, int iWidth, 
        int iHeight )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) pstBuffer;
    PARAM( 4 ) = ( QWORD ) iWidth;
    PARAM( 5 ) = ( QWORD ) iHeight;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_BITBLT, &stParameter );        
}

//==============================================================================
// JPEG 모듈 관련
//==============================================================================
/**
 *  JPEG 이미지 파일의 전체가 담긴 파일 버퍼와 크기를 이용해서 JPEG 자료구조를 초기화
 *      파일 버퍼의 내용을 분석하여 이미지 전체의 크기와 기타 정보를 JPEG 자료구조에 삽입 
 */
BOOL JPEGInit(JPEG *jpeg, BYTE* pbFileBuffer, DWORD dwFileSize)
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) jpeg;
    PARAM( 1 ) = ( QWORD ) pbFileBuffer;
    PARAM( 2 ) = ( QWORD ) dwFileSize;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_JPEGINIT, &stParameter );     
}

/**
 *  JPEG 자료구조에 저장된 정보를 이용하여 디코딩한 결과를 출력 버퍼에 저장
 */
BOOL JPEGDecode(JPEG *jpeg, COLOR* rgb)
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) jpeg;
    PARAM( 1 ) = ( QWORD ) rgb;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_JPEGDECODE, &stParameter );       
}

//==============================================================================
// RTC 관련
//==============================================================================
/**
 *  CMOS 메모리에서 RTC 컨트롤러가 저장한 현재 시간을 읽음
 */
BOOL ReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pbHour;
    PARAM( 1 ) = ( QWORD ) pbMinute;
    PARAM( 2 ) = ( QWORD ) pbSecond;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_READRTCTIME, &stParameter );        
}

/**
 *  CMOS 메모리에서 RTC 컨트롤러가 저장한 현재 일자를 읽음
 */
BOOL ReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, 
                  BYTE* pbDayOfWeek )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pwYear;
    PARAM( 1 ) = ( QWORD ) pbMonth;
    PARAM( 2 ) = ( QWORD ) pbDayOfMonth;
    PARAM( 3 ) = ( QWORD ) pbDayOfWeek;
    
    // 시스템 콜 호출
    return ( BOOL ) ExecuteSystemCall( SYSCALL_READRTCDATE, &stParameter );      
}

//==============================================================================
// 시리얼 통신 관련
//==============================================================================
/**
 *  시리얼 포트로 데이터를 송신
 */
void SendSerialData( BYTE* pbBuffer, int iSize )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pbBuffer;
    PARAM( 1 ) = ( QWORD ) iSize;
    
    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_SENDSERIALDATA, &stParameter );
}

/**
 *  시리얼 포트에서 데이터를 읽음
 */
int ReceiveSerialData( BYTE* pbBuffer, int iSize )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = ( QWORD ) pbBuffer;
    PARAM( 1 ) = ( QWORD ) iSize;
    
    // 시스템 콜 호출
    return ( int ) ExecuteSystemCall( SYSCALL_RECEIVESERIALDATA, &stParameter );
}

/**
 *  시리얼 포트 컨트롤러의 FIFO를 초기화
 */
void ClearSerialFIFO( void )
{
    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_CLEARSERIALFIFO, NULL );
}

//==============================================================================
// 기타
//==============================================================================
/**
 *  RAM 크기를 반환
 */
QWORD GetTotalRAMSize( void )
{
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETTOTALRAMSIZE, NULL );
}

/**
 *  Tick Count를 반환
 */
QWORD GetTickCount( void )
{
    // 시스템 콜 호출
    return ExecuteSystemCall( SYSCALL_GETTICKCOUNT, NULL );
}

/**
 *  밀리세컨드(milisecond) 동안 대기
 */
void Sleep( QWORD qwMillisecond )
{
    PARAMETERTABLE stParameter;
    
    // 파라미터 삽입
    PARAM( 0 ) = qwMillisecond;
    
    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_SLEEP, &stParameter );    
}

/**
 *  그래픽 모드인지 여부를 반환
 */
BOOL IsGraphicMode( void )
{
    // 시스템 콜 호출
    ExecuteSystemCall( SYSCALL_ISGRAPHICMODE, NULL );    
}
