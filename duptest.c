

// CPP program to illustrate dup2()  
#include<stdlib.h> 
#include<unistd.h> 
#include<stdio.h> 
#include<fcntl.h> 

 int status;
  int i;

  
int main() 
{ 
    char *cat_args[] = {"cat", "shell.c", NULL};
    char *grep_args[] = {"grep", "dup", NULL};
    char *cut_args[] = {"wc", NULL};

    int pipes[4];
    pipe(pipes); // sets up 1st pipe
    pipe(pipes + 2); // sets up 2nd pipe

    for(int k = 0; k < 3; k++){
        if (fork() == 0)
        {
            if(k==0){
                printf("first command\n");
                dup2(pipes[1], 1);

                // close all pipes (very important!); end we're using was safely copied

                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);

                execvp(*cat_args, cat_args);
            }
            else if (k == 2){
                printf("last command\n");
                dup2(pipes[2], 0);

                // close all ends of pipes

                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);

                execvp(*cut_args, cut_args);
            }
            else{
                printf("mid command\n");
                dup2(pipes[0], 0);

                // replace grep's stdout with write end of 2nd pipe

                dup2(pipes[3], 1);

                // close all ends of pipes

                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);

                execvp(*grep_args, grep_args);
            }
        }
        else{
            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
            close(pipes[3]);

            for (i = 0; i < 3; i++)
                wait(&status);
        }
    }

//     if (fork() == 0)
//     {
//       // replace cat's stdout with write part of 1st pipe

//       dup2(pipes[1], 1);

//       // close all pipes (very important!); end we're using was safely copied

//       close(pipes[0]);
//       close(pipes[1]);
//       close(pipes[2]);
//       close(pipes[3]);

//       execvp(*cat_args, cat_args);
//     }
//   else
//     {
//       // fork second child (to execute grep)

//       if (fork() == 0)
// 	{
// 	  // replace grep's stdin with read end of 1st pipe
	  
// 	  dup2(pipes[0], 0);

// 	  // replace grep's stdout with write end of 2nd pipe

// 	  dup2(pipes[3], 1);

// 	  // close all ends of pipes

// 	  close(pipes[0]);
// 	  close(pipes[1]);
// 	  close(pipes[2]);
// 	  close(pipes[3]);

// 	  execvp(*grep_args, grep_args);
// 	}
//       else
// 	{
// 	  // fork third child (to execute cut)

// 	  if (fork() == 0)
// 	    {
// 	      // replace cut's stdin with input read of 2nd pipe

// 	      dup2(pipes[2], 0);

// 	      // close all ends of pipes

// 	      close(pipes[0]);
// 	      close(pipes[1]);
// 	      close(pipes[2]);
// 	      close(pipes[3]);

// 	      execvp(*cut_args, cut_args);
// 	    }
// 	}
//     }
      
//   // only the parent gets here and waits for 3 children to finish
  
//   close(pipes[0]);
//   close(pipes[1]);
//   close(pipes[2]);
//   close(pipes[3]);

//   for (i = 0; i < 3; i++)
//     wait(&status);
      
return 0; 

} 