# file		makefile
# date		2008/11/12
# author	kkamagui 
# brief		보호 모드 커널을 빌드하기 위한 make 파일

# 기본적으로 빌드를 수행할 목록
all: Kernel32.bin

# 커널 엔트리 포인트 빌드
Kernel32.bin: Source/EntryPoint.s
	nasm -o Kernel32.bin $<

# 소스 파일을 제외한 나머지 파일 정리	
clean:
	rm -f Kernel32.bin
