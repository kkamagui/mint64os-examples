# file		makefile
# date		2008/11/12
# author	kkamagui 
# brief		부트 로더를 빌드하기 위한 make 파일

# 기본적으로 빌드를 수행할 목록
all: BootLoader.bin

# 부트 로더 빌드
BootLoader.bin: BootLoader.asm
	nasm -o BootLoader.bin BootLoader.asm
	
# 소스 파일을 제외한 나머지 파일 정리	
clean:
	rm -f BootLoader.bin
