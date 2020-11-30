#include <stdio.h>
#include <unistd.h>

int PATH_MAX = 100;
char curr_command[100];

int main(int argc, char **argv) {

    char cwd[PATH_MAX];

    //get the current working directory and ask user for input
    if (getcwd(cwd, sizeof(cwd)) != NULL) { //SHOULD THIS BE A WHILE LOOP..?
        printf("%s $", cwd);
        scanf("%s", curr_command);
        printf("You entered %s\n", curr_command);

        //Tokenize the command user inputted

        //Execute the command(s)

        //1. fork a child process that will execute the command
        //2. in the child process, pass in the tokenized command
        //3. the child process will call execvp with the command
    } else {
        perror("getcwd() error");
        return 1;
    }

    return 1;
}