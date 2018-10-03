/**
 *  file    Types.h
 *  date    2008/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   유저 레벨에서 사용하는 각종 타입이나 자료구조를 정의한 헤더 파일
 */

#ifndef __TYPES_H__
#define __TYPES_H__

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
//==============================================================================
//  기본 타입 관련 매크로
//==============================================================================
#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define QWORD   unsigned long
#define BOOL    unsigned char

#define TRUE    1
#define FALSE   0
#define NULL    0

//==============================================================================
//  콘솔 관련 매크로
//==============================================================================
// 콘솔의 너비(Width)와 높이(Height),그리고 비디오 메모리의 시작 어드레스 설정
#define CONSOLE_WIDTH                       80
#define CONSOLE_HEIGHT                      25


//==============================================================================
//  키보드에 관련된 매크로
//==============================================================================
// 키 상태에 대한 플래그
#define KEY_FLAGS_UP             0x00
#define KEY_FLAGS_DOWN           0x01
#define KEY_FLAGS_EXTENDEDKEY    0x02

// 스캔 코드 매핑 테이블에 대한 매크로
#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08

#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8A
#define KEY_F5          0x8B
#define KEY_F6          0x8C
#define KEY_F7          0x8D
#define KEY_F8          0x8E
#define KEY_F9          0x8F
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9A
#define KEY_PAGEDOWN    0x9B
#define KEY_INS         0x9C
#define KEY_DEL         0x9D
#define KEY_F11         0x9E
#define KEY_F12         0x9F
#define KEY_PAUSE       0xA0

//==============================================================================
//  태스크와 스케줄러 관련 매크로
//==============================================================================
// 태스크의 우선 순위
#define TASK_FLAGS_HIGHEST            0
#define TASK_FLAGS_HIGH               1
#define TASK_FLAGS_MEDIUM             2
#define TASK_FLAGS_LOW                3
#define TASK_FLAGS_LOWEST             4
#define TASK_FLAGS_WAIT               0xFF          

// 태스크의 플래그
#define TASK_FLAGS_ENDTASK            0x8000000000000000
#define TASK_FLAGS_SYSTEM             0x4000000000000000
#define TASK_FLAGS_PROCESS            0x2000000000000000
#define TASK_FLAGS_THREAD             0x1000000000000000
#define TASK_FLAGS_IDLE               0x0800000000000000
#define TASK_FLAGS_USERLEVEL          0x0400000000000000

// 함수 매크로
#define GETPRIORITY( x )            ( ( x ) & 0xFF )
#define SETPRIORITY( x, priority )  ( ( x ) = ( ( x ) & 0xFFFFFFFFFFFFFF00 ) | \
        ( priority ) )
#define GETTCBOFFSET( x )           ( ( x ) & 0xFFFFFFFF )

// 유효하지 않은 태스크 ID
#define TASK_INVALIDID              0xFFFFFFFFFFFFFFFF

// 프로세서 친화도 필드에 아래의 값이 설정되면, 해당 태스크는 특별한 요구사항이 없는 
// 것으로 판단하고 태스크 부하 분산 수행
#define TASK_LOADBALANCINGID        0xFF

//==============================================================================
//  파일 시스템 관련 매크로
//==============================================================================
// 파일 이름의 최대 길이
#define FILESYSTEM_MAXFILENAMELENGTH        24

// SEEK 옵션 정의
#define SEEK_SET                            0
#define SEEK_CUR                            1
#define SEEK_END                            2

// MINT 파일 시스템 타입과 필드를 표준 입출력의 타입으로 재정의
#define size_t      DWORD       
#define dirent      DirectoryEntryStruct
#define d_name      vcFileName


//==============================================================================
//  GUI 시스템 관련 매크로
//==============================================================================
// 색을 저장하는데 사용할 자료구조, 16비트 색을 사용하므로 WORD로 정의
typedef WORD    COLOR;

// 0~255 범위의 R, G, B를 16비트 색 형식으로 변환하는 매크로
// 0~255의 범위를 0~31, 0~63으로 축소하여 사용하므로 각각 8과 4로 나누어줘야 함
// 나누기 8과 나누기 4는 >> 3과 >> 2로 대체
#define RGB( r, g, b )      ( ( ( BYTE )( r ) >> 3 ) << 11 | \
                ( ( ( BYTE )( g ) >> 2 ) ) << 5 |  ( ( BYTE )( b ) >> 3 ) )

// 윈도우 제목의 최대 길이
#define WINDOW_TITLEMAXLENGTH       40

// 유효하지 않은 윈도우 ID
#define WINDOW_INVALIDID            0xFFFFFFFFFFFFFFFF

