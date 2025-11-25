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

int leer_linea(int fd_in, int fd_out, int pid)
{
    char c;
    char buffer[BUF_SIZE];
    int i = 0;
    int r;

    while ((r = read(fd_in, &c, 1)) > 0)
    {
        buffer[i++] = c;
        if (c == '\n' || i == BUF_SIZE - 1)
            break;
    }

    if (r < 0)
    {
        perror("read fichero");
        return -1;
    }

    if (i > 0)
    {
        int w = write(fd_out, buffer, i);
        if (w != i)
        {
            perror("write fichero");
            return -1;
        }

        buffer[i] = '\0';
        printf("P%d: %s", pid, buffer);
        fflush(stdout);

        return 1;
    }

    return 0;
}

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
    {
        if (pipe(pipe_fd[i]) < 0)
        {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_PROCESOS; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }

        if (pid == 0)
        {
            int pid_hijo = i + 1;

            for (int j = 0; j < NUM_PROCESOS; j++)
            {
                if (j == i)
                {
                    close(pipe_fd[j][1]);
                }
                else if (j == (i + 1) % NUM_PROCESOS)
                {
                    close(pipe_fd[j][0]);
                }
                else
                {
                    close(pipe_fd[j][0]);
                    close(pipe_fd[j][1]);
                }
            }

            int read_fd = pipe_fd[i][0];
            int write_fd = pipe_fd[(i + 1) % NUM_PROCESOS][1];

            while (1)
            {
                char token;
                int r_token = read(read_fd, &token, 1);
                if (r_token < 0)
                {
                    perror("read token");
                    exit(1);
                }
                if (r_token == 0)
                    break;

                int result = leer_linea(fd_in, fd_out, pid_hijo);
                if (result < 0)
                    exit(1);

                if (result > 0)
                {
                    if (write(write_fd, "T", 1) != 1)
                    {
                        perror("write token");
                        exit(1);
                    }
                }

                if (result == 0)
                {
                    close(write_fd);
                    break;
                }
            }

            close(read_fd);
            close(fd_in);
            close(fd_out);
            exit(0);
        }
    }

    for (int i = 0; i < NUM_PROCESOS; i++)
    {
        close(pipe_fd[i][0]);
        if (i != 0)
            close(pipe_fd[i][1]);
    }

    if (write(pipe_fd[0][1], "T", 1) != 1)
    {
        perror("write token inicial");
        exit(1);
    }
    close(pipe_fd[0][1]);

    for (int i = 0; i < NUM_PROCESOS; i++)
        wait(NULL);

    pid_t pid_diff = fork();
    if (pid_diff < 0)
    {
        perror("fork diff");
        exit(1);
    }
    if (pid_diff == 0)
    {
        execlp("diff", "diff", argv[1], argv[2], NULL);
        perror("exec diff");
        exit(1);
    }
    else
    {
        wait(NULL);
    }

    close(fd_in);
    close(fd_out);
    return 0;
}