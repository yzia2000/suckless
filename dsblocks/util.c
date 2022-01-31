#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "util.h"

        void
cspawn(char *const *arg)
{
        setsid();
        execv(arg[0], arg);
        perror("cspawn - execv");
        _exit(127);
}

        void
csigself(int sig, int sigval)
{
        union sigval sv;

        sig += SIGRTMIN;
        sv.sival_int = sigval;
        if (sigqueue(pid, sig, sv) == -1) {
                perror("csigself - sigqueue");
                exit(1);
        }
}

/* getcmdout doesn't null terminate */
        ssize_t
getcmdout(char *const *arg, char *cmdout, size_t cmdoutlen)
{
        int fd[2];
        ssize_t rd;

        if (pipe(fd) == -1) {
                perror("getcmdout - pipe");
                exit(1);
        }
        switch (fork()) {
                case -1:
                        perror("getcmdout - fork");
                        exit(1);
                case 0:
                        close(ConnectionNumber(dpy));
                        close(fd[0]);
                        if (fd[1] != STDOUT_FILENO) {
                                if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
                                        perror("getcmdout - child - dup2");
                                        exit(1);
                                }
                                close(fd[1]);
                        }
                        execv(arg[0], arg);
                        perror("getcmdout - child - execv");
                        _exit(127);
                default:
                        close(fd[1]);
                        rd = read(fd[0], cmdout, cmdoutlen);
                        if (rd == -1) {
                                perror("getcmdout - read");
                                exit(1);
                        }
                        close(fd[0]);
        }
        return rd;
}

int
readint(const char *path, int *var) {
        FILE *fp;

        if (!(fp = fopen(path, "r")))
                return 0;
        if (fscanf(fp, "%d", var) != 1) {
                fclose(fp);
                return 0;
        }
        fclose(fp);
        return 1;
}

        void
uspawn(char *const *arg)
{
        switch (fork()) {
                case -1:
                        perror("uspawn - fork");
                        break;
                case 0:
                        close(ConnectionNumber(dpy));
                        setsid();
                        execv(arg[0], arg);
                        perror("uspawn - child - execv");
                        _exit(127);
        }
}

        int
pscanf(const char *path, const char *fmt, ...)
{
        FILE *fp;
        va_list ap;
        int n;

        if (!(fp = fopen(path, "r"))) {
                return -1;
        }
        va_start(ap, fmt);
        n = vfscanf(fp, fmt, ap);
        va_end(ap);
        fclose(fp);

        return (n == EOF) ? -1 : n;
}