// 윈도우의 속성
// 윈도우를 화면에 나타냄
#define WINDOW_FLAGS_SHOW               0x00000001
// 윈도우 테두리 그림
#define WINDOW_FLAGS_DRAWFRAME          0x00000002
// 윈도우 제목 표시줄 그림
#define WINDOW_FLAGS_DRAWTITLE          0x00000004
// 윈도우 크기 변경 버튼을 그림
#define WINDOW_FLAGS_RESIZABLE          0x00000008
// 윈도우 기본 속성, 제목 표시줄과 프레임을 모두 그리고 화면에 윈도우를 보이게 설정
#define WINDOW_FLAGS_DEFAULT        ( WINDOW_FLAGS_SHOW | WINDOW_FLAGS_DRAWFRAME | \
                                      WINDOW_FLAGS_DRAWTITLE )

// 제목 표시줄의 높이
#define WINDOW_TITLEBAR_HEIGHT      21
// 윈도우의 닫기 버튼의 크기
#define WINDOW_XBUTTON_SIZE         19
// 윈도우의 최소 너비, 버튼 2개의 너비에 30픽셀의 여유 공간 확보
#define WINDOW_WIDTH_MIN            ( WINDOW_XBUTTON_SIZE * 2 + 30 )
// 윈도우의 최소 높이, 제목 표시줄의 높이에 30픽셀의 여유 공간 확보
#define WINDOW_HEIGHT_MIN           ( WINDOW_TITLEBAR_HEIGHT + 30 )

// 윈도우의 색깔
#define WINDOW_COLOR_FRAME                      RGB( 109, 218, 22 )
#define WINDOW_COLOR_BACKGROUND                 RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARTEXT               RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARACTIVEBACKGROUND   RGB( 79, 204, 11 )
#define WINDOW_COLOR_TITLEBARINACTIVEBACKGROUND RGB( 55, 135, 11 )
#define WINDOW_COLOR_TITLEBARBRIGHT1            RGB( 183, 249, 171 )
#define WINDOW_COLOR_TITLEBARBRIGHT2            RGB( 150, 210, 140 )
#define WINDOW_COLOR_TITLEBARUNDERLINE          RGB( 46, 59, 30 )
#define WINDOW_COLOR_BUTTONBRIGHT               RGB( 229, 229, 229 )
#define WINDOW_COLOR_BUTTONDARK                 RGB( 86, 86, 86 )
#define WINDOW_COLOR_SYSTEMBACKGROUND           RGB( 232, 255, 232 )
#define WINDOW_COLOR_XBUTTONLINECOLOR           RGB( 71, 199, 21 )

// 배경 윈도우의 제목
#define WINDOW_BACKGROUNDWINDOWTITLE            "SYS_BACKGROUND"

// 윈도우와 윈도우 매니저 태스크 사이에서 전달되는 이벤트의 종류
// 마우스 이벤트
#define EVENT_UNKNOWN                                   0
#define EVENT_MOUSE_MOVE                                1
#define EVENT_MOUSE_LBUTTONDOWN                         2
#define EVENT_MOUSE_LBUTTONUP                           3
#define EVENT_MOUSE_RBUTTONDOWN                         4
#define EVENT_MOUSE_RBUTTONUP                           5
#define EVENT_MOUSE_MBUTTONDOWN                         6
#define EVENT_MOUSE_MBUTTONUP                           7
// 윈도우 이벤트
#define EVENT_WINDOW_SELECT                             8
#define EVENT_WINDOW_DESELECT                           9
#define EVENT_WINDOW_MOVE                               10
#define EVENT_WINDOW_RESIZE                             11
#define EVENT_WINDOW_CLOSE                              12
// 키 이벤트
#define EVENT_KEY_DOWN                                  13
#define EVENT_KEY_UP                                    14

// 영문 폰트의 너비와 길이
#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

//==============================================================================
//  기타 매크로
//==============================================================================
#define MIN( x, y )     ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )
#define MAX( x, y )     ( ( ( x ) > ( y ) ) ? ( x ) : ( y ) )

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )

//==============================================================================
//  키보드에 관련된 자료구조
//==============================================================================
// 키 큐에 삽입할 데이터 구조체
typedef struct KeyDataStruct
{
    // 키보드에서 전달된 스캔 코드
    BYTE bScanCode;
    // 스캔 코드를 변환한 ASCII 코드
    BYTE bASCIICode;
    // 키 상태를 저장하는 플래그(눌림/떨어짐/확장 키 여부)
    BYTE bFlags;
} KEYDATA;

//==============================================================================
//  파일 시스템에 관련된 자료구조
//==============================================================================
// 디렉터리 엔트리 자료구조
typedef struct DirectoryEntryStruct
{
    // 파일 이름
    char vcFileName[ FILESYSTEM_MAXFILENAMELENGTH ];
    // 파일의 실제 크기
    DWORD dwFileSize;
    // 파일이 시작하는 클러스터 인덱스
    DWORD dwStartClusterIndex;
} DIRECTORYENTRY;

#pragma pack( pop )

