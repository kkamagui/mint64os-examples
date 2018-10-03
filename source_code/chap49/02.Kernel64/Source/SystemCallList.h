/**
 *  file    SystemCallList.h
 *  date    2009/12/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   MINT64 OS에서 지원하는 시스템 콜 목록을 정의한 헤더 파일
 */

#ifndef __SYSTEMCALLLIST_H__
#define __SYSTEMCALLLIST_H__

// 콘솔 I/O 관련
#define SYSCALL_CONSOLEPRINTSTRING          0 
#define SYSCALL_SETCURSOR                   1 
#define SYSCALL_GETCURSOR                   2 
#define SYSCALL_CLEARSCREEN                 3 
#define SYSCALL_GETCH                       4 

// 동적 메모리 할당/해제 관련
#define SYSCALL_MALLOC                      5 
#define SYSCALL_FREE                        6 

// 파일과 디렉터리 I/O 관련
#define SYSCALL_FOPEN                       7 
#define SYSCALL_FREAD                       8 
#define SYSCALL_FWRITE                      9 
#define SYSCALL_FSEEK                       10
#define SYSCALL_FCLOSE                      11
#define SYSCALL_REMOVE                      12
#define SYSCALL_OPENDIR                     13
#define SYSCALL_READDIR                     14
#define SYSCALL_REWINDDIR                   15
#define SYSCALL_CLOSEDIR                    16
#define SYSCALL_ISFILEOPENED                17

// 하드 디스크 I/O 관련
#define SYSCALL_READHDDSECTOR               18
#define SYSCALL_WRITEHDDSECTOR              19

// 태스크와 스케줄러 관련
#define SYSCALL_CREATETASK                  20
#define SYSCALL_SCHEDULE                    21
#define SYSCALL_CHANGEPRIORITY              22
#define SYSCALL_ENDTASK                     23
#define SYSCALL_EXITTASK                    24
#define SYSCALL_GETTASKCOUNT                25
#define SYSCALL_ISTASKEXIST                 26
#define SYSCALL_GETPROCESSORLOAD            27
#define SYSCALL_CHANGEPROCESSORAFFINITY     28

// GUI 시스템 관련
#define SYSCALL_GETBACKGROUNDWINDOWID       31
#define SYSCALL_GETSCREENAREA               32
#define SYSCALL_CREATEWINDOW                33
#define SYSCALL_DELETEWINDOW                34
#define SYSCALL_SHOWWINDOW                  35
#define SYSCALL_FINDWINDOWBYPOINT           36
#define SYSCALL_FINDWINDOWBYTITLE           37
#define SYSCALL_ISWINDOWEXIST               38
#define SYSCALL_GETTOPWINDOWID              39
#define SYSCALL_MOVEWINDOWTOTOP             40
#define SYSCALL_ISINTITLEBAR                41
#define SYSCALL_ISINCLOSEBUTTON             42
#define SYSCALL_MOVEWINDOW                  43
#define SYSCALL_RESIZEWINDOW                44
#define SYSCALL_GETWINDOWAREA               45
#define SYSCALL_UPDATESCREENBYID            46
#define SYSCALL_UPDATESCREENBYWINDOWAREA    47
#define SYSCALL_UPDATESCREENBYSCREENAREA    48
#define SYSCALL_SENDEVENTTOWINDOW           49
#define SYSCALL_RECEIVEEVENTFROMWINDOWQUEUE 50
#define SYSCALL_DRAWWINDOWFRAME             51
#define SYSCALL_DRAWWINDOWBACKGROUND        52
#define SYSCALL_DRAWWINDOWTITLE             53
#define SYSCALL_DRAWBUTTON                  54
#define SYSCALL_DRAWPIXEL                   55
#define SYSCALL_DRAWLINE                    56
#define SYSCALL_DRAWRECT                    57
#define SYSCALL_DRAWCIRCLE                  58
#define SYSCALL_DRAWTEXT                    59
#define SYSCALL_MOVECURSOR                  60
#define SYSCALL_GETCURSORPOSITION           61
#define SYSCALL_BITBLT                      62

// JPEG 디코더 관련
#define SYSCALL_JPEGINIT                    63
#define SYSCALL_JPEGDECODE                  64

// RTC 관련
#define SYSCALL_READRTCTIME                 65
#define SYSCALL_READRTCDATE                 66

// 시리얼 I/O 관련
#define SYSCALL_SENDSERIALDATA              67
#define SYSCALL_RECEIVESERIALDATA           68
#define SYSCALL_CLEARSERIALFIFO             69

// 기타
#define SYSCALL_GETTOTALRAMSIZE             70
#define SYSCALL_GETTICKCOUNT                71
#define SYSCALL_SLEEP                       72
#define SYSCALL_ISGRAPHICMODE               73
#define SYSCALL_TEST                        0xFFFFFFFF

#endif /*__SYSTEMCALLLIST_H_*/
