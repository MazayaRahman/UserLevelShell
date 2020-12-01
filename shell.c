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

int main(int argc, char **argv) {

    char cwd[PATH_MAX];
    char curr_token[100];

    int redir_exists = 0;

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


                char ** args  = NULL;
                char *  p    = strtok (curr_token, " ");
                int n_spaces = 0, i;


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

                //ARGS IS READY

                if(strcmp(args[0], "cd") == 0){
                    chdir(args[1]);
                }
                else{

                    //carry out the command
                    pid_t child_process_id = fork();
                    if(child_process_id == 0){
                        if(redir_exists){
                            
                            if(strcmp(target,">") == 0){
                                //remove file if it already exists
                                remove(routeTo);
                                //create a new file
                                FILE *fp;
                                fp  = fopen (routeTo, "w");
                                fclose (fp);
                                //initialize file desc with both write and append options
                                file_desc = open(routeTo, O_WRONLY|O_APPEND); 
                                printf("its write\n");
                            }
                            else{
                                
                                file_desc = open(routeTo, O_WRONLY|O_APPEND); 
                                printf("its append\n");
                            }
      
                            //save_stdin = dup(1);
                            // here the newfd is the file descriptor of stdout (i.e. 1) 
                            dup2(file_desc, 1); 
                            close(file_desc);

                            execvp(args[0], args);
                            //dup2(1, file_desc);


                        }
                        else{
                            execvp(args[0], args);
                        }
 
                    }
                    else{
                        //wait for child
                        wait(NULL);
                        redir_exists = 0;
                        //dup2(1, file_desc);
                        
                    }
                    

                }


            }
        }
        else{
            perror("getcwd() error");
            return 1;
        }
    }
}