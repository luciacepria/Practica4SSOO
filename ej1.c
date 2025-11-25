#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define NUM_PROCESOS 3
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s <archivo_entrada> <archivo_salida>\n", argv[0]);
        exit(1);
    }

    int fd_in = open(argv[1], O_RDONLY);
    int fd_out = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd_in < 0 || fd_out < 0)
    {
        perror("open");
        exit(1);
    }

    int pipe_fd[NUM_PROCESOS][2];
    for (int i = 0; i < NUM_PROCESOS; i++)
        if (pipe(pipe_fd[i]) < 0)
        {
            perror("pipe");
            exit(1);
        }

    if (fork() == 0)
    {
        close(pipe_fd[0][1]);
        close(pipe_fd[1][0]);
        close(pipe_fd[2][0]);
        close(pipe_fd[1][1]);
        close(pipe_fd[2][1]);

        while (1)
        {
            char token;
            if (read(pipe_fd[0][0], &token, 1) <= 0)
                break;
            char buffer[BUF_SIZE];
            int i = 0;
            char c;
            int r;
            while ((r = read(fd_in, &c, 1)) > 0)
            {
                buffer[i++] = c;
                if (c == '\n' || i == BUF_SIZE - 1)
                    break;
            }
            if (i > 0)
            {
                write(fd_out, buffer, i);
                buffer[i] = '\0';
                printf("P1: %s", buffer);
                fflush(stdout);
                write(pipe_fd[1][1], "T", 1);
            }
            else
                break;
        }
        close(pipe_fd[0][0]);
        close(pipe_fd[1][1]);
        close(fd_in);
        close(fd_out);
        exit(0);
    }

    if (fork() == 0)
    {
        close(pipe_fd[1][1]);
        close(pipe_fd[0][0]);
        close(pipe_fd[2][0]);
        close(pipe_fd[0][1]);
        close(pipe_fd[2][1]);

        while (1)
        {
            char token;
            if (read(pipe_fd[1][0], &token, 1) <= 0)
                break;
            char buffer[BUF_SIZE];
            int i = 0;
            char c;
            int r;
            while ((r = read(fd_in, &c, 1)) > 0)
            {
                buffer[i++] = c;
                if (c == '\n' || i == BUF_SIZE - 1)
                    break;
            }
            if (i > 0)
            {
                write(fd_out, buffer, i);
                buffer[i] = '\0';
                printf("P2: %s", buffer);
                fflush(stdout);
                write(pipe_fd[2][1], "T", 1);
            }
            else
                break;
        }
        close(pipe_fd[1][0]);
        close(pipe_fd[2][1]);
        close(fd_in);
        close(fd_out);
        exit(0);
    }

    if (fork() == 0)
    {
        close(pipe_fd[2][1]);
        close(pipe_fd[0][0]);
        close(pipe_fd[1][0]);
        close(pipe_fd[0][1]);
        close(pipe_fd[1][1]);

        while (1)
        {
            char token;
            if (read(pipe_fd[2][0], &token, 1) <= 0)
                break;
            char buffer[BUF_SIZE];
            int i = 0;
            char c;
            int r;
            while ((r = read(fd_in, &c, 1)) > 0)
            {
                buffer[i++] = c;
                if (c == '\n' || i == BUF_SIZE - 1)
                    break;
            }
            if (i > 0)
            {
                write(fd_out, buffer, i);
                buffer[i] = '\0';
                printf("P3: %s", buffer);
                fflush(stdout);
                write(pipe_fd[0][1], "T", 1);
            }
            else
                break;
        }
        close(pipe_fd[2][0]);
        close(pipe_fd[0][1]);
        close(fd_in);
        close(fd_out);
        exit(0);
    }

    close(pipe_fd[0][0]);
    close(pipe_fd[1][0]);
    close(pipe_fd[2][0]);
    close(pipe_fd[1][1]);
    close(pipe_fd[2][1]);
    write(pipe_fd[0][1], "T", 1);
    close(pipe_fd[0][1]);

    for (int i = 0; i < NUM_PROCESOS; i++)
        wait(NULL);

    if (fork() == 0)
    {
        execlp("diff", "diff", argv[1], argv[2], NULL);
        perror("exec diff");
        exit(1);
    }
    else
        wait(NULL);

    close(fd_in);
    close(fd_out);
    return 0;
}