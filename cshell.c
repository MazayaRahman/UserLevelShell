#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<fcntl.h> 
#include <signal.h>

int PATH_MAX = 100;

int n_spaces = 0;
char* redir_target;
char* routeFile; //USED AD FILENAME BUFFER FOR >/>>
int redir_exists = 0;

pid_t pid;
int * status =0;

struct command* cmds_exec; //ARRAY OF COMMANDS TO BE POPULATED

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
        printf("should not run in first command\n");
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
        printf("setting output to fd\n");
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

    
  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */  
  if (in != 0){
    printf("in, 0\n");
    dup2 (in, 0);
  }
  
  /* Execute the last stage with the current process. */
  //FOR LAST CMD, CHECK IF REDIR EXISTS (> OR >>), IF IT DOES, INSTEAD OF STDOUT, DIRECT TO FILE
  if(redir_exists){
    int file_desc;
    if(strcmp(redir_target,">") == 0){
      //remove file if it already exists
      remove(routeFile);
      //create a new file
      FILE *fp;
      fp  = fopen (routeFile, "w");
      fclose (fp);
      //initialize file desc with both write and append options
      file_desc = open(routeFile, O_WRONLY|O_APPEND); 
      printf("its write\n");
    }
    else{
      file_desc = open(routeFile, O_WRONLY|O_APPEND); 
      printf("its append\n");
    }
        
                                
    // here the newfd is the file descriptor of stdout (i.e. 1) 
    dup2(file_desc, 1); 
    close(file_desc);
  }
  //CHECK IF ITS A CD COMMAND
  else if(strcmp(cmd [i].argv [0], "cd") == 0){
    chdir(cmd [i].argv [1]);
    return;
  }
 
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv); //LAST PIPE CMD
  
}

void sigintHandler(int sig_num){

  printf("RING! RING! Ctrl+C is here\n");
  kill(pid, SIGTERM);
  pid_t waitId = waitpid(pid, &status, WNOHANG);
  if(waitId ==0){ //waitId is 0 if no child has returned
    kill(pid, SIGKILL);
  }
  waitpid(pid, &status,0); //harvest the zombie process
  printf("Exiting the program\n");
}

int main(int argc, char **argv) {

    char cwd[PATH_MAX];
    char curr_token[100];
    char curr_cmd[100];

    char* curr_command = malloc(sizeof(char)* 200);
    routeFile = malloc(100);

    int pipe_exists = 0;

    int file_desc; //stores the current file descriptor
    int save_stdin;
    

    
    
    while(1){
      
        if(getcwd(cwd, sizeof(cwd)) != NULL) { //SHOULD THIS BE A WHILE LOOP..?
            printf("%s $", cwd);

            curr_command = readline("");
            if(curr_command == NULL) curr_command = readline("");

            printf("You entered %s\n", curr_command);

            if(strcmp(curr_command, "exit") == 0){
                break;
            }

            //TOKENIZE BY ;
            char* token = strtok(curr_command, ";");
            while(token){
                printf("token %s\n", token);

                memset(curr_token, '\0', sizeof(curr_token));
                strcpy(curr_token, token);

                token = strtok(NULL, ";");
                printf("curr token %s\n", curr_token);

                //SPLIT CURR TOKEN BY > OR >> IF THEY EXIST
                redir_target = ">>";
                char* ptr = strstr(curr_token, redir_target);
                if(!ptr){
                    redir_target = ">";
                    ptr = strstr(curr_token, redir_target);
                }

                char* cmdToRoute;
                char* routeTo;

                if(ptr){ //it has > or >>
                    char* token_a = strtok(curr_token, redir_target);
                    cmdToRoute = token_a;
                    token_a = strtok(NULL, redir_target);
                    routeTo = token_a;

                    //routeTo = trimwhitespace(routeTo);
                    trimwhitespace(routeFile, sizeof(routeTo)+1, routeTo);

                    printf("cmd to Route: %s\n", cmdToRoute);
                    printf("routeTo: %s\n", routeTo);
                    printf("routeFile: %s\n", routeFile);

                    redir_exists = 1;
                }

                printf("curr token %s\n", curr_token);

                //SAVE CURR TOKEN HERE IF NEEDED
                memset(curr_cmd, '\0', sizeof(curr_cmd));
                strcpy(curr_cmd, curr_token);

                printf("curr cmd %s\n", curr_cmd);

                //TOKENIZE BY PIPE
                char ** pipe_tokens = NULL;
                char* pipe_ptr = strstr(curr_cmd, "|");
                int p_spaces = 0, j;

                if(pipe_ptr){
                    pipe_exists = 1;

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

                    //SET UP COMMANDS BY MALLOCING ARRAY OF CMDS
                    cmds_exec = malloc(2 * sizeof(struct command));
                    //cmds_exec [p_spaces]; //# of commands = # of piped tokens
                }
                else{
                    //cmds_exec [1]; //handling 1 command
                    cmds_exec = malloc(1 * sizeof(struct command));
                }

                //TOKENIZES EACH CMD BY SPACE AND POPULATES GLOBAL CMD ARRAY
                populateCommands(curr_token, p_spaces, pipe_exists, pipe_tokens);

                
                //COMMANDS ARE POPULATED
                pid = fork();
		
		            signal(SIGINT, sigintHandler);

                if(pid == 0){
                  if(pipe_exists){
                    fork_pipes(p_spaces, cmds_exec);
                  }
                else{
                    fork_pipes(1, cmds_exec);
                  }
                }else{
                  wait(NULL);
                }

                //RESET STUFF BEFORE NEXT CMD          
                pipe_exists = 0;
                redir_exists = 0;
            }


        }
        else{
            perror("getcwd() error");
            return 1;
        }
    }

}

void populateCommands(char* curr_token, int p_spaces, int pipe_exists, char** pipe_tokens){
  for(int j= 0; j <= p_spaces; j++){
    if(pipe_exists){
      strcpy(curr_token, pipe_tokens[j]);
    }
    char* tok = malloc(sizeof(curr_token));
    strcpy(tok, curr_token);

    printf("j is %d tok is %s\n", j, tok);

     //TOKENIZE EACH CMD BY SPACE
    cmds_exec[j].argv = NULL;
    char *  p  = strtok (tok, " ");
    int n_spaces = 0;
    int i = 0;

    /* split string and append tokens to cmds array */
    while (p) {
      cmds_exec[j].argv = realloc (cmds_exec[j].argv, sizeof (char*) * ++n_spaces);

      if (cmds_exec[j].argv == NULL)
        exit (-1); /* memory allocation failed */

      cmds_exec[j].argv[n_spaces-1] = p;

      p = strtok (NULL, " ");
    }

    /* realloc one extra element for the last NULL */
    cmds_exec[j].argv = realloc (cmds_exec[j].argv, sizeof (char*) * (n_spaces+1));
    cmds_exec[j].argv[n_spaces] = 0;

    /* print the result */
    for (i = 0; i < (n_spaces+1); ++i)
      printf ("args[%d] = %s\n", i, cmds_exec[j].argv[i]);


    if(j == p_spaces-1) break;
  }

}

void trimwhitespace(char *out, size_t len, const char *str)
{
  if(len == 0)
    return 0;

  const char *end;
  size_t out_size;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
  {
    *out = 0;
    return 1;
  }

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end++;

  // Set output size to minimum of trimmed string length and buffer size minus 1
  out_size = (end - str) < len-1 ? (end - str) : len-1;

  // Copy trimmed string and add null terminator
  memcpy(out, str, out_size);
  out[out_size] = 0;

  return out_size;
}

