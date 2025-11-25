#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Introduce un fichero de entrada y otro de salida\n");
        exit(1);
    }

    int fd_in = open(argv[1], O_RDONLY);
    int fd_out = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644); /// crea si el fichero no existe y si existe lo abre para escritura y elimina su contenido
    if (fd_in < 0 || fd_out < 0)
    {
        perror("open");
        exit(1);
    }

    int t1[2], t2[2], t3[2];
    if (pipe(t1) < 0 || pipe(t2) < 0 || pipe(t3) < 0)
    {
        perror("pipe");
        exit(1);
    }

    if (fork() == 0)
    {
        close(t1[1]);
        close(t2[0]);
        close(t3[0]);
        close(t2[1]);
        close(t3[1]);

        while (1)
        {
            char token;
            if (read(t1[0], &token, 1) <= 0)
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
                write(t2[1], "T", 1); // pasa token a P2
            }
            else
                break;
        }
        close(t1[0]);
        close(t2[1]);
        close(fd_in);
        close(fd_out);
        exit(0);
    }

    // P2
    if (fork() == 0)
    {
        close(t2[1]);
        close(t1[0]);
        close(t3[0]);
        close(t1[1]);
        close(t3[1]);

        while (1)
        {
            char token;
            if (read(t2[0], &token, 1) <= 0)
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
                write(t3[1], "T", 1); // pasa token a P3
            }
            else
                break;
        }
        close(t2[0]);
        close(t3[1]);
        close(fd_in);
        close(fd_out);
        exit(0);
    }

    // P3
    if (fork() == 0)
    {
        close(t3[1]);
        close(t1[0]);
        close(t2[0]);
        close(t1[1]);
        close(t2[1]);

        while (1)
        {
            char token;
            if (read(t3[0], &token, 1) <= 0)
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
                write(t1[1], "T", 1); // pasa token a P1
            }
            else
                break;
        }
        close(t3[0]);
        close(t1[1]);
        close(fd_in);
        close(fd_out);
        exit(0);
    }

    // Padre
    close(t1[0]);
    close(t2[0]);
    close(t3[0]);
    close(t2[1]);
    close(t3[1]);

    // iniciar token
    write(t1[1], "T", 1);
    close(t1[1]);

    // esperar hijos
    for (int i = 0; i < 3; i++)
        wait(NULL);

    // ejecutar diff
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