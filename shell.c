#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include<fcntl.h> 

int PATH_MAX = 100;
char* curr_command;

int n_spaces = 0;

char output[500]; 

struct command
{
  const char **argv;
};

int main(int argc, char **argv) {

    char cwd[PATH_MAX];
    char curr_token[100];
    char curr_cmd[100];

    int redir_exists = 0;
    int pipe_exists = 0;

    int file_desc; //stores the current file descriptor
    int save_stdin;
    
    while(1){
        if(getcwd(cwd, sizeof(cwd)) != NULL) { //SHOULD THIS BE A WHILE LOOP..?
            printf("%s $", cwd);
            curr_command = readline("");
            //scanf("%s", curr_command);
            printf("You entered %s\n", curr_command);

            if(strcmp(curr_command, "exit") == 0){
                break;
            }

            char* token = strtok(curr_command, ";");
            while(token){
                printf("token %s\n", token);

                memset(curr_token, '\0', sizeof(curr_token));
                strcpy(curr_token, token);

                token = strtok(NULL, ";");
                printf("curr token %s\n", curr_token);

                //SPLIT CURR TOKEN BY > OR >> IF THEY EXIST
                char* target = ">>";
                char* ptr = strstr(curr_token, target);
                if(!ptr){
                    target = ">";
                    ptr = strstr(curr_token, target);
                }

                char* cmdToRoute;
                char* routeTo;

                
                if(ptr){ //it has > or >>
                    char* token_a = strtok(curr_token, target);
                    cmdToRoute = token_a;
                    token_a = strtok(NULL, target);
                    routeTo = token_a;

                    printf("cmd to Route: %s\n", cmdToRoute);
                    printf("routeTo: %s\n", routeTo);

                    redir_exists = 1;
                }

                printf("curr token %s\n", curr_token);

                //SAVE CURR TOKEN HERE IF NEEDED
                memset(curr_cmd, '\0', sizeof(curr_cmd));
                strcpy(curr_cmd, curr_token);

                //TOKENIZE BY PIPE
                char ** pipe_tokens = NULL;
                char* pipe_ptr = strstr(curr_cmd, "|");
                int p_spaces = 0, i;
                
                printf("curr cmd %s\n", curr_cmd);

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

                    for (i = 0; i < (p_spaces); ++i)
                    printf ("pipe_tokens[%d] = %s\n", i, pipe_tokens[i]);
                }

                struct command cmds_exec [p_spaces];

                for (int k = 0; k <= (p_spaces); ++k){

                    //TOKENIZE EACH PIPE TOKEN BY SPACE
                    if(pipe_exists){
                        strcpy(curr_token, pipe_tokens[k]);
                    }
                    printf("i is %d curr token is %s\n", k, curr_token);

                    char ** args  = NULL;
                    char *  p    = strtok (curr_token, " ");
                    int n_spaces = 0;
                    

                    /* split string and append tokens to 'res' */

                    while (p) {
                        args = realloc (args, sizeof (char*) * ++n_spaces);

                        if (args == NULL)
                            exit (-1); /* memory allocation failed */

                        args[n_spaces-1] = p;

                        p = strtok (NULL, " ");
                    }

                    /* realloc one extra element for the last NULL */

                    args = realloc (args, sizeof (char*) * (n_spaces+1));
                    args[n_spaces] = 0;

                    /* print the result */


                    for (i = 0; i < (n_spaces+1); ++i)
                        printf ("args[%d] = %s\n", i, args[i]);

                    //ARGS IS READY, PUT INTO CMDS

                    cmds_exec[k].argv = args;

                    if(k == p_spaces-1) break;
                }

                fork_pipes(p_spaces, cmds_exec);




/////////////////////////////////////////////////////////////////////////
        //         int k = 0;

        //         int num_pipes = p_spaces-1;
        //         printf("num of pipes %d\n", num_pipes);

        //         //SETTING UP PIPES
        //         int pipes[num_pipes];
        //         for(int j = 0; j < num_pipes; j+=2){ //incrementing pipes by 2
        //             pipe(pipes + j);
        //         }


        //         for (k = 0; k <= (p_spaces); ++k){

        //             if(pipe_exists){
        //                 strcpy(curr_token, pipe_tokens[k]);
        //             }
        //             printf("i is %d curr token is %s\n", k, curr_token);

        //             char ** args  = NULL;
        //             char *  p    = strtok (curr_token, " ");
        //             int n_spaces = 0;
                    

        //             /* split string and append tokens to 'res' */

        //             while (p) {
        //                 args = realloc (args, sizeof (char*) * ++n_spaces);

        //                 if (args == NULL)
        //                     exit (-1); /* memory allocation failed */

        //                 args[n_spaces-1] = p;

        //                 p = strtok (NULL, " ");
        //             }

        //             /* realloc one extra element for the last NULL */

        //             args = realloc (args, sizeof (char*) * (n_spaces+1));
        //             args[n_spaces] = 0;

        //             /* print the result */


        //             for (i = 0; i < (n_spaces+1); ++i)
        //                 printf ("args[%d] = %s\n", i, args[i]);

        //             //ARGS IS READY

        //             if(strcmp(args[0], "cd") == 0){
        //                 chdir(args[1]);
        //             }
        //             else{

        //                 //carry out the command
        //                 pid_t child_process_id = fork();
        //                 if(child_process_id == 0){
        //                     if(pipe_exists){
        //                         if(k==0){ //FIRST PIPE TOKEN
        //                             //set write to pipe
        //                             printf("set pipe at 1 to stdin\n");
        //                             dup2(pipes[1], 1);

                                    

        //                             for(int j = 0; j < num_pipes; j++){
        //                                 close(pipes[j]);
        //                             }

        //                         }
        //                         else if(k == p_spaces-1){ //LAST PIPE TOKEN
        //                             //set read from pipe
        //                             printf("set pipe at %d to stdin\n", k-1);
        //                             dup2(pipes[k-1], 0); //setting input of cmd from pipe

                                    
        //                             for(int j = 0; j < num_pipes; j++){
        //                                 close(pipes[j]);
        //                             }
        //                         }
        //                         else{
        //                             //set read from pipe
        //                             //set write to pipe
        //                             printf("set pipe at %d to stdin\n", k-1);
        //                             printf("set pipe at %d to stdout\n", k+2);
        //                             dup2(pipes[k-1], 0); //setting input of cmd from pipe

        //                             dup2(pipes[k+2], 1); //setting stdout of cmd to pipe

                                    

        //                             for(int j = 0; j < num_pipes; j++){
        //                                 close(pipes[j]);
        //                             }
        //                         }
        //                     }

                            

        //                     if(redir_exists){
                                
        //                         if(strcmp(target,">") == 0){
        //                             //remove file if it already exists
        //                             remove(routeTo);
        //                             //create a new file
        //                             FILE *fp;
        //                             fp  = fopen (routeTo, "w");
        //                             fclose (fp);
        //                             //initialize file desc with both write and append options
        //                             file_desc = open(routeTo, O_WRONLY|O_APPEND); 
        //                             printf("its write\n");
        //                         }
        //                         else{
                                    
        //                             file_desc = open(routeTo, O_WRONLY|O_APPEND); 
        //                             printf("its append\n");
        //                         }
        
                                
        //                         // here the newfd is the file descriptor of stdout (i.e. 1) 
        //                         dup2(file_desc, 1); 
        //                         close(file_desc);

        //                         execvp(args[0], args);


        //                     }
        //                     else{
        //                         execvp(args[0], args);
                                
        //                         read(pipes[0], output, 500);
        //                         printf("output: %s\n", output);
        //                     }
    
        //                 }
        //                 else{
        //                     for(int j = 0; j < num_pipes; j++){
        //                         close(pipes[j]);
        //                     }

        //                     //wait for child
        //                     wait(NULL);
        //                     redir_exists = 0;
        //                     //dup2(1, file_desc);                            
        //                 }
                        
        //                 if(k == p_spaces-1) break;
        //                 printf("output: %s\n", output);
        //             }

        //         }

                                

        //         pipe_exists = 0;
        //         printf("command complete\n");
        //     }
        // }
        // else{
        //     perror("getcwd() error");
        //     return 1;
        // }
    }  
}
    }}

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

  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */  
  if (in != 0)
    dup2 (in, 0);

  /* Execute the last stage with the current process. */
  printf("last cmd %s\n", cmd[i].argv[0]);
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv); //LAST PIPE CMD
  //FOR LAST CMD, CHECK IF REDIR EXISTS (> OR >>), IF IT DOES, INSTEAD OF STDOUT, DIRECT TO FILE
}


