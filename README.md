# system-programming

### 프로세스
1. **[fork-exec](./프로세스/fork-exec-example.c)**

    fork-exec 모델을 간단히 구현한 프로그램입니다.
2. **[파이프](./프로세스/pipe-example.c)**

    쉘에서 파이프(` | `) 연산을 간단히 구현한 프로그램입니다.
3. **[멀티 파이프](./프로세스/multi-pipe-example.c)**

    멀티 파이프 연산을 간단히 구현한 프로그램 입니다.
4. **[리다이렉션](./프로세스/redirection-example.c)**

    쉘에서 리다이렉션(`<`, `>`, `>>`) 연산을 간단히 구현한 프로그램 입니다.

5. **[데몬 프로세스](./프로세스/daemon-example.c)**

    데몬 프로세스를 만드는 프로그램입니다.
    

### 스레드
1. **[스레드 풀](./스레드/thread-pool.c)**

    스레드 풀 구현 예제 입니다.

2. **[스레드 방식을 사용한 타이머](./스레드/timer-use-thread.c)**

    스레드 방식을 사용한 타이머 예제 입니다.


### 시그널
1. **[시그널 방식을 사용한 타이머](./시그널/timer-use-signal.c)**

    시그널 방식을 사용한 타이머 예제 입니다.


### 메모리
1. **[파일 복사](./메모리/simple-copy.c)**

    파일을 복사하는 프로그램 입니다.

2. **[익명 매모리 매핑](./메모리/map-anonymous-exampl.c)**

    익명 메모리 매핑을 하는 프로그램 입니다. (매핑된 메모리는 0으로 초기화 되어있습니다.)


### 디렉토리
1. **[ls](./디렉토리/simple-ls.c)**

    `ls`명령어를 간단히 구현한 프로그램 입니다.

2. **[디렉토리 순회](./디렉토리/dir-traversal.c)**

    디렉토리 순회를 구현한 프로그림 입니다.


### 네트워크
1. **[바이트 순서](./네트워크/endian.c)**

    이 컴퓨터의 바이트 순서가 어떤 엔디안을 사용하는지 출력하는 프로그램 입니다.

    <details>
    <summary>실행 결과</summary>
    <pre>
    $ ./endian
    little endian!
    </pre>
    </deatils>

3. **[체크섬](./네트워크/internet-checksum.c)**

    rfc1071의 체크섬 함수를 구현한 프로그램 입니다.

4. **[recv timeout](./네트워크/recv-timeout.c)**

    recv()의 동작에서 타임아웃 기능을 구현한 프로그램 입니다.

5. **[dnslookup](./네트워크/dnslookup.c)**

    도메인 주소의 IP주소를 출력하는 프로그램 입니다.

    <details>
    <summary>실행 결과</summary>
    <p>case #1</p>
    <pre>
    $ ./dnslookup
    usage: ./dnslookup &lt;domain name&gt;
    </pre>
    <p>case #2</p>
    <pre>
    $ ./dnslookup google.com
    Name: google.com
    Address: 172.217.25.14
    Address: 2404:6800:4005:815::200e
    </pre>
    <p>case #3</p>
    <pre>
    $ ./dnslookup naver.com
    Name: naver.com
    Address: 223.130.200.236
    Address: 223.130.192.248
    Address: 223.130.200.219
    Address: 223.130.192.247
    </pre>
    </details>

6. **[패킷 캡처](./네트워크/packet-capture.c)**

    raw 소켓을 사용해서 패킷을 캡쳐하는 프로그램입니다.

7. **[ping](./네트워크/ping.c)**

    raw 소켓을 이용해서 핑을 구현한 프로그램 입니다.

    <details>
    <summary>실행 결과</summary>
    <p>case #1</p>
    <pre>
    $ sudo ./ping
    Usage: ./ping &lt;destination&gt;
    </pre>
    <p>case #2</p>
    <pre>
    $ sudo ./ping 8.8.8.8
    ING 8.8.8.8(8.8.8.8): 56 bytes data in ICMP packets.
    64 byte from 8.8.8.8: icmp_seq=1 ttl=113 rtt=48.943 ms
    64 byte from 8.8.8.8: icmp_seq=2 ttl=113 rtt=49.171 ms
    64 byte from 8.8.8.8: icmp_seq=3 ttl=113 rtt=49.208 ms
    ^C

    -----------8.8.8.8 PING statistics-----------
    3 packets transmitted, 3 received,  0%  loss, time 2090ms
    rtt min/avg/max = 48.943466 49.107475 49.207863
    </pre>
    <p>case #3</p>
    <pre>
    $ sudo ./ping naver.com
    PING naver.com(223.130.192.247): 56 bytes data in ICMP packets.
    ^C

    -----------naver.com PING statistics-----------
    3 packets transmitted, 0 received, 100%  loss, time 2246ms
    </pre>
    </details>

8. **[traceroute](./네트워크/traceroute.c)**

    raw 소켓을 이용해서 목적지 주소로 이동하기 위한 경로들을 출력하는 프로그램 입니다.

    <details>
    <summary>실행 결과</summary>
    <p>case #1</p>
    <pre>
    $ sudo ./traceroute
    Usage: ./traceroute &lt;destination&gt;
    </pre>
    <p>case #2</p>
    <pre>
    $ sudo ./traceroute google.com
    traceroute to google.com (142.250.66.110), 30 hops max, 28 byte packets
    hop: 1  recv ip: 192.168.1.1    1.061 ms
    hop: 2  recv ip: 10.62.96.1     9.240 ms
    hop: 3  recv ip: 10.20.14.9     8.801 ms
    hop: 4  recv ip: 61.43.176.237  8.582 ms
    timeout!
    timeout!
    hop: 7  recv ip: 1.208.149.1    42.063 ms
    hop: 8  recv ip: 1.208.106.18   45.247 ms
    hop: 9  recv ip: 61.42.0.26     43.987 ms
    hop: 10 recv ip: 142.250.168.244        47.233 ms
    hop: 11 recv ip: 72.14.233.125  47.906 ms
    hop: 12 recv ip: 66.249.95.129  52.076 ms
    hop: 13 recv ip: 142.250.66.110 44.054 ms
    </pre>
    <p>case #3</p>
    <pre>
    $ sudo ./traceroute naver.com
    traceroute to naver.com (223.130.200.236), 30 hops max, 28 byte packets
    hop: 1  recv ip: 192.168.1.1    0.869 ms
    hop: 2  recv ip: 10.62.96.1     7.911 ms
    hop: 3  recv ip: 10.20.14.1     9.747 ms
    hop: 4  recv ip: 61.43.176.233  10.473 ms
    timeout!
    hop: 6  recv ip: 1.208.167.206  9.798 ms
    hop: 7  recv ip: 182.162.152.122        10.301 ms
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    timeout!
    </pre>
    </details>


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

8. **[c언어 내장 매크로](./etc/built-in-macros.c)**

    c언어 내장 매크로들을 출력하는 예제 입니다.
    