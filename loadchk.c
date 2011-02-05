#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int num_cpus(void)
{
    return 1;
}

/* Show 20 lines of top output */
int show_top(void)
{
    pid_t top_child;
    int top_exec_result;
    int top_error;
    int head_exec_result;
    int head_error;

    top_child = fork();
    if (top_child == -1) {
        perror("Error forking to call top");
        return 3;
    }
    else if (top_child == 0) {
        /* We're in the child process */
        top_exec_result = execlp("bash", "bash", "-c", "top -b -n 1 | head -n20", (char*) NULL);
        if (top_exec_result == -1) {
            perror("Error in child exec()ing bash");
            return 3;
        }
    }
    else { /* We're still in the parent. */
        wait(&top_error);
        if (top_error) {
            fprintf(stderr, "bash returned non-zero exit status %d\n", top_error);
            return 0;
        }
    }
}

int main(int argc, char** argv)
{
    int num_samples;   /* Number of load average samples retrieved */
    double loadavg[3]; /* Space for up to 3 load average samples */
    char* fmt;

    num_samples = getloadavg(loadavg, 3);

    if (num_samples < 0) {
        fprintf(stderr, "Load averages unobtainable.\n");
        return 2;
    }

    if (loadavg[0] > (double) num_cpus()) {
        printf("Current load average %0.02f exceeds %d\n", loadavg[0], num_cpus());
        (void) show_top();
    }
}
