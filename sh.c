// Shell.

#include "types.h"
#include "user.h"
#include "fcntl.h"


// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5
#define ASSIGN 6

#define MAXARGS 10
#define MAX_HISTORY 16

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};
struct assigncmd {
  int type;
  char var[32];
  char val[128];
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;
  struct assigncmd *acmd;

  if(cmd == 0)
    exit();

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
    exec(ecmd->argv[0], ecmd->argv);
    printf(2, "exec %s failed\n", ecmd->argv[0]);
    break;
  case ASSIGN:
    acmd=(struct assigncmd*)cmd;
    setVariable(acmd->var,acmd->val);
    break;
  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      printf(2, "open %s failed\n", rcmd->file);
      exit();
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait();
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit();
}

int
getcmd(char *buf, int nbuf)
{
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

char * hist[MAX_HISTORY];
int shiftedAlready=0;
void shiftHistoryArray(char * newStr){
  if(hist[MAX_HISTORY-1]!=0)shiftedAlready=1;
  int i=0;
  for(i =MAX_HISTORY-1;i>0;i--){
    hist[i]=hist[i-1];
  }
  hist[0]=malloc(strlen(newStr));
  strcpy(hist[0],newStr);
}
void writeToHistoryFile(void){
  int fd;
  if((fd=open("history",O_RDWR|O_CREATE))<0)
  {
    exit();
  }
  int i=0;
  for(i=0;i<MAX_HISTORY;i++){
    write(fd,hist[i],strlen(hist[i]));
    write(fd,"\n",1);
  }
  close(fd);
}
int getline(char** line,int * len,int fd){
  char c=0;
  int index=0;
  char * l=malloc(128);
  while((c=read(fd,&c,1)) >0){
    if(c=='\n'){
      l[index]=0;
      *line=l;
      *len=index;
      break;
    }else {
      l[index]=c;
      index++;
    }
  }
return index;
}

char* replaceVars(char *buf){
    int i=0;int start=0;int terminate=1;int j=0;
    char newStr[128];
    memset(newStr,0,128);
    char var[32];
    memset(var,0,32);

    while(terminate){
    for(;i<strlen(buf);i++)
        if(buf[i]=='$'){
            break;
        }
        else newStr[j++]=buf[i];
       
    for(i=i+1;i<strlen(buf);i++)
        if(buf[i]=='$' || buf[i]==' ' || buf[i]==10)
            break;
        else
            var[start++]=buf[i];
        i=i-5;
        if(start>0){
        getVariable(var,newStr+j);
        printf(2,"%s,%s--\n",var,newStr+j);
        j=strlen(newStr);
        }
        if(i>=strlen(buf))
            terminate=0;
        start=0;
        memset(var,0,32);
    }
    //for(i=0;i<128;i++)buf[i]=newStr[i];
    printf(2,"*****%s\n",newStr);
    strcpy(buf,newStr);
    return buf;
}



void processcmd(char *buf){
  printf(2,"%s\n",buf);
           buf=replaceVars(buf);
           printf(2,"%s\n",buf);
     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        printf(2, "cannot cd %s\n", buf+3);
      return;
    }
    if(buf[0]=='h' && buf[1]=='i' && buf[2]=='s' && buf[3]=='t' && buf[4]=='o' && buf[5]=='r' && buf[6]=='y'){
        if(buf[7]==' ' && buf[8]=='-' && buf[9]=='l' && buf[10]==' '){
            int i=atoi(buf+11);
            int size=0;
            for(size=0;size<MAX_HISTORY;size++){
              if(hist[size]==0){
                break;
              }
            }
           // if(shiftedAlready)
              buf=hist[size-i+1];
            //else 
             // buf=hist[size-i];
            processcmd(buf);
            return;
    }
        int i=0,j=1;
        for(i=MAX_HISTORY-1;i>=0;i--){
            if(hist[i]!=0)
                printf(2,"%d. %s",j++,hist[i]);
            
        }
        return;
    }
    int pid=fork1();
    if(pid == 0)
      runcmd(parsecmd(buf));
    //int a=0;int b=0;int c=0;
    //wait2(pid,&a,&b,&c);
    //printf(2, "%d,%d\n", c,b);
    wait();   

}



int
main(void)
{
  static char buf[128];
  int fd;

    int fp;
    char * line = 0;
    int len = 0;
    int read;
    int counter=0;

    fp = open("history", O_RDWR|O_CREATE);
    if (fp == -1)
        exit();

    while (((read = getline(&line, &len, fp)) > 0) && counter<MAX_HISTORY && 0) {
      hist[counter]=line;
      counter++;
    }

    close(fp);
   // if (line)
        //free(line);

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }
  //int j;
  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){

    shiftHistoryArray(buf);

    processcmd(buf);
  //  for(j=0;j<100;j++)buf[j]=0;
  }
  writeToHistoryFile();
  exit();
}

void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
assigncmd(char *left,char *right)
{ 
  
  struct assigncmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ASSIGN;
  strcpy(cmd->var,left);
  strcpy(cmd->val,right);
  int i;
  for(i=0;i<strlen(cmd->val);i++)if(cmd->val[i]=='\n'){cmd->val[i]=0; break;}
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '=':
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{ 
  char *es;
  struct cmd *cmd;
  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    printf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

int isAssign(char *cmd){
  int i=0;
  int foundEq=0;
  for(i=0;i<strlen(cmd);i++){
    if(cmd[i]=='=')return i+1;
    if((!foundEq )&& !((cmd[i]>='a' && cmd[i] <='z') || (cmd[i]>='A' && cmd[i] <='Z')))
      return -1;
  }
  return -1;
}
void strncpyy(char *dest,char *src,int size){
  int i;
  for(i=0;i<size;i++){
    dest[i]=src[i];
    if(src[i]==0)break;
  }
  dest[size]=0;
}
struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  int x=isAssign(*ps);
  if(x!=-1){
    char temp[32];
    strncpyy(temp,*ps,x-1);
    cmd=assigncmd(temp,*ps+x);
    *ps=es;
    return cmd;
  }
  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;
  case ASSIGN:
    break;
  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
