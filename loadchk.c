/**
 * Load average checker for Linux.
 * (c) 2010, Nicholas Booker
 *
 * OUTPUT
 *
 * This program checks the load average for the system.
 * If it is < 1, then this program exits silently without producing any output.
 * If it is >= 1, then this program will:
 *  * print the current one-minute load average
 *  * print a list of most hungry processes at the moment
 *
 * The silence when less than one is a design feature, allowing this program
 * to run as a cron job and produce a report email only if the load is
 * worth reporting.
 *
 * EXIT STATUS
 * 
 * 0   -- Ran successfully (regardless of system load)
 * >0  -- We couldn't get the load average, or something else failed.
 *
 * Note at the moment we treat errors running top as inconveniences rather
 * than errors.  You'll be warned of the failure but the program will still
 * exit with status 0 if that's all that failed.
 *
 * This may change soon.
 *
 * OPTIONS
 *
 * the -t option overrides the default load average threshold of 1.0
 * e.g.:
 *   loadavg -t 2.2
 *
 * FUTURE CHANGES
 *
 * * Don't call 'top' -- leave that to calling scripts.
 * * -q option for quiet output.
 *
 * SUPER-FUTURE CHANGES
 * * allow comparisons (implying -q), e.g.
 *    chkload -g 1
 *    chkload -ge 1
 *    chkload -l 1 (same as -t 1 -q)
 *    chkload -le 1
 *   I wont include equal or not equal, because they don't make sense.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

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

void print_usage(char *argv0)
{
    fprintf(stderr, "USAGE: %s [-t threshold]\n", basename(argv0));
}

int main(int argc, char** argv)
{
    int num_samples;   /* Number of load average samples retrieved */
    double loadavg[3]; /* Space for up to 3 load average samples */
    char* fmt;
    double threshold;
    char flag;
    char *p;    /* For the call to strtod */

    threshold = 1.0;

    while ((flag = getopt(argc, argv, "t:")) != -1) {
        switch (flag) {
            case 't':
                threshold = strtod(optarg, &p); break;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                print_usage(argv[0]);
                abort();
        }
    }

    num_samples = getloadavg(loadavg, 3);

    if (num_samples < 0) {
        fprintf(stderr, "Load averages unobtainable.\n");
        return 2;
    }

    if (loadavg[0] > threshold) {
        printf("Current load average %0.02f exceeds %0.02f\n", loadavg[0], threshold);
        (void) show_top();
    }
    return 0;
}
