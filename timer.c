#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <signal.h>  
#include <sys/time.h>  
#include <sys/select.h>  
#include <time.h>
#include <linux/rtc.h>  
#include <sys/ioctl.h>
#include <fcntl.h> 

int sigCount = 0;

void sigTimer(int sig)
{
    if (SIGALRM == sig)
    {

    	sigCount++;
    	
    	#ifdef ALARM_TIMER
    	printf("timer signal SIGALRM by alarm()\r\n");
    	alarm(1);	/* 继续设置定时器 */
    	#else
    	printf("timer signal SIGALRM by setitimer()\r\n");
    	#endif
    }
}

int main()  
{   
    int i = 0;
    struct timeval timeVal;	/* select使用的参数结构体 */
    int fd = -1;
    unsigned long data = 0;
    struct itimerval tick = {0};	/* setitimer使用 */
    
    /* sleep(单位s)实现定时 */
    for (i = 0; i < 5; i++) 
    {   
        printf("timer by sleep(1) function\r\n");   
        sleep(1);
    }  
    
    /* usleep(单位us)实现定时 */
    for (i = 0; i < 5; i++) 
    {   
        printf("timer by usleep(1000000) function\r\n");   
        usleep(100000);
    }  
    
    /* select实现定时器功能 */
    for (i = 0; i < 5; i++) 
    { 
        timeVal.tv_sec = 1;				/* 设置时间s */
        timeVal.tv_usec = 500000;	/* 秒后面的零头，单位微秒 */
        
        select(0, NULL, NULL, NULL, &timeVal);
        printf("timer by select function\r\n");
    }
    
    /* 使用SIGALRM和alarm完成定时器功能 */
    signal(SIGALRM , sigTimer);	/* 设置信号处理函数 */
    #ifdef ALARM_TIMER
    alarm(1);	/* 触发产生SIGALRM信号，设置定时长度为1s */
    while(1)
    {
        if (5 == sigCount)
        	break;
    }
    #endif
    
    /* 使用RTC实现定时器，精度较高 */
    fd = open("/dev/rtc", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
    }
    else
    {
        ioctl(fd, RTC_IRQP_SET, 1);	/* 设置频率1HZ */
        ioctl(fd, RTC_PIE_ON, 0);		/* 开启定时器中断 */
        for (i = 0; i < 5; i++)
        {
            if(read(fd, &data, sizeof(unsigned long)) < 0)
            {
                perror("read");
                close(fd);		
            }
            printf("timer by RTC(real timer clock)\r\n");
        }
        ioctl(fd, RTC_PIE_OFF, 0);	/* 关闭定时器中断 */
        close(fd);
    }
    
    /*
    int setitimer(int which, const struct itimerval *value, struct itimerval *ovalue)); 
    which决定产生哪种信号：
    ITIMER_REAL:以系统真实的时间来计算，它送出SIGALRM信号
    ITIMER_VIRTUAL:以该进程在用户态下花费的时间来计算，它送出SIGVTALRM信号
    ITIMER_PROF:以该进程在用户态下和内核态下所费的时间来计算，它送出SIGPROF信号
    */
    //Timeout to run first time
    tick.it_value.tv_sec = 1;
    tick.it_value.tv_usec = 0;

    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 1;
    tick.it_interval.tv_usec = 0;
    
    #ifndef ALARM_TIMER
    setitimer(ITIMER_REAL, &tick, NULL);
    while(1)
    {
        if (5 == sigCount)
        	break;
    }
    #endif
    
    exit(EXIT_SUCCESS);  
}  