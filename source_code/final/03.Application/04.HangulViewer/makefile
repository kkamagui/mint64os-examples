# file		makefile
# date		2009/01/02
# author	kkamagui 
# brief		응용 프로그램을 빌드하기 위한 make 파일

################################################################################
# 빌드 환경 및 규칙 설정 
################################################################################
# 컴파일러와 링커, 그리고 라이브러리 정의
NASM64 = nasm -f elf64
GCC64 = x86_64-pc-linux-gcc.exe -c -m64 -ffreestanding -mcmodel=large -fno-common
LD64 = x86_64-pc-linux-ld.exe -melf_x86_64 -T ../elf_x86_64.x -nostdlib -e _START -Ttext 0x0000
LDFLAGS = -lMINT64OS -L../../UserLibrary
CFLAGS = -I../../UserLibrary/Source

# 응용 프로그램 이미지 생성을 위한 OBJCOPY 옵션 정의
OBJCOPY64 = x86_64-pc-linux-objcopy -j .text -j .data -j .rodata -j .bss

# 디렉터리 정의
OBJECTDIRECTORY = Temp
SOURCEDIRECTORY = Source

################################################################################
# 빌드 항목 및 빌드 방법 설정
################################################################################
# 기본적으로 빌드를 수행할 목록
all: prepare Output.elf

# 오브젝트 파일이 위치할 디렉터리를 생성
prepare:
	mkdir -p $(OBJECTDIRECTORY)


# 커널의 C 소스 파일에 대한 의존성 정보 생성
dep:
	@echo === Make Dependancy File ===
	make -C $(OBJECTDIRECTORY) -f ../makefile InternalDependency
	@echo === Dependancy Search Complete ===

# 디렉터리를 오브젝트 파일 디렉터리로 이동해서 의존성 파일 및 실행 파일을 생성
ExecuteInternalBuild: dep
	make -C $(OBJECTDIRECTORY) -f ../makefile Main.elf

# 커널 이미지를 바이너리 파일로 변환하여 IA-32e 모드 바이너리 생성
Output.elf: ExecuteInternalBuild
	$(OBJCOPY64) $(OBJECTDIRECTORY)/Main.elf $@
		
# 소스 파일을 제외한 나머지 파일 정리	
clean:
	rm -f *.elf
	rm -f $(OBJECTDIRECTORY)/*.*

################################################################################
# Make에 의해 다시 호출되는 부분, Temp 디렉터리를 기준으로 수행됨
################################################################################
# 빌드할 C 소스 파일 정의, Temp 디렉터리를 기준으로 설정
CSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.asm)
COBJECTFILES = $(notdir $(patsubst %.c,%.o,$(CSOURCEFILES)))
ASSEMBLYOBJECTFILES = $(notdir $(patsubst %.asm,%.o,$(ASSEMBLYSOURCEFILES)))

# .c 파일을 .o 파일로 바꾸는 규칙 정의
%.o: ../$(SOURCEDIRECTORY)/%.c
	$(GCC64) -c $< $(CFLAGS)

# .asm 파일을 .o 파일로 바꾸는 규칙 정의
%.o: ../$(SOURCEDIRECTORY)/%.asm
	$(NASM64) -o $@ $<

# 의존성에 관련된 파일을 생성
InternalDependency:
	$(GCC64) -MM $(CSOURCEFILES) > Dependency.dep $(CFLAGS)

# 실제 응용 프로그램 실행 파일을 빌드
Main.elf: $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)
	$(LD64) -r -o $@ $^ $(LDFLAGS)

# 현재 디렉터리의 파일 중, dependency 파일이 있으면 make에 포함	
ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
