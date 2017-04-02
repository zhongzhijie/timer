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
    	alarm(1);	/* �������ö�ʱ�� */
    	#else
    	printf("timer signal SIGALRM by setitimer()\r\n");
    	#endif
    }
}

int main()  
{   
    int i = 0;
    struct timeval timeVal;	/* selectʹ�õĲ����ṹ�� */
    int fd = -1;
    unsigned long data = 0;
    struct itimerval tick = {0};	/* setitimerʹ�� */
    
    /* sleep(��λs)ʵ�ֶ�ʱ */
    for (i = 0; i < 5; i++) 
    {   
        printf("timer by sleep(1) function\r\n");   
        sleep(1);
    }  
    
    /* usleep(��λus)ʵ�ֶ�ʱ */
    for (i = 0; i < 5; i++) 
    {   
        printf("timer by usleep(1000000) function\r\n");   
        usleep(100000);
    }  
    
    /* selectʵ�ֶ�ʱ������ */
    for (i = 0; i < 5; i++) 
    { 
        timeVal.tv_sec = 1;				/* ����ʱ��s */
        timeVal.tv_usec = 500000;	/* ��������ͷ����λ΢�� */
        
        select(0, NULL, NULL, NULL, &timeVal);
        printf("timer by select function\r\n");
    }
    
    /* ʹ��SIGALRM��alarm��ɶ�ʱ������ */
    signal(SIGALRM , sigTimer);	/* �����źŴ����� */
    #ifdef ALARM_TIMER
    alarm(1);	/* ��������SIGALRM�źţ����ö�ʱ����Ϊ1s */
    while(1)
    {
        if (5 == sigCount)
        	break;
    }
    #endif
    
    /* ʹ��RTCʵ�ֶ�ʱ�������Ƚϸ� */
    fd = open("/dev/rtc", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
    }
    else
    {
        ioctl(fd, RTC_IRQP_SET, 1);	/* ����Ƶ��1HZ */
        ioctl(fd, RTC_PIE_ON, 0);		/* ������ʱ���ж� */
        for (i = 0; i < 5; i++)
        {
            if(read(fd, &data, sizeof(unsigned long)) < 0)
            {
                perror("read");
                close(fd);		
            }
            printf("timer by RTC(real timer clock)\r\n");
        }
        ioctl(fd, RTC_PIE_OFF, 0);	/* �رն�ʱ���ж� */
        close(fd);
    }
    
    /*
    int setitimer(int which, const struct itimerval *value, struct itimerval *ovalue)); 
    which�������������źţ�
    ITIMER_REAL:��ϵͳ��ʵ��ʱ�������㣬���ͳ�SIGALRM�ź�
    ITIMER_VIRTUAL:�Ըý������û�̬�»��ѵ�ʱ�������㣬���ͳ�SIGVTALRM�ź�
    ITIMER_PROF:�Ըý������û�̬�º��ں�̬�����ѵ�ʱ�������㣬���ͳ�SIGPROF�ź�
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