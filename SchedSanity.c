#include "types.h"
#include "stat.h"
#include "user.h"

int main(void)
{
    int pid1;
    if ((pid1 = fork()) == 0)
    {
        int i;
        for (i = 0; i < 10000; i++)
        {
            int x = 48132226;
            x = 5 * x;
            x = x / 5;
        }
        exit();
        return 1;
    }
    int pid2;
    if ((pid2 = fork()) == 0)
    {
        int i;
        for (i = 0; i < 100000000; i++)
        {
            int x = 48132226;
            x = 5 * x;
            //printf(2, "process 2-%d\n", x + i);
            x = x / 5;
        }
        exit();
        return 1;
    }
    int pid3;
    if ((pid3 = fork()) == 0)
    {
        int i;
        for (i = 0; i < 100; i++)
        {
            int x = 48132226;
            x = 5 * x;
            x = x / 5;
            printf(2, "process 3-%d\n", x + i);
        }
        exit();
        return 1;
    }
    int pid4;
    if ((pid4 = fork()) == 0)
    {
        int i;
        for (i = 0; i < 1000; i++)
        {
            int x = 48132226;
            x = 5 * x;
            x = x / 5;
            printf(2, "process 4-%d\n", x + i);
        }
        exit();
        return 1;
    }
    int waite1,waite2,waite3,waite4;
    int rtime1,rtime2,rtime3,rtime4;
    int iotime1,iotime2,iotime3,iotime4;
    wait2(pid1,&waite1,&rtime1,&iotime1);
    wait2(pid2,&waite2,&rtime2,&iotime2);
    wait2(pid3,&waite3,&rtime3,&iotime3);
    wait2(pid4,&waite4,&rtime4,&iotime4);
    printf(2,"pid=%d wtime=%d rtime=%d iotime=%d\n",pid1,waite1,rtime1,iotime1);
    printf(2,"pid=%d wtime=%d rtime=%d iotime=%d\n",pid2,waite2,rtime2,iotime2);
    printf(2,"pid=%d wtime=%d rtime=%d iotime=%d\n",pid3,waite3,rtime3,iotime3);
    printf(2,"pid=%d wtime=%d rtime=%d iotime=%d\n",pid4,waite4,rtime4,iotime4);
    exit();
    return 0;
}