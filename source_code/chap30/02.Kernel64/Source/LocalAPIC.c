/**
 *  file    LocalAPIC.c
 *  date    2009/06/28
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   로컬 APIC(Local APIC)에 관련된 소스 파일
 */
#include "LocalAPIC.h"
#include "MPConfigurationTable.h"

/**
 *  로컬 APIC의 메모리 맵 I/O 어드레스를 반환
 */
QWORD kGetLocalAPICBaseAddress( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
    return pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
}

/**
 *  의사 인터럽트 벡터 레지스터(Spurious Interrupt Vector Register)에 있는 
 *  APIC 소프트웨어 활성/비활성 필드를 1로 설정하여 로컬 APIC를 활성화함
 */
void kEnableSoftwareLocalAPIC( void )
{
    QWORD qwLocalAPICBaseAddress;
    
    // MP 설정 테이블 헤더에 저장된 로컬 APIC의 메모리 맵 I/O 어드레스를 사용
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();
    
    // 의사 인터럽트 벡터 레지스터(Spurious Interrupt Vector Register, 0xFEE000F0)의 
    // APIC 소프트웨어 활성/비활성 필드(비트 8)를 1로 설정해서 로컬 APIC를 활성화
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_SVR ) |= 0x100;
}
