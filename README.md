# 실행 방법

>> MacOS 기준으로 설명

1. 터미널에서 g++ --version을 통해 버전 확인
   1.1 본인은 Xcode CLI에서 제공하는 Apple clang version 17.0.0 사용
만약 다르다면 안정적인 사용을 위해 17 버전 이상의 g++ 설치

2. homebrew install qt@6.9 명령을 통해 qt를 다운로드
   
3. 터미널에서 qmake --version 과 g++ --version을 통해 잘 설치되었는지 확인
4. 현재 디렉터리로 이동 후 qmake one-card.pro 명령을 통해 Makefile 생성
5. make 명령어로 client의 실행 파일 생성
  5.1 client 실행 ./one-card.app/Contents/MacOS/one-card
6. cd server로 server 파일 이동 -> make로 실행 파일 생성
  6.1 server 실행 ./one-card-server


> sys/socket 를 사용한 코드이다보니 윈도우 OS에서는 컴파일이 되지 않음(윈속을 사용한 코드 없음)