// 파일을 관리하는 파일 핸들 자료구조
typedef struct kFileHandleStruct
{
    // 파일이 존재하는 디렉터리 엔트리의 오프셋
    int iDirectoryEntryOffset;
    // 파일 크기
    DWORD dwFileSize;
    // 파일의 시작 클러스터 인덱스
    DWORD dwStartClusterIndex;
    // 현재 I/O가 수행중인 클러스터의 인덱스
    DWORD dwCurrentClusterIndex;
    // 현재 클러스터의 바로 이전 클러스터의 인덱스
    DWORD dwPreviousClusterIndex;
    // 파일 포인터의 현재 위치
    DWORD dwCurrentOffset;
} FILEHANDLE;

// 디렉터리를 관리하는 디렉터리 핸들 자료구조
typedef struct kDirectoryHandleStruct
{
    // 루트 디렉터리를 저장해둔 버퍼
    DIRECTORYENTRY* pstDirectoryBuffer;
    
    // 디렉터리 포인터의 현재 위치
    int iCurrentOffset;
} DIRECTORYHANDLE;

// 파일과 디렉터리에 대한 정보가 들어있는 자료구조
typedef struct kFileDirectoryHandleStruct
{
    // 자료구조의 타입 설정. 파일 핸들이나 디렉터리 핸들, 또는 빈 핸들 타입 지정 가능
    BYTE bType;

    // bType의 값에 따라 파일 또는 디렉터리로 사용
    union
    {
        // 파일 핸들
        FILEHANDLE stFileHandle;
        // 디렉터리 핸들
        DIRECTORYHANDLE stDirectoryHandle;
    };    
} FILE, DIR;

//==============================================================================
//  GUI 시스템에 관련된 자료구조
//==============================================================================
// 사각형의 정보를 담는 자료구조
typedef struct kRectangleStruct
{
    // 왼쪽 위(시작점)의 X 좌표
    int iX1;
    // 왼쪽 위(시작점)의 Y 좌표
    int iY1;
    
    // 오른쪽 아래(끝점)의 X 좌표
    int iX2;
    // 오른쪽 아래(끝점)의 Y좌표
    int iY2;
} RECT;

// 점의 정보를 담는 자료구조
typedef struct kPointStruct
{
    // X와 Y의 좌표
    int iX;
    int iY;
} POINT;

// 마우스 이벤트 자료구조
typedef struct kMouseEventStruct
{
    // 윈도우 ID
    QWORD qwWindowID;

    // 마우스 X,Y좌표와 버튼의 상태
    POINT stPoint;
    BYTE bButtonStatus;
} MOUSEEVENT;

// 키 이벤트 자료구조
typedef struct kKeyEventStruct
{
    // 윈도우 ID
    QWORD qwWindowID;
    
    // 키의 ASCII 코드와 스캔 코드
    BYTE bASCIICode;
    BYTE bScanCode;    
    
    // 키 플래그
    BYTE bFlags;
} KEYEVENT;

// 윈도우 이벤트 자료구조
typedef struct kWindowEventStruct
{
    // 윈도우 ID
    QWORD qwWindowID;
    
    // 영역 정보
    RECT stArea;
} WINDOWEVENT;

// 이벤트 자료구조
typedef struct kEventStruct
{
    // 이벤트 타입
    QWORD qwType;
    
    // 이벤트 데이터 영역을 정의한 공용체
    union
    {
        // 마우스 이벤트 관련 데이터
        MOUSEEVENT stMouseEvent;

        // 키 이벤트 관련 데이터
        KEYEVENT stKeyEvent;

        // 윈도우 이벤트 관련 데이터
        WINDOWEVENT stWindowEvent;

        // 위의 이벤트 외에 유저 이벤트를 위한 데이터
        QWORD vqwData[ 3 ];
    };
} EVENT;

//==============================================================================
//  JPEG 디코더에 관련된 자료구조
//==============================================================================
// 허프만 테이블
typedef struct{
    int elem; // 요소 개수
    unsigned short code[256];
    unsigned char  size[256];
    unsigned char  value[256];
}HUFF;

// JPEG 디코딩을 위한 자료구조
typedef struct{
    // SOF
    int width;
    int height;
    // MCU
    int mcu_width;
    int mcu_height;

    int max_h,max_v;
    int compo_count;
    int compo_id[3];
    int compo_sample[3];
    int compo_h[3];
    int compo_v[3];
    int compo_qt[3];

    // SOS
    int scan_count;
    int scan_id[3];
    int scan_ac[3];
    int scan_dc[3];
    int scan_h[3];  // 샘플링 요소 수
    int scan_v[3];  // 샘플링 요소 수
    int scan_qt[3]; // 양자화 테이블 인덱스
    
    // DRI
    int interval;

    int mcu_buf[32*32*4]; // 버퍼
    int *mcu_yuv[4];
    int mcu_preDC[3];
    
    // DQT
    int dqt[3][64];
    int n_dqt;
    
    // DHT
    HUFF huff[2][3];
    
    
    // i/o
    unsigned char *data;
    int data_index;
    int data_size;
    
    unsigned long bit_buff;
    int bit_remain;
    
}JPEG;

#endif /*TYPES_H_*/
