#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int pipe1[2];
    int pipe2[2];

    int pid1;
    int pid2;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    // Make sure the user supplied an argument
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <grep argument>\n", argv[0]);
        return 1;
    }

    // Create first pipe
    if (pipe(pipe1) == -1)
    {
        perror("pipe");
        return 1;
    }

    // Create second pipe
    if (pipe(pipe2) == -1)
    {
        perror("pipe");
        return 1;
    }

    // ---------------- P1: cat scores ----------------

    pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid1 == 0)
    {
        // Send cat's output into pipe1
        dup2(pipe1[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        // Execute cat scores
        execvp("cat", cat_args);

        // Only reached if execvp fails
        perror("execvp cat");
        exit(1);
    }

    // ---------------- P2: grep argument ----------------

    pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid2 == 0)
    {
        // Get input from pipe1
        dup2(pipe1[0], STDIN_FILENO);

        // Send grep output into pipe2
        dup2(pipe2[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        // Execute grep
        execvp("grep", grep_args);

        // Only reached if execvp fails
        perror("execvp grep");
        exit(1);
    }

    // ---------------- P3: sort ----------------

    int pid3 = fork();

    if (pid3 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid3 == 0)
    {
        // Get input from pipe2
        dup2(pipe2[0], STDIN_FILENO);

        // Sort outputs to normal stdout

        // Close unused pipe ends
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        // Execute sort
        execvp("sort", sort_args);

        // Only reached if execvp fails
        perror("execvp sort");
        exit(1);
    }

    // Parent closes all pipe ends
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Wait for all children
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}