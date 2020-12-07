#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <stdlib.h>
#include<ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<fcntl.h> 
#include <signal.h>

void adding_Commands(char *, int, int, char **);
void remove_space(char *, size_t , const char *);
int PATH_MAX = 100;

int n_spaces = 0;
char* redir_target;
char* routeFile; //USED AD FILENAME BUFFER FOR >/>>
int redir_exists = 0;

pid_t pid;
int * status =0;
int processToKill = 0;

struct command* cmds_exec; //ARRAY OF COMMANDS TO BE POPULATED

struct command
{
  char **argv;
};

int
spawnProcess (int pipe_in, int pipe_out, struct command *cmd)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (pipe_in != 0)
        {
          dup2 (pipe_in, 0);
          close (pipe_in);
        }

      if (pipe_out != 1)
        {
          dup2 (pipe_out, 1); //comd output to pipe out
          close (pipe_out);
        }

      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}

int
fork_function (int n, struct command *cmd)
{
  int i;
  pid_t pid;
  int pipe_in, fd [2];

  pipe_in = 0; //first process getting the input

  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);

      spawnProcess (pipe_in, fd [1], cmd + i);

      close (fd [1]);
      
      pipe_in = fd [0]; //next child will read from here.
      
    }

    
 
  if (pipe_in != 0){ //stdin the read end of the previous file and output it to the original file descriptor 1
    dup2 (pipe_in, 0);
  }
  
  
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
    }
    else{
      file_desc = open(routeFile, O_WRONLY|O_APPEND); 
    }
        
                                
    // here the newfd is the file descriptor of stdout (i.e. 1) 
    dup2(file_desc, 1); 
    close(file_desc);
  }
 
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv); //LAST PIPE CMD
  
}

void sigintHandler(int sig_num){
  if(processToKill){
    kill(pid, SIGTERM);
    pid_t waitId = waitpid(pid, status, WNOHANG);
    if(waitId ==0){ //waitId is 0 if no child has returned
      kill(pid, SIGKILL);
    }
    waitpid(pid, status,0); //harvest the zombie process
  
  }
  
}

int main(int argc, char **argv) {

    char cwd[PATH_MAX];
    char curr_token[100];
    char curr_cmd[100];

    char* curr_command = malloc(sizeof(char)* 500);
    routeFile = malloc(100);


    int pipe_exists = 0;

    int file_desc; //stores the current file descriptor
    int save_stdin;

    signal(SIGINT, sigintHandler);
    

    
    
    while(1){
      
        if(getcwd(cwd, sizeof(cwd)) != NULL) { //SHOULD THIS BE A WHILE LOOP..?
            printf("%s$ ", cwd);

            curr_command = readline("");
            if(curr_command == NULL) curr_command = readline("");

           

            if(strcmp(curr_command, "exit") == 0){
                break;
            }

            //TOKENIZE BY ;
            char* token = strtok(curr_command, ";");
            while(token){

                memset(curr_token, '\0', sizeof(curr_token));
                strcpy(curr_token, token);

                char* exit_token = malloc(sizeof(curr_token));
                remove_space(exit_token, sizeof(curr_token)+2, curr_token);
                if(strcmp(exit_token, "exit") == 0){
                    free(exit_token);
                    return 1;
                }
                free(exit_token);

                token = strtok(NULL, ";");

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
                    remove_space(routeFile, sizeof(routeTo)+2, routeTo);


                    redir_exists = 1;
                }


                //SAVE CURR TOKEN HERE IF NEEDED
                memset(curr_cmd, '\0', sizeof(curr_cmd));
                strcpy(curr_cmd, curr_token);


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

                    //SET UP COMMANDS BY MALLOCING ARRAY OF CMDS
                    cmds_exec = malloc(p_spaces * sizeof(struct command));
                    //cmds_exec [p_spaces]; //# of commands = # of piped tokens
                }
                else{
                    //cmds_exec [1]; //handling 1 command
                    cmds_exec = malloc(1 * sizeof(struct command));
                }

                //TOKENIZES EACH CMD BY SPACE AND POPULATES GLOBAL CMD ARRAY
                adding_Commands(curr_token, p_spaces, pipe_exists, pipe_tokens);

                if(strcmp(cmds_exec[0].argv[0], "cd") == 0){
                  chdir(cmds_exec[0].argv [1]);
                  //RESET STUFF BEFORE NEXT CMD          
                  pipe_exists = 0;
                  redir_exists = 0;
                  processToKill = 0;

                  for (int f = 0; f < (p_spaces); ++f)
                      free(cmds_exec[f].argv);
                  free(cmds_exec);
                  continue;
                }
                //COMMANDS ARE POPULATED
                pid = fork();
		
                if(pid == 0){
                  if(pipe_exists){
                    fork_function(p_spaces, cmds_exec);
                  }
                else{
                    fork_function(1, cmds_exec);
                  }
                }else{
                  processToKill = 1; //SIGINT HANDLER ACTIVATED
                  wait(NULL);
                }

                //RESET STUFF BEFORE NEXT CMD          
                pipe_exists = 0;
                redir_exists = 0;
                processToKill = 0;

                for (int f = 0; f < (p_spaces); ++f)
                    free(cmds_exec[f].argv);
                free(cmds_exec);
            }


        }
        else{
            perror("getcwd() error");
            return 1;
        }
    }

    //free memory
    free(curr_command);
    free(routeFile);

}

void adding_Commands(char* curr_token, int p_spaces, int pipe_exists, char** pipe_tokens){
  for(int j= 0; j <= p_spaces; j++){
    if(pipe_exists){
      strcpy(curr_token, pipe_tokens[j]);
    }
    char* tok = malloc(sizeof(curr_token));
    strcpy(tok, curr_token);

     //TOKENIZE EACH CMD BY SPACE
    cmds_exec[j].argv = NULL;
    char *  p  = strtok (tok, " ");
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

    if(j == p_spaces-1) {
      //free(tok);
      break;
    }
  }
}

void remove_space(char *out, size_t len, const char *str)
{
  if(len == 0)
    return;

  const char *end;
  size_t output;

  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  
  {
    *out = 0;
    return;
  }

  
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end++;

  output = (end - str) < len-1 ? (end - str) : len-1;

  memcpy(out, str, output); 
  out[output] = 0;

  return;
}

