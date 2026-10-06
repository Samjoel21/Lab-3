#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd1[2];
    int fd2[2];

    char fixed_str[] = "howard.edu";
    char final_str[] = "gobison.org";
    char input_str[100];

    pid_t p;

    // Create first pipe
    if (pipe(fd1) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    // Create second pipe
    if (pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    // P1 gets the first input
    printf("Enter a string to concatenate:");
    scanf("%s", input_str);

    p = fork();

    if (p < 0)
    {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // ---------------- P1 / Parent ----------------
    else if (p > 0)
    {
        char result[100];

        // P1 writes to fd1
        close(fd1[0]);

        // P1 reads from fd2
        close(fd2[1]);

        // Send original string to P2
        write(fd1[1], input_str, strlen(input_str) + 1);

        close(fd1[1]);

        // Wait for P2 to send the string back
        read(fd2[0], result, 100);

        close(fd2[0]);

        // Add gobison.org
        strcat(result, final_str);

        printf("Output : %s\n", result);

        wait(NULL);
    }

    // ---------------- P2 / Child ----------------
    else
    {
        char concat_str[100];
        char second_input[100];

        // P2 reads from fd1
        close(fd1[1]);

        // P2 writes to fd2
        close(fd2[0]);

        // Receive string from P1
        read(fd1[0], concat_str, 100);

        close(fd1[0]);

        // Add howard.edu
        strcat(concat_str, fixed_str);

        printf("Other string is: %s\n", fixed_str);
        printf("Input : %s\n", concat_str);

        printf("Output : %s\n", concat_str);

        // Ask for second input
        printf("Input : ");
        scanf("%s", second_input);

        // Append second input
        strcat(concat_str, second_input);

        // Send result back to P1
        write(fd2[1], concat_str, strlen(concat_str) + 1);

        close(fd2[1]);

        exit(0);
    }

    return 0;
}