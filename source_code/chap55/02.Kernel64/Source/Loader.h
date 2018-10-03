/**
 *  file    ApplicationLoader.h
 *  date    2009/12/26
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   응용프로그램을 로드하여 실행하는 로더(Loader)에 관련된 함수를 정의한 헤더 파일
 */

#ifndef __LOADER_H__
#define __LOADER_H__

#include "Types.h"
#include "Task.h"

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 기본 데이터 타입을 정의한 매크로
#define Elf64_Addr      unsigned long
#define Elf64_Off       unsigned long
#define Elf64_Half      unsigned short int
#define Elf64_Word      unsigned int
#define Elf64_Sword     int
#define Elf64_Xword     unsigned long
#define Elf64_Sxword    long

// e_ident[]의 index 의미
#define EI_MAG0         0 
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_OSABI        7
#define EI_ABIVERSION   8
#define EI_PAD          9
#define EI_NIDENT       16

// e_ident[EI_MAGX] 
#define ELFMAG0         0x7F
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

// e_ident[EI_CLASS]
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

// e_ident[EI_DATA]
#define ELFDATANONE     0 
#define ELFDATA2LSB     1 
#define ELFDATA2MSB     2 

// e_ident[OSABI]
#define ELFOSABI_NONE       0
#define ELFOSABI_HPUX       1
#define ELFOSABI_NETBSD     2
#define ELFOSABI_LINUX      3
#define ELFOSABI_SOLARIS    6
#define ELFOSABI_AIX        7
#define ELFOSABI_FREEBSD    9

// e_type
#define ET_NONE         0
#define ET_REL          1
#define ET_EXEC         2
#define ET_DYN          3
#define ET_CORE         4
#define ET_LOOS         0xFE00
#define ET_HIOS         0xFEFF
#define ET_LOPROC       0xFF00
#define ET_HIPROC       0xFFFF

// e_machine
#define EM_NONE         0
#define EM_M32          1
#define EM_SPARC        2
#define EM_386          3
#define EM_PPC          20
#define EM_PPC64        21
#define EM_ARM          40
#define EM_IA_64        50
#define EM_X86_64       62
#define EM_AVR          83
#define EM_AVR32        185
#define EM_CUDA         190

// 특별한 섹션 인덱스(Special Section Index)
#define SHN_UNDEF       0
#define SHN_LOERSERVE   0xFF00
#define SHN_LOPROC      0xFF00
#define SHN_HIPROC      0xFF1F
#define SHN_LOOS        0xFF20
#define SHN_HIOS        0xFF3F
#define SHN_ABS         0xFFF1
#define SHN_COMMON      0xFFF2
#define SHN_XINDEX      0xFFFF
#define SHN_HIRESERVE   0xFFFF

// sh_type
#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_LOOS        0x60000000
#define SHT_HIOS        0x6FFFFFFF
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7FFFFFFF
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xFFFFFFFF

// sh_flags
#define SHF_WRITE       1
#define SHF_ALLOC       2
#define SHF_EXECINSTR   4
#define SHF_MASKOS      0x0FF00000
#define SHF_MASKPROC    0xF0000000

// Special Section Index
#define SHN_UNDEF       0
#define SHN_LORESERVE   0xFF00
#define SHN_LOPROC      0xFF00
#define SHN_HIPROC      0xFF1F
#define SHN_ABS         0xFFF1
#define SHN_COMMON      0xFFF2
#define SHN_HIRESERVE   0xFFFF

// Relocation Type
#define R_X86_64_NONE       0       // none
#define R_X86_64_64         1       // word64   S + A
#define R_X86_64_PC32       2       // word32   S + A - P
#define R_X86_64_GOT32      3       // word32   G + A
#define R_X86_64_PLT32      4       // word32   L + A - P
#define R_X86_64_COPY       5       // none
#define R_X86_64_GLOB_DAT   6       // word64   S
#define R_X86_64_JUMP_SLOT  7       // word64   S
#define R_X86_64_RELATIVE   8       // word64   B + A
#define R_X86_64_GOTPCREL   9       // word32   G + GOT + A - P
#define R_X86_64_32         10      // word32   S + A
#define R_X86_64_32S        11      // word32   S + A
#define R_X86_64_16         12      // word16   S + A
#define R_X86_64_PC16       13      // word16   S + A - P
#define R_X86_64_8          14      // word8    S + A
#define R_X86_64_PC8        15      // word8    S + A - P
#define R_X86_64_DPTMOD64   16      // word64
#define R_X86_64_DTPOFF64   17      // word64
#define R_X86_64_TPOFF64    18      // word64
#define R_X86_64_TLSGD      19      // word32
#define R_X86_64_TLSLD      20      // word32
#define R_X86_64_DTPOFF32   21      // word32
#define R_X86_64_GOTTPOFF   22      // word32
#define R_X86_64_TPOFF32    23      // word32
#define R_X86_64_PC64       24      // word64   S + A - P
#define R_X86_64_GOTOFF64   25      // word64   S + A - GOT
#define R_X86_64_GOTPC32    26      // word32   GOT + A - P 
#define R_X86_64_SIZE32     32      // word32   Z + A
#define R_X86_64_SIZE64     33      // word64   Z + A

