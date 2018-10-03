# file		makefile
# date		2010/03/31
# author	kkamagui 
# brief		패키지 파일을 생성하는 프로그램을 빌드하는 make 파일

# 기본적으로 빌드를 수행할 목록
all: PackageMaker.exe

# PackageMaker 빌드
PackageMaker.exe: PackageMaker.c
	gcc -o $@ $<
	
# 소스 파일을 제외한 나머지 파일 정리	
clean:
	rm -f PackageMaker.exe
