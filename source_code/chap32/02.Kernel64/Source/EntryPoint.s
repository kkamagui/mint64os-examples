# file      EntryPoint.s
# date      2009/01/01
# author    kkamagui 
#           Copyright(c)2008 All rights reserved by kkamagui
# brief     IA-32e 모드 커널 엔트리 포인트에 관련된 소스 파일

[BITS 64]           ; 이하의 코드는 64비트 코드로 설정

SECTION .text       ; text 섹션(세그먼트)을 정의

; 외부에서 정의된 함수를 쓸 수 있도록 선언함(Import)
extern Main
; APIC ID 레지스터의 어드레스와 깨어난 코어의 개수
extern g_qwAPICIDAddress, g_iWakeUpApplicationProcessorCount

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x10        ; IA-32e 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    mov ds, ax          ; DS 세그먼트 셀렉터에 설정
    mov es, ax          ; ES 세그먼트 셀렉터에 설정
    mov fs, ax          ; FS 세그먼트 셀렉터에 설정
    mov gs, ax          ; GS 세그먼트 셀렉터에 설정

    ; 스택을 0x600000~0x6FFFFF 영역에 1MB 크기로 생성
    mov ss, ax          ; SS 세그먼트 셀렉터에 설정
    mov rsp, 0x6FFFF8   ; RSP 레지스터의 어드레스를 0x6FFFF8로 설정
    mov rbp, 0x6FFFF8   ; RBP 레지스터의 어드레스를 0x6FFFF8로 설정
    
    ; 부트 로더 영역의 Bootstrap Processor 플래그를 확인하여, Bootstrap Processor이면
    ; 바로 Main 함수로 이동
    cmp byte [ 0x7C09 ], 0x01
    je .BOOTSTRAPPROCESSORSTARTPOINT
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Application Processor만 실행하는 영역
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 스택의 꼭대기(Top)는 APIC ID를 이용해서 0x700000 이하로 이동
    ; 최대 16개 코어까지 지원 가능하므로 스택 영역인 1M를 16으로 나눈 값인 
    ; 64Kbyte(0x10000)만큼씩 아래로 이동하면서 설정 
    ; 로컬 APIC의 APIC ID 레지스터에서 ID를 추출. ID는 Bit 24~31에 위치함
    mov rax, 0                              ; RAX 레지스터 초기화
    mov rbx, qword [ g_qwAPICIDAddress ]    ; APIC ID 레지스터의 어드레스를 읽음
    mov eax, dword [ rbx ] ; APIC ID 레지스터에서 APIC ID를 읽음(비트 24~31)
    shr rax, 24            ; 비트 24~31에 존재하는 APIC ID를 시프트 시켜서 비트 0~7로 이동
    
    ; 추출한 APIC ID에 64Kbyte(0x10000)을 곱하여 스택의 꼭대기를 이동시킬 거리를 계산
    mov rbx, 0x10000       ; RBX 레지스터에 스택의 크기(64Kbyte)를 저장
    mul rbx                ; RAX 레지스터에 저장된 APIC ID와 RBX 레지스터의 스택 값을 곱함
    
    sub rsp, rax   ; RSP와 RBP 레지스터에서 RAX 레지스터에 저장된 값(스택의 꼭대기를
    sub rbp, rax   ; 이동시킬 거리)을 빼서 각 코어 별 스택을 할당해줌

    ; 깨어난 Application Processor 수를 1 증가시킴. lock 명령어를 사용하여 변수에
    ; 배타적(Exclusive) 접근이 가능하도록 함
    lock inc dword [ g_iWakeUpApplicationProcessorCount ]
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Bootstrap Processor와 Application Processor가 공통으로 실행하는 영역
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.BOOTSTRAPPROCESSORSTARTPOINT:
    call Main           ; C 언어 엔트리 포인트 함수(Main) 호출
    
    jmp $
