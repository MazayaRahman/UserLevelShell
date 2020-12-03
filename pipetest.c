// CPP program to illustrate dup2()  
#include<stdlib.h> 
#include<unistd.h> 
#include<stdio.h> 
#include<fcntl.h> 
#include <readline/readline.h>
#include <readline/history.h>


struct command
{
  const char **argv;
};


int
spawn_proc (int in, int out, struct command *cmd)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1); //comd output to pipe out
          close (out);
        }

      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}

int
fork_pipes (int n, struct command *cmd)
{
  int i;
  pid_t pid;
  int in, fd [2];

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);

      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc (in, fd [1], cmd + i);

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];
    }

  wait(NULL);
  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */  
  if (in != 0)
    dup2 (in, 0);

  /* Execute the last stage with the current process. */
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv); //LAST PIPE CMD
  //FOR LAST CMD, CHECK IF REDIR EXISTS (> OR >>), IF IT DOES, INSTEAD OF STDOUT, DIRECT TO FILE
}

int main() 
{ 

  char* line = malloc(100);
  line = readline("enter smth ");
  printf("u entered %s\n", line);
    //args for each cmd
  const char *ls[] = { "ls", "-l", 0 };
  const char *awk[] = { "wc", 0 };
  const char *sort[] = { "sort", 0 };
  const char *uniq[] = { "uniq", 0 };

//putting all args (cmds) in the command structre
struct command cmd[2];
  //struct command cmd [] = { {ls}, {awk}};
  cmd[0].argv = ls;
  cmd[1].argv = awk;

  struct command* commands = malloc(2 * sizeof(struct command));

  for(int i = 0; i < 2; i++){
    commands[i].argv = malloc(2*sizeof(char*)); //2 spots in args for each cmd

  }

  commands[0].argv[0] = "ls"; commands[0].argv[1] = 0;
  commands[1].argv[0] = "wc"; commands[1].argv[1] = 0;

  for (int i = 0; i < 2; ++i)
    printf ("args[%d] = %s args[%d] = %s\n", 0, commands[i].argv[0], 1, commands[i].argv[1]);

if(fork() == 0){
  fork_pipes (2, commands); 
}
else{
  wait(NULL);
  char* line = malloc(100);
  line = readline("enter smth ");
  printf("u entered %s\n", line);
}
  

}

