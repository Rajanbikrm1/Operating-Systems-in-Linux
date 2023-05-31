#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/time.h>

int main(int argc, char *argv[], char *env[])
{

    pid_t cpid, gcpid;
    int status;
    struct tms tstart, cpu_start, cpu_end;
    struct timeval timeFisrt, startTime, endTime;

    gettimeofday(&timeFisrt, NULL); // record the start time before running any command or starting any processes
    times(&tstart);                 // record the cpu start time for final user and system time

    // loop through each command and run the commands
    for (int i = 1; i < argc; i++)
    {

        printf("Executing: %s\n", argv[i]); // Execute each commands

        char *command[] = {"/bin/bash", "-c", argv[i], NULL};

        /**
         * for the fork child process, i used the code from textbook
         * Reference: Textbook Chapter3: Processes
         * page:118
         */
        cpid = fork(); // child process
        if (cpid < 0)
        { // error occured
            fprintf(stderr, "Fork Failed");
            return 1;
        }
        else if (cpid == 0)
        { // repeat the same for grandchild process
            times(&cpu_start);
            gettimeofday(&startTime, NULL); // record the start time before starting any processes
            gcpid = fork();                 // grandchild process
            if (cpid < 0)
            { // error occured
                fprintf(stderr, "Fork Failed");
                return 1;
            }
            else if (gcpid == 0)
            {
                execve(command[0], command, env); // grandchild process executed the command
                perror("execve error");
                return 1;
            }
            else
            {

                wait(NULL); // child process waits for grandchild to finish

                gettimeofday(&endTime, NULL); // record the end time in the child processes
                double real_time = ((endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)/ 1000000.0);

                times(&cpu_end); // record the cpu end time in the child processes
                double user_time = ((cpu_end.tms_utime - cpu_start.tms_utime) + (cpu_end.tms_cutime - cpu_start.tms_cutime)) / (double)sysconf(_SC_CLK_TCK);
                double sys_time = ((cpu_end.tms_stime - cpu_start.tms_stime) + (cpu_end.tms_cstime - cpu_start.tms_cstime)) / (double)sysconf(_SC_CLK_TCK);

                // Displaying all the times for each command
                printf("Real: %.3fs\n", real_time);
                printf("Usr: %.3fs\n", user_time);
                printf("Sys: %.3fs\n", sys_time);
                printf("\n");

                return 0;
            }
        }
        else
        {               // parent process
            waitpid(gcpid, &status, 0); // wait for child to finish
        }
    }

    gettimeofday(&endTime, NULL); // record the end time after ending all the processes
    double total_real = ((endTime.tv_sec - timeFisrt.tv_sec) + (endTime.tv_usec - timeFisrt.tv_usec) / 1000000.0);

    times(&cpu_end); // record the cpu end time for final user and system time
    double total_user = ((cpu_end.tms_utime - tstart.tms_utime) + (cpu_end.tms_cutime - tstart.tms_cutime)) / (double)sysconf(_SC_CLK_TCK);
    double total_sys = ((cpu_end.tms_stime - tstart.tms_stime) + (cpu_end.tms_cstime - tstart.tms_cstime)) / (double)sysconf(_SC_CLK_TCK);

    // Displaying time information for each command
    printf("\n\nSummary Statistics: \n");
    printf("Real: %.3fs\n", total_real);
    printf("Usr: %.3fs\n", total_user);
    printf("Sys: %.3fs\n", total_sys);
    return 0;
}