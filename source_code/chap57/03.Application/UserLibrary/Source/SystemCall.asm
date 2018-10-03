# file      SystemCall.asm
# date      2009/12/12
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     유저 레벨에서 시스템 콜을 호출하는데 관련된 소스 파일

[BITS 64]           ; 이하의 코드는 64비트 코드로 설정

SECTION .text       ; text 섹션(세그먼트)을 정의

; 외부에서 호출할 수 있도록 이름을 노출함(Export)
global _START, ExecuteSystemCall

; 외부에서 정의된 함수를 쓸 수 있도록 선언함(Import)
extern Main, exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   코드 영역
;       응용프로그램이 로딩되었을 때 실제로 실행을 시작할 엔트리 포인트
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_START:
    call Main       ; C 언어 엔트리 포인트 함수(Main) 호출

    mov rdi, rax    ; 파라미터에 Main 함수의 반환 값(RAX 레지스터)을 저장
    call exit       ; 프로세스 종료 함수를 호출

    jmp $           ; 위에서 프로세스가 종료되므로 여기는 실행되지 않음

    ret

; 시스템 콜을 실행
;   PARAM: QWORD qwServiceNumber, PARAMETERTABLE* pstParameter
ExecuteSystemCall:
    push rcx        ; SYSCALL을 호출할 때 RCX 레지스터에 RIP 레지스터가 저장되고 
    push r11        ; R11 레지스터에 RFLAGS 레지스터가 저장되므로 스택에 보관
    
    syscall         ; SYSCALL 수행
    
    pop r11         ; 스택에 저장된 값으로 RCX 레지스터와 R11 레지스터를 복원
    pop rcx
    ret