// 상위 32비트와 하위 32비트 값을 추출하는 매크로
#define RELOCATION_UPPER32( x )     ( ( x ) >> 32 )
#define RELOCATION_LOWER32( x )     ( ( x ) & 0xFFFFFFFF )

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// ELF64 파일 포맷의 ELF 헤더 자료구조
typedef struct
{
    unsigned char e_ident[16];      // ELF 식별자(Identification)
    Elf64_Half e_type;              // 오브젝트 파일 형식
    Elf64_Half e_machine;           // 머신(Machine) 타입
    Elf64_Word e_version;           // 오브젝트 파일 버전
    Elf64_Addr e_entry;             // 엔트리 포인트 어드레스
    Elf64_Off e_phoff;              // 파일 내에 존재하는 프로그램 헤더 테이블의 위치
    Elf64_Off e_shoff;              // 파일 내에 존재하는 섹션 헤더 테이블의 위치
    Elf64_Word e_flags;             // 프로세서 의존적인(Processor-specific) 플래그
    Elf64_Half e_ehsize;            // ELF 헤더의 크기
    Elf64_Half e_phentsize;         // 프로그램 헤더 엔트리 한 개의 크기
    Elf64_Half e_phnum;             // 프로그램 헤더 엔트리의 개수
    Elf64_Half e_shentsize;         // 섹션 헤더 엔트리 한 개의 크기
    Elf64_Half e_shnum;             // 섹션 헤더 엔트리의 개수
    Elf64_Half e_shstrndx;          // 섹션 이름 문자열이 저장된 섹션 헤더의 인덱스
} Elf64_Ehdr;

// ELF64의 섹션 헤더 자료구조
typedef struct
{
    Elf64_Word sh_name;             // 섹션 이름이 저장된 오프셋
    Elf64_Word sh_type;             // 섹션 타입
    Elf64_Xword sh_flags;           // 섹션 플래그
    Elf64_Addr sh_addr;             // 메모리에 로딩할 어드레스
    Elf64_Off sh_offset;            // 파일 내에 존재하는 섹션의 오프셋
    Elf64_Xword sh_size;            // 섹션 크기
    Elf64_Word sh_link;             // 연결된 다른 섹션
    Elf64_Word sh_info;             // 부가적인 정보
    Elf64_Xword sh_addralign;       // 어드레스 정렬
    Elf64_Xword sh_entsize;         // 섹션에 들어있는 데이터 엔트리의 크기
} Elf64_Shdr;

// ELF64의 심볼 테이블 엔트리 자료구조
typedef struct
{
    Elf64_Word st_name;             // 심볼 이름이 저장된 오프셋
    unsigned char st_info;          // 심볼 타입과 바인딩(Binding) 속성
    unsigned char st_other;         // 예약됨(Reserved)
    Elf64_Half st_shndx;            // 심볼이 정의된 섹션 헤더의 인덱스
    Elf64_Addr st_value;            // 심볼의 값
    Elf64_Xword st_size;            // 심볼의 크기
} Elf64_Sym;

// ELF64의 재배치 엔트리 자료구조(SHT_REL 섹션 타입)
typedef struct
{
    Elf64_Addr r_offset;            // 재배치를 수행할 어드레스
    Elf64_Xword r_info;             // 심볼의 인덱스와 재배치 타입
} Elf64_Rel;

// ELF64의 재배치 엔트리 자료구조(SHT_RELA 섹션 타입)
typedef struct
{
    Elf64_Addr r_offset;            // 재배치를 수행할 어드레스
    Elf64_Xword r_info;             // 심볼의 인덱스와 재배치 타입
    Elf64_Sxword r_addend;          // 더하는 수(상수 부분)
} Elf64_Rela;

#pragma pack( pop)

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
QWORD kExecuteProgram( const char* pcFileName, const char* pcArgumentString, 
        BYTE bAffinity );
static BOOL kLoadProgramAndRelocation( BYTE* pbFileBuffer, 
        QWORD* pqwApplicationMemoryAddress, QWORD* pqwApplicationMemorySize, 
        QWORD* pqwEntryPointAddress );
static BOOL kRelocation( BYTE* pbFileBuffer );
static void kAddArgumentStringToTask( TCB* pstTask, const char* pcArgumentString );
QWORD kCreateThread( QWORD qwEntryPoint, QWORD qwArgument, BYTE bAffinity, 
        QWORD qwExitFunction );

#endif /*__LOADER_H__*/
