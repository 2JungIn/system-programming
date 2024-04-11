# system-programming

### 프로세스
1. **[fork-exec](./프로세스/fork-exec-example.c)**

    fork-exec 모델을 간단히 구현한 프로그램입니다.
2. **[pipe](./프로세스/pipe-example.c)**

    쉘에서 파이프(` | `) 연산을 간단히 구현한 프로그램입니다.
3. **[multi pipe](./프로세스/multi-pipe-example.c)**

    멀티 파이프 연산을 간단히 구현한 프로그램 입니다.
4. **[redirection](./프로세스/redirection-example.c)**

    쉘에서 리다이렉션(`<`, `>`, `>>`) 연산을 간단히 구현한 프로그램 입니다.

5. **[daemon](./프로세스/daemon-example.c)**

    데몬 프로세스를 만드는 프로그램입니다.
    

### 스레드
1. **[thread pool](./스레드/thread-pool.c)**

    스레드풀 구현 예제 입니다.

### 메모리
1. **[file copy](./메모리/simple-copy.c)**

    파일을 복사하는 프로그램 입니다.

2. **[익명 매모리 매핑](./메모리/map-anonymous-exampl.c)**

    익명 메모리 매핑을 하는 프로그램 입니다. (매핑된 메모리는 0으로 초기화 되어있습니다.)


### 디렉토리
1. **[ls](./디렉토리/simple-ls.c)**

    `ls`명령어를 간단히 구현한 프로그램 입니다.

2. **[dir traversal](./디렉토리/dir-traversal.c)**

    디렉토리 순회를 구현한 프로그림 입니다.


### 네트워크
1. **[recv timeout](./네트워크/recv-timeout.c)**

    recv()의 동작에서 타임아웃 기능르 구현한 프로그램 입니다.

2. **[dnslookup](./네트워크/dnslookup.c)**

    도메인 주소의 IP주소를 출력하는 프로그램 입니다.

3. **[packet capture](./네트워크/packet-capture.c)**

    raw 소켓을 사용해서 패킷을 캡쳐하는 프로그램입니다.

4. **[ping](./네트워크/ping.c)**

    raw 소켓을 이용해서 핑을 구현한 프로그램 입니다.

5. **[traceroute](./네트워크/traceroute.c)**

    raw 소켓을 이용해서 목적지 주소로 이동하기 위한 경로들을 출력하는 프로그램 입니다.


### etc
1. **[현재 시간 얻기](./etc/get-current-time.c)**

    현재 시간을 출력하는 프로그램입니다.

2. **[특정 범위의 랜덤값 얻기](./etc/range-random.c)**

    특정 범위의 랜덤값을 반환하는 프로그램입니다.

3. **[시간 측정하기](./etc/time-measure.c)**

    어떤 작업의 수행 시간을 측정하는 프로그램 입니다.

4. **[프로세스의 자원 사용량 얻기](./etc/get-resource.c)**

    자원 사용량을 얻는 프로그램입니다. 

5. **[시스템 정보 얻기](./etc/get-system-info.c)**

    시스템 정보를 얻는 프로그램입니다.

6. **[런타임 구성정보 얻기](./etc/get-runtime-sysconf.c)**

    런타임 구성 정보를 얻는 프로그램 입니다.

7. **[파일이나 디렉토리의 구성 정보를 얻기](./etc/get-fileconf.c)**

    파일이나 디렉토리의 구성 정보를 얻는 프로그램 입니다.
