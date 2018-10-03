# file		makefile
# date		2009/06/06
# author	kkamagui 
# brief		시리얼로 데이터를 전송하는 프로그램을 빌드하는 make 파일

# 기본적으로 빌드를 수행할 목록
all: NetworkTransfer.exe

# NetworkTransfer 빌드
NetworkTransfer.exe: NetworkTransfer.c
	gcc -o $@ $<
	
# 소스 파일을 제외한 나머지 파일 정리	
clean:
	rm -f NetworkTransfer.exe
