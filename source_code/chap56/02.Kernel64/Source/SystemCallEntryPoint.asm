# file      SystemCallEntryPoint.asm
# date      2009/12/12
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     시스템 콜 엔트리 포인트에 관련된 소스 파일

[BITS 64]           ; 이하의 코드는 64비트 코드로 설정

SECTION .text       ; text 섹션(세그먼트)을 정의

; 외부에서 정의된 함수를 쓸 수 있도록 선언함(Import)
extern kProcessSystemCall

; C 언어에서 호출할 수 있도록 이름을 노출함(Export)
global kSystemCallEntryPoint, kSystemCallTestTask


; 유저 레벨에서 SYSCALL을 실행했을 때 호출되는 시스템 콜 엔트리 포인트
;   PARAM: QWORD qwServiceNumber, PARAMETERTABLE* pstParameter
kSystemCallEntryPoint:
    push rcx        ; SYSRET로 되돌아갈 때 사용할 RIP 레지스터의 값과 RFLAGS 레지스터의
    push r11        ; 값을 저장 
    mov cx, ds      ; CS와 SS 세그먼트 셀렉터를 제외한 나머지 세그먼트 셀렉터의 
    push cx         ; 값을 스택에 저장
    mov cx, es      ; DS 세그먼트 셀렉터와 ES 세그먼트 셀렉터는 스택에 바로 저장할 수
    push cx         ; 없으므로 CX 레지스터로 옮긴 뒤 저장
    push fs         
    push gs
    
    mov cx, 0x10    ; 커널 데이터 세그먼트 셀렉터의 인덱스를 CX 레지스터에 저장
    mov ds, cx      ; 세그먼트 셀렉터를 커널 데이터 세그먼트 셀렉터로 교체
    mov es, cx
    mov fs, cx
    mov gs, cx
    
    call kProcessSystemCall     ; 서비스 번호에 따라 커널 레벨 함수를 호출

    pop gs          ; 스택에 저장된 값으로 세그먼트 셀렉터의 값과 RCX, R11 
    pop fs          ; 레지스터를 복원
    pop cx
    mov es, cx
    pop cx
    mov ds, cx
    pop r11
    pop rcx

    o64 sysret      ; SYSCALL 명령어로 호출되었으므로 SYSRET 명령어로 복귀해야 함

; 시스템 콜을 테스트하는 유저 레벨 태스크
;   테스트 시스템 콜 서비스를 3번 연속으로 호출한 뒤 exit() 서비스를 호출하여 종료
;   PARAM: 없음
kSystemCallTestTask:
    mov rdi, 0xFFFFFFFF ; 서비스 번호에 테스트 시스템 콜(SYSCALL_TEST, 0xFFFFFFFF)를 저장
    mov rsi, 0x00   ; 시스템 콜을 호출할 때 사용하는 파라미터 테이블 포인터에 NULL을 저장
    syscall         ; 시스템 콜을 호출

    mov rdi, 0xFFFFFFFF ; 서비스 번호에 테스트 시스템 콜(SYSCALL_TEST, 0xFFFFFFFF)를 저장
    mov rsi, 0x00   ; 시스템 콜을 호출할 때 사용하는 파라미터 테이블 포인터에 NULL을 저장
    syscall         ; 시스템 콜을 호출

    mov rdi, 0xFFFFFFFF ; 서비스 번호에 테스트 시스템 콜(SYSCALL_TEST, 0xFFFFFFFF)를 저장
    mov rsi, 0x00   ; 시스템 콜을 호출할 때 사용하는 파라미터 테이블 포인터에 NULL을 저장
    syscall         ; 시스템 콜을 호출

    mov rdi, 24     ; 서비스 번호에 태스크 종료(SYSCALL_EXITTASK, 24)를 저장
    mov rsi, 0x00   ; 시스템 콜을 호출할 때 사용하는 파라미터 테이블 포인터에 NULL을 저장
    syscall         ; 시스템 콜을 호출
    jmp $
