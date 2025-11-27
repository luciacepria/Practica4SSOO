#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Introduce un número\n");
        exit(-1);
    }
    int numero = atoi(argv[1]);
    int val, i;
    int t1[2], t2[2], t3[2], t4[2], t5[2];

    pipe(t1);
    pipe(t2);
    pipe(t3);
    pipe(t4);
    pipe(t5);

    if (fork() != 0){ //bisabuelo lee el numero de teclado
        close(t5[1]);
        close(t4[1]);close(t4[0]);
        close(t3[1]);close(t3[0]);
        close(t2[0]);
        close(t1[0]);

        for (i = 0; i < numero; i++) {        //P1 lee del teclado y reparte a t1 y t2
            printf("Give me an integer: ");
            scanf("%d", &val);
            if (val % 2 == 0) {
                write(t1[1], &val, sizeof(int)); // Al abuelo (P2)
            } else {
                write(t2[1], &val, sizeof(int)); // Al padre (P3)
            }
        }
        close(t2[1]); 
        close(t1[1]);

        int total_final;
        if (read(t5[0], &total_final, sizeof(int)) > 0) {//Lee la suma total de P4
            printf("P1: Sum of all the numbers: %d\n", total_final);
        }

        close(t5[0]);
    }else{
        if (fork() != 0){ //abuelo P2 suma los pares
            int suma_pares = 0, num;
            close(t5[1]);close(t5[0]);
            close(t4[1]);close(t4[0]);
            close(t3[0]);
            close(t2[1]);close(t2[0]);
            close(t1[1]);

            
            while(read(t1[0], &num, sizeof(int)) > 0) { // Lee hasta que P1 cierra el pipe 
                suma_pares += num;
            }
            printf("P2: Sum of even numbers:    %d\n", suma_pares);// Muestra por pantalla
            
            write(t3[1], &suma_pares, sizeof(int)); // Enviar a P4
            
            close(t1[0]);
            close(t3[1]);
        }else{
            if (fork() != 0){ //padre P3 suma los impares
                int suma_impares = 0, num;
                close(t5[1]);close(t5[0]);
                close(t4[0]);
                close(t3[1]);close(t3[0]);
                close(t2[1]);
                close(t1[1]);close(t1[0]);
            
                while(read(t2[0], &num, sizeof(int)) > 0) { // Lee hasta que P1 cierra el pipe
                    suma_impares += num;
                }
                printf("P3: Sum of odd numbers:     %d\n", suma_impares);// Muestra por pantalla
                    
                write(t4[1], &suma_impares, sizeof(int)); //Enviar a P4
                    
                close(t2[0]);
                close(t4[1]);
            }else{ //hijo P4 suma la suma de pares e impares.
                int par = 0, impar = 0, total = 0;
                close(t5[0]);
                close(t4[1]); 
                close(t3[1]);
                close(t2[1]); close(t2[0]);
                close(t1[1]); close(t1[0]);
                
                read(t3[0], &par, sizeof(int));//Lee la suma de pares P4
                read(t4[0], &impar, sizeof(int));//Lee la suma de impares de P3
                        
                total = par + impar;

                printf("P4: Sum of all the numbers: %d\n", total);// Muestra por pantalla

                write(t5[1], &total, sizeof(int)); // Manda a P1

                close(t3[0]);
                close(t4[0]);
                close(t5[1]);
            }
        }
    }
}