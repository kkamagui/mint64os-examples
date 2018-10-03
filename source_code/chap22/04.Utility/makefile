# file		makefile
# date		2008/12/16
# author	kkamagui 
# brief		유틸리티를 생성하기 위한 make 파일

# 기본적으로 빌드를 수행할 목록
all: ImageMaker

# 이미지 메이커 빌드를 위해 ImageMaker 디렉터리에서 make 실행
ImageMaker:
	@echo 
	@echo ============= Build ImageMaker ===============
	@echo 
	
	make -C 00.ImageMaker

	@echo 
	@echo =============== Build Complete ===============
	@echo 
	
# 소스 파일을 제외한 나머지 파일 정리	
clean:
	make -C 00.ImageMaker clean
