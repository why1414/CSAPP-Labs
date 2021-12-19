# proxy lab
proxy lab主要分为三个部分，
* part I: Implementing a sequential web proxy
* part II: Dealing with multiple concurrent request
* part III: Caching web objects

## part I Implementing a sequential web proxy
第一部分可以直接根据Tiny server中的代码构建一个proxy server
这一部分需要解析HTTP/1.0 GET requests并处理request headers  

    Host: www.cmu.edu
    User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3
    Connection: close
    Proxy-Connection: close

## part II Dealing with multiple concurrent requests
这一部分利用多线程实现并发。服务器监听到一个并获取一个连接socket，创建一个子进程去处理该socket。

> 报错：在执行测试```./driver.sh```中，并发测试一直timeout。

    *** Concurrency ***
    Starting tiny on port 30725
    Starting proxy on port 17901
    Starting the blocking NOP server on port 22894
    Timeout waiting for the server to grab the port reserved for it
    Terminated
> solution：经检查是```nop-server.py```中执行环境的问题。handout中```nop-server.py```的初始环境path```#!/usr/bin/python```，而我的wsl中环境path中是```/usr/bin/python3```。把```nop-server.py```首行的环境改为自己本地对应的python环境即可。




## References:
https://www.jianshu.com/p/a501d0c2f131