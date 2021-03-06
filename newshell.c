#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define BUF_SIZE 100

// Simplifed xv6 shell.

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;
  int dup_fd, fd;
  pid_t pid;

  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
    execvp(ecmd->argv[0], ecmd->argv);
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    dup_fd = dup(rcmd->fd);
    
    close(rcmd->fd);
    
    fd = open(rcmd->file, rcmd->mode,0777);/* Open will always return least unused file descriptor */ 
    if (fd < 0) {
      fprintf(stderr, "Error opening file\n");
      exit(1);
    }
    runcmd(rcmd->cmd);

    close(fd);   /* closing the file */
    dup2(dup_fd, rcmd->fd); /* Restoring stdout/stdin */
    close(dup_fd);
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    pipe(p);
    pid = fork();
    if (pid == 0) {
        dup2(p[1], 1);
        close(p[0]);
        runcmd(pcmd->left);
    } else {
       dup2(p[0], 0);
       close(p[1]);
       runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait(&r);
    break;
  }    
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  
  if (isatty(fileno(stdin)))
    fprintf(stdout, "238P$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

void multiCommandCheck(char* buf, char*nexbuf){
	int  t, it, lit, flag = 0;
	for(it =0; it < BUF_SIZE; it++){
		if(buf[it] == ';'){
			flag = 1;
			//	printf("Found ; in the input command\n");
			char localBuf[it];
			//localEndPoint = it;
			for(lit = 0; lit < it; lit++){
				localBuf[lit] = buf[lit];
				buf[lit] = ' ';
			}
			buf[it] = ' ';
			//	printf("command before ; is %s with size %d . size of is %ld and removing it is %s\n", localBuf, it, sizeof(localBuf), buf);
			//endPoint = localEndPoint;  
			for(t=0; t<100;t++){
				nexbuf[t] = buf[t];
				if(t < it)
					buf[t] = localBuf[t];
				else
					buf[t] = ' ';
			}
			memset(localBuf, 0, it);
		}
		if(it == BUF_SIZE -1 && flag == 0){
			memset(nexbuf, 0, BUF_SIZE);
			//	 printf("No more ; resetting nexbuf\n");
		}
	}
}

int check_for_background(char *buf)
{
    if (buf[strlen(buf) -2] == '&') {
        buf[strlen(buf) - 2] = 0;
        return 1;
    }
    return 0;
}

static void sig_child_handler(int sig)
{
    pid_t pid;
    int status;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

int
main(void)
{
  static char buf[100];
  static char nexbuf[100];
  // int fd;
  int r, z, pid, t;
  int background = 0;

  struct sigaction sig_ac;
  sigemptyset(&sig_ac.sa_mask);
  sig_ac.sa_flags = SA_RESTART;
  sig_ac.sa_handler = sig_child_handler;
  sigaction(SIGCHLD, &sig_ac, NULL); /* This is needed to handle zombie process as child process 
                                  send sigchild to parent on completion */

  z = getcmd(buf, sizeof(buf));

  // Read and run input commands.
  while(z >= 0){
    
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(nexbuf[0] == 0){
         multiCommandCheck(buf, nexbuf);
    } else {
	//	printf("In else as nexbuf is %s \n", nexbuf);
		for(t=0; t<100;t++)
			buf[t] = nexbuf[t];
		memset(nexbuf, 0, sizeof(nexbuf));
		multiCommandCheck(buf, nexbuf);
    }
	//	printf("Executing %s\n", buf);
    background = check_for_background(buf);
    if((pid = fork1()) == 0)
      runcmd(parsecmd(buf));
    if (!background)
        wait(&r);
    else
        fprintf(stdout, "PID %d\n", pid);
    if(nexbuf[0] == 0)
    z = getcmd(buf, sizeof(buf));
}
  exit(0);
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

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
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
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

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;

  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsepipe(ps, es);
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
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}
