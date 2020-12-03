#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<fcntl.h> 


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

int main(){

    char* test_cmd = "ls|wc";
    printf("test cmd %s\n", test_cmd);

    char* curr_cmd = malloc(sizeof(test_cmd));
    strcpy(curr_cmd, test_cmd);

    char ** pipe_tokens = NULL;
    char* pipe_ptr = strstr(curr_cmd, "|");
    int p_spaces = 0, j;

    printf("curr cmd %s\n", curr_cmd);
    if(pipe_ptr){

        pipe_ptr = strtok (curr_cmd, "|");

        while(pipe_ptr){
            pipe_tokens = realloc (pipe_tokens, sizeof (char*) * ++p_spaces);

            if (pipe_tokens == NULL)
                exit (-1); /* memory allocation failed */

            pipe_tokens[p_spaces-1] = pipe_ptr;


            pipe_ptr = strtok(NULL, "|");
        }

        for (int f = 0; f < (p_spaces); ++f)
            printf ("pipe_tokens[%d] = %s\n", f, pipe_tokens[f]);

        struct command* cmds_exec = malloc(p_spaces * sizeof(struct command));

        for(j= 0; j < p_spaces; j++){

            char* curr_token = malloc(sizeof(pipe_tokens[j]));
            strcpy(curr_token, pipe_tokens[j]);

            printf("curr token %s\n", curr_token);

            cmds_exec[j].argv = NULL;
            char *  p  = strtok (curr_token, " ");
            int n_spaces = 0;
            int i = 0;

            while (p) {
                cmds_exec[j].argv = realloc (cmds_exec[j].argv, sizeof (char*) * ++n_spaces);

                if (cmds_exec[j].argv == NULL)
                    exit (-1); /* memory allocation failed */

                cmds_exec[j].argv[n_spaces-1] = p;

                p = strtok (NULL, " ");
            }

            cmds_exec[j].argv = realloc (cmds_exec[j].argv, sizeof (char*) * (n_spaces+1));
            cmds_exec[j].argv[n_spaces] = 0;

            for (i = 0; i < (n_spaces+1); ++i)
                printf ("args[%d] = %s\n", i, cmds_exec[j].argv[i]);

        }

        printf("first cmd %s\n", cmds_exec[0].argv[0]);
        printf("last cmd %s\n", cmds_exec[1].argv[0]);

                    
    }

    return 1;

}