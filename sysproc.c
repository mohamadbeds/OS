#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

struct keyValue{
 char* var;
 char* val;
}
    
struct keyValue* variables[MAX_VARIABLES];


int setVariable(char* variable, char* value){
   int i;
   for(i=0;i<MAX_VARIABLES;i++)
       if(variables[i]!=0){
           if(strcmp(variables[i]->var,variable)==0)
               strcpy(variables[i]->val,value);
       }else {
           struct keyValue* pair=(sizeof(keyValue));
           strcpy(pair->var,variable);
           strcpy(pair->val,value);
       }
    return 1;
}

int getVariable(char* variable, char* value){
    int i;
     for(i=0;i<MAX_VARIABLES;i++)
       if(variables[i]!=0){
           if(strcmp(variables[i]->var,variable)==0)
               value=variables[i]->val;
    
    
}


int sys_setvar(void)
{
  char* var,val;

  argstr(0, &var);
  argstr(1, &val);
  return setVariable(var,val);
}

int sys_getvar(void)
{
  char* var,val;

  argstr(0, &var);
  argstr(1, &val);
  return getVariable(var,val);
}

int sys_yield(void)
{
  yield(); 
  return 0;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
