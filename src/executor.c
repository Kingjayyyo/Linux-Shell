/**
Name: Jesuseyi Oyeyemi
*/

#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "command.h"
#include "executor.h"

#define PERMISSION_NUM 0664
#define MAX_NUM_LINE 1024
#define SUCCESSFUL 0

static int execute_aux(struct tree *t, int input_fd, int output_fd);

int execute(struct tree *t) {
   if(t != NULL) {
      /* call the auxiliary function */
      return execute_aux(t, STDIN_FILENO, STDOUT_FILENO);
   }
}

static int execute_aux(struct tree *t, int input_fd, int output_fd) {
   pid_t pid;
   int idx, idx2, idx3;

   /* Commands for a NONE Node */
   if(t->conjunction == NONE) {
      if(strcmp(t->argv[0], "cd") == 0) {
         /* If given no argumentts */
         if(t->argv[1] == NULL) {
            idx = chdir(getenv("HOME"));
            if(idx == -1) {
               perror("Cannot change directory");
               return EXIT_FAILURE;
            }
         }
         else {
            idx = chdir(t->argv[1]);
            if(idx == -1) {
               perror("Cannot change directory");
               return EXIT_FAILURE;
            }
         }
      }
      else if(strcmp(t->argv[0], "exit") == 0) {
         exit(0);
      }
      /* Code for other unix commands */
      else{
         if((pid = fork()) < 0) {
            perror("fork error");
            return EXIT_FAILURE;
         }
         if(pid) {
            wait(&idx);
            return idx;
         }
         else {
            if(t->input != NULL) {
               if((idx2 = open(t->input, O_RDONLY)) < 0) {
                  perror("File opening (read) failed");
                  return EXIT_FAILURE;
               }
               if(dup2(idx2, STDIN_FILENO) < 0) {
                  perror("dup2 (read) failed");
                  return EXIT_FAILURE;
               }
               if(close(idx2) < 0) {
                  perror("close error");
                  return EXIT_FAILURE;
               }
            }
            if(t->output != NULL) {
               if((idx3 = open(t->output, O_WRONLY | O_CREAT | O_TRUNC, PERMISSION_NUM)) < 0) {
                  perror("File opening (write) failed");
                  return EXIT_FAILURE;
               }
               if(dup2(idx3, STDOUT_FILENO) < 0) {
                  perror("dup2 (write) failed");
                  return EXIT_FAILURE;
               }
               if(close(idx3) < 0) {
                  perror("close error");
                  return EXIT_FAILURE;
               }
            }
            execvp(t->argv[0], t->argv);
            if (execvp(t->argv[0], t->argv) == -1) {
               fprintf(stderr, "Failed to execute %s\n", t->argv[0]);      
               exit(-1);
            }
         }
      }
   }
   /* Commands for AND Node */
   else if(t->conjunction == AND) {
      if(execute_aux(t->left, input_fd, output_fd) != SUCCESSFUL ||
       execute_aux(t->right, input_fd, output_fd) != SUCCESSFUL) {
         return EXIT_FAILURE;
      }
   }
   /* Commands for PIPE Node */
   else if(t->conjunction == PIPE) {
      int pipe_fd[2];
      if(t->left->output != NULL) {
         printf("Ambiguous output redirect.\n");
         return EXIT_FAILURE;
      }
      else if(t->right->input != NULL) {
         printf("Ambiguous input redirect.\n");
         return EXIT_FAILURE;
      }
      else {
         if(t->input != NULL) {
            if((idx2 = open(t->input, O_RDONLY)) < 0) {
               perror("Input file for PIPE couldn't be opened\n");
               return EXIT_FAILURE;
            }
         }
         if(t->output != NULL) {
            if((idx3 = open(t->output, O_WRONLY | O_CREAT | O_TRUNC, PERMISSION_NUM)) < 0) {
               perror("Output file for PIPE couldn't be opened\n");
               return EXIT_FAILURE;
            }
         }

         /* Creating pipe before fork */
         if(pipe(pipe_fd) < 0) {
            perror("pipe error");
            return EXIT_FAILURE;
         }
         if((pid = fork()) < 0) {
            perror("fork error");
            return EXIT_FAILURE;
         }
         if(pid) {
            if((close(pipe_fd[1])) < 0) {
               perror("Closing pipe_fd[1] error");
               return EXIT_FAILURE;
            }
            if(dup2(pipe_fd[0], STDIN_FILENO) < 0) {
               perror("dup2 error");
               return EXIT_FAILURE;
            }
            execute_aux(t->right, pipe_fd[0], output_fd);
            if((close(pipe_fd[0])) < 0) {
               perror("Closing pipe_fd[0] error");
               return EXIT_FAILURE;
            }
            wait(NULL);
         }
         else {
            if((close(pipe_fd[0])) < 0) {
               perror("Closing pipe_fd[0] error");
               return EXIT_FAILURE;
            }
            if(dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
               perror("dup2 error");
               return EXIT_FAILURE;
            }
            execute_aux(t->left, input_fd, pipe_fd[1]);
            if((close(pipe_fd[1])) < 0) {
               perror("Closing pipe_fd[1] error");
               return EXIT_FAILURE;
            }
            wait(NULL);
         }
      }
   }
   /* Commands for SUBSHELL Node */
   else if(t->conjunction == SUBSHELL) {
      if(t->input != NULL) {
         if((idx2 = open(t->input, O_RDONLY)) < 0) {
            perror("Input file for SUBSHELL couldn't be opened\n");
            return EXIT_FAILURE;
         }
      }
      if(t->output != NULL) {
         if((idx3 = open(t->output, O_WRONLY | O_CREAT | O_TRUNC, PERMISSION_NUM)) < 0) {
            perror("Output file for SUBSHELL couldn't be opened\n");
            return EXIT_FAILURE;
         }
      }
      if((pid = fork()) < 0) {
         perror("fork error");
         return EXIT_FAILURE;
      }
      if(pid) {
         wait(NULL);
      }
      else {
         execute_aux(t->left, input_fd, output_fd);
         exit(0);
      }
   }
   return 0;
}
