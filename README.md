# 1. 프로젝트 소개 
MINT64 OS는 x86 호환 64bit 멀티코어 프로세서에서 동작하는 작은 운영체제(Operating System, OS)입니다. MINT64 OS는 멀티태스킹과 멀티스레딩, 간단한 파일 시스템과 한글 GUI시스템을 갖추고 있습니다.

이 저장소는 MINT64 OS를 제작하는 과정이 담긴 ["64비트 멀티코어 OS 구조와 원리" 도서](http://www.hanbit.co.kr/search/search_list.html?frm_keyword_str=64%EB%B9%84%ED%8A%B8+%EB%A9%80%ED%8B%B0%EC%BD%94%EC%96%B4+OS+%EC%9B%90%EB%A6%AC%EC%99%80+%EA%B5%AC%EC%A1%B0)의 예제 파일과 관련된 저장소입니다. 책의 최신 예제 소스코드, 크로스 컴파일러 제작, 실행 환경과 관련된 내용을 담고 있습니다.

64비트 멀티코어 OS 구조와 원리는 [한빛미디어](http://www.hanbit.co.kr/search/search_list.html?frm_keyword_str=64%EB%B9%84%ED%8A%B8+%EB%A9%80%ED%8B%B0%EC%BD%94%EC%96%B4+OS+%EC%9B%90%EB%A6%AC%EC%99%80+%EA%B5%AC%EC%A1%B0)나 온라인 및 오프라인 서점에서 구매하실 수 있습니다.

<a href='http://www.hanbit.co.kr/store/books/look.php?p_code=B3548683222' target='_blank'><image src='http://www.hanbit.co.kr/data/books/B3548683222_l.jpg' width='200' /></a>
<a href='http://www.hanbit.co.kr/store/books/look.php?p_code=B1588486644' target='_blank'><image src='http://www.hanbit.co.kr/data/books/B1588486644_l.jpg' width='200' /></a>

## 1.1. MINT64 OS 시연 영상
아래는 MINT64 OS 최종 버전의 시연 영상입니다. 터미널, GUI 응용프로그램, 한글 입력기, 간단한 게임의 동작 과정이 들어있습니다.

<a href='http://www.youtube.com/watch?feature=player_embedded&v=TmfPimwaM4Q' target='_blank'><img src='http://img.youtube.com/vi/TmfPimwaM4Q/0.jpg' width='425' height=344 /></a>

# 2. 최신 Cygwin 버전(2018/10/03)으로 크로스 컴파일러 생성 방법
책이 쓰여질 당시, PC와 노트북이 32bit 프로세서를 내장하고 있었습니다. 제가 사용하던 노트북 역시 32bit였습니다. 그래서 32bit 환경에서도 64bit OS를 만들고 실행할 수 있는 환경이 필요했는데, 여러 방법 중에 선택한 것이 GCC(GNU Compiler Collection) + QEMU(Quick Emulator) + Eclipse 조합이었습니다. GCC는 윈도우에서 동작하는 [Cygwin(https://www.cygwin.com/)](https://www.cygwin.com)을 사용했는데, 당시 32bit와 64bit 코드를 모두 생성할 수 있는 Multilib 지원이 미비해서 GCC 소스로 직접 크로스 컴파일러를 빌드했습니다. 

크로스 컴파일러를 빌드해본 분들은 아시겠지만, 소스를 이용해서 컴파일하는 과정이 쉽지 않고 시간도 많이 걸리는데다가 Cygwin 버전이 올라가면 빌드 과정도 달라져서 많은 분들이 고생을 하셨습니다. 그래서 MINT64 OS의 예제 저장소와 함께 크로스 컴파일러 생성 과정도 같이 기록해 놓습니다.

## 2.1. Cygwin 32bit 설치 및 크로스 컴파일 준비
Cygwin은 [https://www.cygwin.com/](https://www.cygwin.com/)에서 다운로드 할 수 있으며, 책의 크로스 컴파일 과정을 진행하기 위해 32bit 설치 파일인 [setup-x86.exe](https://www.cygwin.com/setup-x86.exe)를 다운로드 합니다. 다운로드 한 파일을 실행하면 설치 경로부터 다운로드 할 파일을 임시로 저장할 경로까지 다양한 내용을 물어보는데요, 실제 설치에 관련된 파일을 다운로드할 주소를 물어보는 "Choose Download Site" 항목까지는 "다음" 버튼을 눌러서 기본 옵션으로 지정하셔도 큰 문제는 없습니다.

"Choose Download Site"에서는 스크롤을 내리다보면 FreeBSD 한국 사용자 그룹에서 제공하는 "ftp://ftp.kr.freebsd.org/" 미러 사이트가 있는데요, 해당 사이트를 선택하고 "다음" 버튼을 누르면 됩니다. 그러면 Cygwin이 해당 사이트에서 필요한 파일을 다운로드 한 후 설치 가능한 프로그램 목록을 보여주는데요, 아래 목록을 선택하여 바이너리(Bin) 또는 소스(Src)를 선택한 후 "다음" 버튼을 누르면 됩니다.
  - Devel
    - binutils - 소스 및 바이너리 모두 설치
    - gcc-core - 소스 및 바이너리 모두 설치
    - bison - 바이너리만 설치
    - flex - 바이너리만 설치
    - libiconv - 바이너리만 설치
    - libtool - 바이너리만 설치
    - make - 바이너리만 설치
    - patchutils - 바이너리만 설치
    - cygport - 바이너리만 설치
    - nasm - 바이너리만 설치
  - Interpreters 
    - python2 - 바이너리만 설치
  - Libs
    - libgmp-devel - 바이너리만 설치
    - libmpfr-devel - 바이너리만 설치
    - libmpc-devel - 바이너리만 설치

위의 항목을 모두 설치를 마치면 바탕화면에 "Cygwin Terminal" 아이콘이 생성되며, 해당 아이콘을 이용해서 크로스 컴파일러 생성 단계를 진행하면 됩니다.

크로스 컴파일러 생성에 앞서, 크로스 컴파일러가 생성될 경로를 미리 시스템 환경 변수에 등록하여 실행파일 경로를 전부 입력하지 않아도 되도록 하겠습니다. 환경변수는 "제어판" -> "시스템" -> "고급 시스템 설정 보기" -> "환경 변수" 버튼을 눌러서 설정할 수 있습니다. "환경 변수" 창이 표시되면 아래에 "시스템 변수"를 스크롤하여 "Path"를 찾습니다. 그리고 "편집" 버튼을 눌러 가장 앞쪽에 'C:\Cygwin\bin;C:\Cygwin\usr\cross\bin;'를 붙여넣으면 됩니다.
<a href='https://github.com/kkamagui/mint64os-examples/blob/master/images/env_path.png' target='_blank'><image src='https://github.com/kkamagui/mint64os-examples/blob/master/images/env_path.png' width='400' /></a>

## 2.2. binutils 크로스 컴파일하기
바탕화면에 생성된 "Cygwin Terminal"을 실행하면 터미널이 실행됩니다. 아래와 같이 입력하여 bintutils 소스코드가 있는 경로로 이동한 후 소스코드의 압축을 해제합니다.

```bash
# 버전 정보는 설치된 환경에 따라 다른 수 있음
$ cd /usr/src/binutils-2.29-1.src

# 소스코드 압축 해제
$ cygport binutils.cygport prep
```

압축이 해제되면 아래와 같이 입력하여 크로스 컴파일을 진행한 후 binutils를 설치합니다.

```bash
# 압축 해제된 소스코드로 이동
$ cd binutils-2.29-1.i686/src/binutils-gdb

# 환경 변수 설정 및 빌드 환경 설정
$ export TARGET=x86_64-pc-linux
$ export PREFIX=/usr/cross
$ ./configure --target=$TARGET --prefix=$PREFIX --enable-64-bit-bfd --disable-shared --disable-nls --disable-unit-tests
$ make configure-host

# 빌드 및 설치
$ make LDFLAGS="-static"
$ make install
```

설치까지 완료된 후 아래 커맨드를 입력했을 때 x86-64가 보이면 정상적으로 빌드가 완료된 것입니다.

```bash
# x86-64 확인
$ /usr/cross/bin/x86_64-pc-linux-ld --help | grep "supported"
/usr/cross/bin/x86_64-pc-linux-ld: supported targets: elf64-x86-64 elf32-i386 elf32-iamcu elf32-x86-64 a.out-i386-linux pei-i386 pei-x86-64 elf64-l1om elf64-k1om elf64-little elf64-big elf32-little elf32-big plugin srec symbolsrec verilog tekhex binary ihex
/usr/cross/bin/x86_64-pc-linux-ld: supported emulations: elf_x86_64 elf32_x86_64 elf_i386 elf_iamcu i386linux elf_l1om elf_k1om
```

## 2.3. GCC 크로스 컴파일하기
아래와 같이 입력하여 GCC 소스코드가 있는 경로로 이동한 후 소스코드의 압축을 해제합니다.

```bash
# 버전 정보는 설치된 환경에 따라 다른 수 있음
$ cd /usr/src/gcc-7.3.0-3.src

# 소스코드 압축 해제
$ cygport gcc.cygport prep
```

압축이 해제되면 아래와 같이 입력하여 크로스 컴파일을 진행한 후 gcc를 설치합니다.

```bash
# 압축 해제된 소스코드로 이동
$ cd gcc-7.3.0-3.i686/src/gcc-7.3.0

# 환경 변수 설정 및 빌드 환경 설정
$ export TARGET=x86_64-pc-linux
$ export PREFIX=/usr/cross
$ export PATH=$PREFIX/bin:$PATH
$ ./configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers --disable-shared --enable-multilib
$ make configure-host

# 공유 라이브러리 이름 변경
$ cp /lib/gcc/i686-pc-cygwin/7.3.0/libgcc_s.dll.a /lib/gcc/i686-pc-cygwin/7.3.0/libgcc_s.a
$ cp /lib/libmpfr.dll.a /lib/libmpfr.a
$ cp /lib/libmpc.dll.a /lib/libmpc.a
$ cp /lib/libgmp.dll.a /lib/libgmp.a

# 빌드 및 설치
$ make all-gcc
$ make install-gcc
```

설치까지 완료된 후 아래 커맨드를 입력했을 때 m64/m32가 보이면 정상적으로 빌드가 완료된 것입니다.

```bash
# m64/m32 확인
$ /usr/cross/bin/x86_64-pc-linux-gcc -dumpspecs | grep -A1 multilib_options
*multilib_options:
m64/m32
```

# 3. QEMU 설치 및 예제 실행 방법
책이 쓰여질 당시 QEMU 버전은 0.10.4 였으나, 지금은 3.0.0 버전까지 출시되어있습니다. 안타깝게도 책의 예제들이 0.10.4 버전에 맞추어 시험되었기 때문에 원활한 진행을 위해서는 [QEMU 0.10.4](files/qemu-0.10.4.zip)를 사용하시거나, QEMU 사이트에서 공식적으로 제공하는 가장 근접한 버전인 [QEMU 0.15.92](https://qemu.weilnetz.de/w32/2011/qemu-20111119-w32.exe)를 사용하시는 것이 좋습니다.

QEMU 설치 후에는 ["64비트 멀티코어 OS 구조와 원리"](http://www.hanbit.co.kr/search/search_list.html?frm_keyword_str=64%EB%B9%84%ED%8A%B8+%EB%A9%80%ED%8B%B0%EC%BD%94%EC%96%B4+OS+%EC%9B%90%EB%A6%AC%EC%99%80+%EA%B5%AC%EC%A1%B0)의 각 장에서 설명한 과정에 맞추어 예제를 실행하면 됩니다.
