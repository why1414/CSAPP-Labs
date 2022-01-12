# Shell Lab
## Introduction
The purpose of this assignment is to become more familiar with the concepts of process control and sig-nalling. You’ll do this by writing a simple Unix shell program that supports job control.

## The tsh Specification
tsh should support the following built-in command:

>The ```quit``` command terminates the shell.

>The ```jobs``` command lists all background jobs.

>The ```bg <job>``` command restarts <job> by sending it a SIGCONT signal, and then runs it in the background. The <job> argument can be either a PID or a JID.
 
>The ```fg <job>``` command restarts <job> by sending it a SIGCONT signal, and then runs it in the foreground. The <job> argument can be either a PID or a 

```tsh```的其余特征见文件 ```shlab.pdf```.

## 待补充函数
    • eval: Main routine that parses and interprets the command line.[70 lines]
    • builtin cmd: Recognizes and interprets the built-in commands: quit, fg, bg, and jobs. [25 lines]
    • do bgfg: Implements the bg and fg built-in commands. [50 lines]
    • waitfg: Waits for a foreground job to complete. [20 lines]
    • sigchld handler: Catches SIGCHILD signals. 80 lines]
    • sigint handler: Catches SIGINT (ctrl-c) signals. [15 lines]
    • sigtstp handler: Catches SIGTSTP (ctrl-z) signals. [15 lines]

## Note
### (1)
    
tsh.c 中虽然已经添加了头文件```<signal.h>```, 但在函数 ```Signal``` 中，调用 ```struct sigaction action ``` 仍然报错。
    
>solution：在 tsh.c 文件开头定义宏  ```#define _XOPEN_SOURCE 700```。

### (2)
在函数 eval 中，fork子进程前需要阻塞 SIGCHLD 信号( sigprocmask() )。fork后，子进程解除阻塞 SIGCHLD，因为子进程可能会需要处理该信号。此外，fork后，父进程在把子进程加入 jobs数组 后也需要解除该阻塞。这样做的目的是，防止子进程过早结束但父进程中还没将子进程加入 jobs数组 中，导致对 SIGCHLD 的handler函数中会 delete 一个jobs数组 中不存在的 job。父进程在addjob后，再解除阻塞，确保处理 SIGCHLD 信号时，子进程已经加入 jobs数组 中。

### (3)
子进程在前台模式执行，如果父进程在waitfg中使用函数waitpid的话，有可能会与信号处理函数 sigchld_handler 中的 waitpid 冲突(相当于对于一个子进程调用两次 waitpid 进行回收，可能会出错)。说明文档中建议的方法是只在 sigchld_handler 中回收子进程(waitpid), 而在waitfg中则采用 sleep 函数定期检查当前子进程是否在仍然在前台运行(利用jobs数组查看前台进程)，如果当前子进程不再是前台进程则等待结束。

```c
/*sleep 方式等待前台子进程 */
void waitfg(pid_t pid)
{
    /* block until the the child process is deleted by sigchld_handler */
    while(pid == fgpid(jobs))
        sleep(0);
    return;
}
```
使用 sleep 的循环等待，如果时间间隔太大，执行很慢；时间间隔太小，循环会太浪费。合适的解决方法是使用```int sigsuspend(const sigset_t *mask)```：sigsuspend函数暂时用mask替换当前的阻塞集合，然后挂起该进程，直到收到一个信号。(详细见 csapp 3th p545)

```c
/* sigsuspend 方式等待前台子进程 */
void waitfg(pid_t pid)
{
    /* a better way to block the foreground child process */
    sigset_t noBlkMask;
    sigemptyset(&noBlkMask); /* 阻塞集合为空 */
    while(pid == fgpid(jobs))
        sigsuspend(&noBlkMask);
    return;
}
```


### (4)
子进程中，用setpgid(0,0)将当前子进程独立形成一个进程组。因为 tsh 运行在标准 Unix shell 中的前台，tsh创建的子进程会默认加入tsh的进程组。而 ctrl-c 发送的 SIGINT 信号会发送给整个前台进程组，但tsh中的后台任务并不需要接收该 Ctrl-c 引发的信号。因此在fork之后，在execve之前，子进程需要用setpgid(0,0)为自己创建一个进程组。

### (5)
在说明文档shalab.pdf中，sigint_handler, sigstp_handler分别用于捕获 SIGINT和SIGTSTP 信号。他们负责将捕捉到的信号发送给目前的前台进程组( kill(-PID,sig) )。然后前台进程组接受到信号后自行处理，一般是采取默认处理方式：SIGINT(结束进程)，SIGTSTP(停止进程)。当子进程结束或停止时，系统会发送SIGCHLD信号给父进程，因此在 sigchld_handler 需要处理不同子进程状态: 

    (a)正常结束的子进程deletejob，
    (b)被其他信号结束的子进程打印信号信息并deletejob，
    (c)被停止的子进程更改子进程job的状态并打印信号信息。

在sigchld_handler中使用waitpid来等待子进程终止或停止：waitpid(-1, &status, WHNOHANG | WUNTRACED)。利用waitpid接受到pid后，利用status判断pid子进程的状态并进行相应的处理。

    参数-1表示等待由父进程创建的所有子进程，
    参数status，用来存储子进程的状态
    参数WHNOHANG | WUNTRACED 表示立即返回，如果等待集合中的子进程都没有被终止或停止，返回值为0；如果有一个停止或终止，则返回值为该子进程的PID。

### (6)
信号处理函数与主函数是并发，因此如何信号函数的安全性是需要考虑下的。csapp p536：
    G2 保存和恢复 errno
    G3 在处理程序和主程序中，访问共享全局数据时（读或写），需要阻塞所有的信号。因为处理程序和主程序是并发的，需要保护共享数据，类似于多线程中的保护。

本实例中只对处理函数加了阻塞全部信号，避免被其他信号中断;主程序中只在部分写全局数据处阻塞了全部信号。(为了安全性，应该读写都加，但实际测试样例trace 01-16都可以通过)


