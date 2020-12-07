# UserLevelShell
## Implementation Description:

We have an infinite while loop in main, that prints the current directory every iteration, and waits for user input.
Upon receiving user input, we first tokenized the input by semicolons to separate each command.
For each command, we checked to see if it contained “>” or “>>”, and if it did, we separated the command to two parts, command to route, and the file to route to.
Then the command is tokeanized by “|” and each command is stored into a commands array. We used a struct called command to represent a command, it contained a string array of args.
The array of commands are passed to a helper function that is responsible for forking and piping the commands.
We set our pipes accordingly depending on the number of commands to pipe. And in the case where the command needed to be routed to a file, we created or appended to a file descriptor corresponding to the file inputted.
We added a signal handler to handle SIGINT, which checks to see if there are current processes running, and kills them. If no processes are running, then the signal is ignored.

## Testing:
We tested the shell program with various commands, and also with pipes and routing.

To test if our signal handler was working properly we created a file test.c in our directory. This test file had an infinite while loop. We compiled this test file in our user created shell using commands gcc test.c -o test -> ./test. In an infinite loop after entering ctrl + c our code returned to the next read line to wait for user input.


## Challenges:
Piping multiple commands was a bit difficult to figure out, and we faced some issues caused by forked processes.  There were some inconsistencies, and some command’s inputs and outputs were not being routed properly. But we were able to fork each piped command and route them accordingly by making sure pipes are closed properly where they need to be.

We faced some issues tokenizing the commands, and so decided to store them in a malloced array of command structs.
