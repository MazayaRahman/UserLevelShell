#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int PATH_MAX = 100;
char* curr_command;

int main(int argc, char **argv) {

    char cwd[PATH_MAX];

    //get the current working directory and ask user for input
    // char* cmd = readline("");
    // char* t = strtok(cmd, ";");
    // while(t){
    //     printf("%s\n", t);
    //     t = strtok(NULL, ";");
    // }
    

   
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
                printf("Current command: %s\n", token);
                char whole_cmd[100];
                //char* whole_cmd = token;
                memset(whole_cmd, '\0', sizeof(whole_cmd));
                strcpy(whole_cmd, token);

                char* cmd = token;
                token = strtok(NULL, ";");
                printf("Whole command: %s\n", whole_cmd);

                //store >> or > ptrs if any
                char* target = ">";
                char* ptr = strstr(whole_cmd, target);
                if(ptr){ //it has >
                    char* token_a = strtok(whole_cmd,">");
                }


                char* argv[3];
                argv[1] = NULL;
                argv[2] = NULL;
                //argv[0] = token;

                //split by space
                char* token_s = strtok(cmd, " ");
                int i = 0;
                while(token_s){
                    //printf("%s ", token_s);
                    argv[i] = token_s;
                    token_s = strtok(NULL, " ");
                    i++;
                }
                cmd = argv[0];

                for(int k = 0; k < 3; k++){
                    printf("%s ", argv[k]);
                }
                printf("Ready to execute command\n");
                printf("Whole command: %s\n", whole_cmd);

                if(strcmp(argv[0], "cd") == 0){
                    chdir(argv[1]);
                }
                else{
                     //carry out the command
                    pid_t child_process_id = fork();
                    if(child_process_id == 0){

                        //check for > or >> (then use dup2 to redirect output)
                        printf("tokens is %s\n", whole_cmd);
                        char* target = ">";
                        char* ptr = strstr(whole_cmd, target);
                        if(ptr){
                            printf("found it %s\n", ptr);
                        }
                        
                        execvp(cmd, argv);

                        //set ouput back to stdout
                    }
                    else{
                        //wait for child
                        wait(NULL);
                    }
                }
                
                
            }



            //Tokenize the command user inputted
            // split by ; -> array of commands



            //Execute the command(s)

            //1. fork a child process that will execute the command
            //2. in the child process, pass in the tokenized command
            //3. the child process will call execvp with the command
            //4. wait on the child, then continue to next command
        } else {
            perror("getcwd() error");
            return 1;
        }
    }
    


    return 1;
}