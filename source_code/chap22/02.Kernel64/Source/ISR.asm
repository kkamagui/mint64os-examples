# file		ISR.asm
# date      2009/01/24
# author	kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief		인터럽트 서비스 루틴(ISR) 관련된 소스 파일

[BITS 64]           ; 이하의 코드는 64비트 코드로 설정

SECTION .text       ; text 섹션(세그먼트)을 정의

; 외부에서 정의된 함수를 쓸 수 있도록 선언함(Import)
extern kCommonExceptionHandler, kCommonInterruptHandler, kKeyboardHandler
extern kTimerHandler, kDeviceNotAvailableHandler

; C 언어에서 호출할 수 있도록 이름을 노출함(Export)
; 예외(Exception) 처리를 위한 ISR
global kISRDivideError, kISRDebug, kISRNMI, kISRBreakPoint, kISROverflow
global kISRBoundRangeExceeded, kISRInvalidOpcode, kISRDeviceNotAvailable, kISRDoubleFault,
global kISRCoprocessorSegmentOverrun, kISRInvalidTSS, kISRSegmentNotPresent
global kISRStackSegmentFault, kISRGeneralProtection, kISRPageFault, kISR15
global kISRFPUError, kISRAlignmentCheck, kISRMachineCheck, kISRSIMDError, kISRETCException

; 인터럽트(Interrupt) 처리를 위한 ISR
global kISRTimer, kISRKeyboard, kISRSlavePIC, kISRSerial2, kISRSerial1, kISRParallel2
global kISRFloppy, kISRParallel1, kISRRTC, kISRReserved, kISRNotUsed1, kISRNotUsed2
global kISRMouse, kISRCoprocessor, kISRHDD1, kISRHDD2, kISRETCInterrupt

; 콘텍스트를 저장하고 셀렉터를 교체하는 매크로
%macro KSAVECONTEXT 0       ; 파라미터를 전달받지 않는 KSAVECONTEXT 매크로 정의
    ; RBP 레지스터부터 GS 세그먼트 셀렉터까지 모두 스택에 삽입
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov ax, ds      ; DS 세그먼트 셀렉터와 ES 세그먼트 셀렉터는 스택에 직접
    push rax        ; 삽입할 수 없으므로, RAX 레지스터에 저장한 후 스택에 삽입
    mov ax, es
    push rax
    push fs
    push gs	

    ; 세그먼트 셀렉터 교체
    mov ax, 0x10    ; AX 레지스터에 커널 데이터 세그먼트 디스크립터 저장
    mov ds, ax      ; DS 세그먼트 셀렉터부터 FS 세그먼트 셀렉터까지 모두
    mov es, ax      ; 커널 데이터 세그먼트로 교체
   	mov gs, ax
   	mov fs, ax
%endmacro           ; 매크로 끝


; 콘텍스트를 복원하는 매크로
%macro KLOADCONTEXT 0   ; 파라미터를 전달받지 않는 KSAVECONTEXT 매크로 정의
    ; GS 세그먼트 셀렉터부터 RBP 레지스터까지 모두 스택에서 꺼내 복원
    pop gs
    pop fs
    pop rax
    mov es, ax      ; ES 세그먼트 셀렉터와 DS 세그먼트 셀렉터는 스택에서 직접
    pop rax         ; 꺼내 복원할 수 없으므로, RAX 레지스터에 저장한 뒤에 복원
    mov ds, ax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp        
%endmacro       ; 매크로 끝

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	예외 핸들러
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #0, Divide Error ISR
kISRDivideError:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 0
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #1, Debug ISR
kISRDebug:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 1
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #2, NMI ISR
kISRNMI:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 2
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #3, BreakPoint ISR
kISRBreakPoint:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 3
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #4, Overflow ISR
kISROverflow:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 4
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #5, Bound Range Exceeded ISR
kISRBoundRangeExceeded:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 5
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #6, Invalid Opcode ISR
kISRInvalidOpcode:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 6
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #7, Device Not Available ISR
kISRDeviceNotAvailable:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 7
    call kDeviceNotAvailableHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #8, Double Fault ISR
kISRDoubleFault:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호와 에러 코드를 삽입하고 핸들러 호출
    mov rdi, 8
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #9, Coprocessor Segment Overrun ISR
kISRCoprocessorSegmentOverrun:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 9
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #10, Invalid TSS ISR
kISRInvalidTSS:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호와 에러 코드를 삽입하고 핸들러 호출
    mov rdi, 10
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #11, Segment Not Present ISR
kISRSegmentNotPresent:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호와 에러 코드를 삽입하고 핸들러 호출
    mov rdi, 11
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #12, Stack Segment Fault ISR
kISRStackSegmentFault:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호와 에러 코드를 삽입하고 핸들러 호출
    mov rdi, 12
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #13, General Protection ISR
kISRGeneralProtection:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호와 에러 코드를 삽입하고 핸들러 호출
    mov rdi, 13
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #14, Page Fault ISR
kISRPageFault:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호와 에러 코드를 삽입하고 핸들러 호출
    mov rdi, 14
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #15, Reserved ISR
kISR15:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 15
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #16, FPU Error ISR
kISRFPUError:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 16
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #17, Alignment Check ISR
kISRAlignmentCheck:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 17
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    add rsp, 8      ; 에러 코드를 스택에서 제거
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #18, Machine Check ISR
kISRMachineCheck:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 18
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #19, SIMD Floating Point Exception ISR
kISRSIMDError:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 19
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #20~#31, Reserved ISR
kISRETCException:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 예외 번호를 삽입하고 핸들러 호출
    mov rdi, 20
    call kCommonExceptionHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	인터럽트 핸들러
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #32, 타이머 ISR
kISRTimer:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 32    
    call kTimerHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #33, 키보드 ISR
kISRKeyboard:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 33
    call kKeyboardHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원

; #34, 슬레이브 PIC ISR
kISRSlavePIC:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 34
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #35, 시리얼 포트 2 ISR
kISRSerial2:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 35
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #36, 시리얼 포트 1 ISR
kISRSerial1:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 36
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #37, 패러렐 포트 2 ISR
kISRParallel2:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 37
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #38, 플로피 디스크 컨트롤러 ISR
kISRFloppy:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 38
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #39, 패러렐 포트 1 ISR
kISRParallel1:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 39
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #40, RTC ISR
kISRRTC:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 40
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #41, 예약된 인터럽트의 ISR
kISRReserved:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 41
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #42, 사용 안함
kISRNotUsed1:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 42
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #43, 사용 안함
kISRNotUsed2:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 43
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #44, 마우스 ISR
kISRMouse:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 44
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #45, 코프로세서 ISR
kISRCoprocessor:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 45
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #46, 하드 디스크 1 ISR
kISRHDD1:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 46
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
    
; #47, 하드 디스크 2 ISR
kISRHDD2:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 47
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
      
; #48 이외의 모든 인터럽트에 대한 ISR              
kISRETCInterrupt:
    KSAVECONTEXT    ; 콘텍스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체
    
    ; 핸들러에 인터럽트 번호를 삽입하고 핸들러 호출
    mov rdi, 48
    call kCommonInterruptHandler
    
    KLOADCONTEXT    ; 콘텍스트를 복원
    iretq           ; 인터럽트 처리를 완료하고 이전에 수행하던 코드로 복원
