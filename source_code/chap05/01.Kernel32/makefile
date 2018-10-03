# file		makefile
# date		2008/11/12
# author	kkamagui 
# brief		가상 OS 이미지를 빌드하기 위한 make 파일

# 기본적으로 빌드를 수행할 목록
all: VirtualOS.bin

# 부트 로더의 기능을 테스트 하기 위한 가상의 OS 이미지 빌드
VirtualOS.bin: VirtualOS.asm
	nasm -o VirtualOS.bin VirtualOS.asm

# 소스 파일을 제외한 나머지 파일 정리	
clean:
	rm -f VirtualOS.bin
