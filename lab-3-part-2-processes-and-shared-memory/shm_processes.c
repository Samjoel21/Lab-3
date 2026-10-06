#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void ClientProcess(int *SharedMem);

int main()
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Create shared memory for two integers:
    // SharedMem[0] = BankAccount
    // SharedMem[1] = Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);

    if (ShmID < 0)
    {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    printf("Server has received a shared memory of two integers...\n");

    // Attach shared memory
    ShmPTR = (int *)shmat(ShmID, NULL, 0);

    if (ShmPTR == (int *)-1)
    {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }

    printf("Server has attached the shared memory...\n");

    // Initialize BankAccount and Turn
    ShmPTR[0] = 0;  // BankAccount
    ShmPTR[1] = 0;  // Turn

    printf("BankAccount = %d\n", ShmPTR[0]);
    printf("Turn = %d\n", ShmPTR[1]);

    printf("Server is about to fork a child process...\n");

    pid = fork();

    if (pid < 0)
    {
        printf("*** fork error (server) ***\n");
        exit(1);
    }

    // Child process
    else if (pid == 0)
    {
        ClientProcess(ShmPTR);
        exit(0);
    }

    // Parent process - Dear Old Dad
    else
    {
        int account;
        int balance;

        srand(time(NULL) ^ getpid());

        for (int i = 0; i < 25; i++)
        {
            // Sleep between 0 and 5 seconds
            sleep(rand() % 6);

            // Copy BankAccount into local variable
            account = ShmPTR[0];

            // Wait for Dad's turn
            while (ShmPTR[1] != 0)
            {
                // do nothing
            }

            // Dad decides whether to deposit
            if (account <= 100)
            {
                // Generate amount between 0 and 100
                balance = rand() % 101;

                // If even, deposit
                if (balance % 2 == 0)
                {
                    account += balance;

                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n",
                           balance, account);
                }
                else
                {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            }
            else
            {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n",
                       account);
            }

            // Copy local account back to shared memory
            ShmPTR[0] = account;

            // Give turn to Student
            ShmPTR[1] = 1;
        }

        // Wait for child
        wait(&status);

        printf("Server has detected the completion of its child...\n");

        // Detach shared memory
        shmdt((void *)ShmPTR);

        printf("Server has detached its shared memory...\n");

        // Remove shared memory
        shmctl(ShmID, IPC_RMID, NULL);

        printf("Server has removed its shared memory...\n");

        printf("Server exits...\n");
    }

    return 0;
}


// Child process - Poor Student
void ClientProcess(int *SharedMem)
{
    int account;
    int balance;

    srand(time(NULL) ^ getpid());

    printf("   Client process started\n");

    for (int i = 0; i < 25; i++)
    {
        // Sleep between 0 and 5 seconds
        sleep(rand() % 6);

        // Copy BankAccount into local variable
        account = SharedMem[0];

        // Wait for Student's turn
        while (SharedMem[1] != 1)
        {
            // do nothing
        }

        // Generate amount Student needs between 0 and 50
        balance = rand() % 51;

        printf("Poor Student needs $%d\n", balance);

        // Student has enough money
        if (balance <= account)
        {
            account -= balance;

            printf("Poor Student: Withdraws $%d / Balance = $%d\n",
                   balance, account);
        }
        else
        {
            printf("Poor Student: Not Enough Cash ($%d)\n",
                   account);
        }

        // Copy local account back to shared memory
        SharedMem[0] = account;

        // Give turn back to Dad
        SharedMem[1] = 0;
    }

    printf("   Client is about to exit\n");
}