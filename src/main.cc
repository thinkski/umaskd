/// @file
/// @author Chris Hiszpanski <chiszp@gmail.com>
///
/// @section DESCRIPTION
/// Main loop and user interface.

#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// POSIX-compliant headers
#include <sys/types.h>
#include <sys/stat.h>

// Autoconf generated header
#include "config.h"

// Local headers
#ifdef HAVE_SYS_INOTIFY_H
#include "inotify.hh"
#endif


///
/// Prints usage statement to standard output and exits successfully.
///
static void help(void)
{
    printf("Utility for maintaining separate per-directory umasks\n");
    printf("\n");
    printf("usage: umaskd [options]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -d, --daemon         Run as a daemon.\n");
    printf("  -f, --file=filename  Load configuration from specific file.\n");
    printf("  -h, --help           Prints this message.\n");
    printf("  -v, --version        Prints version information.\n");
    printf("\n");
    printf("Report bugs to: Chris Hiszpanski <chiszp@gmail.com>\n");
    exit(0);
}

///
/// Prints version to standard output and exits successfully.
///
static void version(void)
{
    printf("umaskd 1.0\n");
    printf("Copyright (C) 2012 Chris Hiszpanski\n");
    printf("License GPLv3+: GNU GPL version 3 or later ");
        printf("<http://gnu.org/licenses/gpl.html>\n");
    printf("This is free software: you are free to change and ");
        printf("redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
    exit(0);
}

///
/// Main loop
///
int main(int argc, char **argv)
{
    // Set defaults
    const char *cfgfile = SYSCONFDIR "/umaskd.conf";
    int daemon = 0;

    // Parse command-line arguments
    while (1) {
        int c;
        int option_index = 0;

        // Structure with long arguments. Must match help() message.
        static struct option long_options[] = {
            {"daemon",  0, 0, 'd'},
            {"file",    1, 0, 'f'},
            {"help",    0, 0, 'h'},
            {"version", 0, 0, 'v'},
            {NULL,      0, 0,  0 }
        };

        c = getopt_long(argc, argv, "df:hv", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'd':
                daemon = 1;
                break;
            case 'f':
                cfgfile = strdup(optarg);
                break;
            case 'h':
                help();
                break;
            case 'v':
                version();
                break;
            case '?':
            default:
                break;
        }
    }

    // Instantiate notification subsystem abstraction
    //Notify *notify = new Notify();
    Notify notify;

    /* Parse configuration file.
     * Format is one directory per line. */
    FILE *fp = fopen(cfgfile, "r");
    char *path = NULL;
    unsigned int perm = 0;
    /* Open configuration file for reading */
    if (!fp) {
        fprintf(stderr, "error: cannot open configuration file. exiting.\n");
        exit(1);
    }
    /* Allocate memory for path string */
    path = (char *)malloc(PATH_MAX);
    if (!path) {
        fprintf(stderr, "error: cannot allocate memory. exiting.\n");
        exit(1);
    }
    /* Construct safe format string for fscanf. Auto-allocates. */
    char *fmt = NULL;
    if (asprintf(&fmt, "%%4o%%%us", PATH_MAX) < 0) {
        perror("asprintf");
        exit(1);
    }
    /* Read each line of configuration file */
    ssize_t nread;
    size_t n = 0;
    char *line = NULL;
    while ((nread = getline(&line, &n, fp)) != -1) {

        /* Replace comment '#' with a null terminator */
        char *tmp = strchr(line, '#');
        if (tmp) {
            *tmp = '\0';
        }

        /* Read line, skipping it if not in correct format */
        if (sscanf(line, fmt, &perm, path) != 2) {
            free(line);
            line = NULL;
            continue;
        }
        free(line);
        line = NULL;

        // Add path to notification queue
        notify.add_path(path, perm);
    }
    /* Free memory, close file */
    free(fmt);
    free(path);
    fclose(fp);

    /* Run as daemon, if directed to do so */
    if (daemon) {
        pid_t session_id = 0;
        pid_t process_id = 0;

        /* Fork a child process */
        process_id = fork();
        if (process_id < 0) {
            fprintf(stderr, "error: cannot fork daemon process. exiting.\n");
            exit(1);
        }

        /* Kill the parent process */
        if (process_id > 0) {
            exit(0);
        }

        /* Unmask the file mode */
        umask(0);

        /* Set new session */
        session_id = setsid();
        if (session_id < 0) {
            exit(1);
        }

        /* Change current working directory to root */
        chdir("/");

        /* Close stdin, stdout, and stderr */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    // Process events
    notify.runloop();

    return 0;
}
