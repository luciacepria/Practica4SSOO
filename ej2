#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


main(int argc, char **argv)
{
int t1[2], t2[2], t3[2], t4[2];
char dato;

pipe(t1);
pipe(t2);
pipe(t3);
pipe(t4);
if (fork()!=0)     
{                /* Tatarabuelo ejecuta grep ^d >> argv[2] */ 
  close(t4[1]); close(t4[0]);
  close(t3[1]); close(t3[0]);
  close(t2[1]); close(t2[0]);

  close(0);   
  dup(t2[0]);
  close(t2[0]);
  close(t2[1]);
  
  wait(NULL);

  close(1);   
  open(argv[2],O_WRONLY|O_CREAT|O_APPEND,0777);

  
  write(1,"Grep output:\n",20); 
  execlp("grep","grep","^d",NULL);
  perror("Error en ejecucion de grep ^d");
  exit(-1);

} else {
   if (fork()!=0) /* Bisabuelo ejecuta head -1 > argv[2] */
   { 
     close(t4[1]); close(t4[0]);
     close(t2[1]); close(t2[0]);
     close(t1[1]); close(t1[0]);

     close(0);   
     dup(t3[0]);
     close(t3[0]);
     close(t3[1]);

     close(1);   
     open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0777); //revisar 0777

     write(1,"Heap output:\n",20); 
     execlp("head","head","-1",NULL);
     perror("Error en ejecucion de head ");
     exit(-1);
   } else {
     if (fork()!=0)    /* Abuelo ejecuta grep -v ^d */
     { 
       close(t4[0]); close(t4[1]);
       close(t2[0]); close(t2[1]);
       
       close(0);
       dup(t1[0]);
       close(t1[0]);
       close(t1[1]);
       
       close(1);
       dup(t3[1]);
       close(t3[0]);
       close(t3[1]);
       
       execlp("grep","grep","-v","^d",NULL);
       perror("Error en ejecucion de grep -v");
       exit(-1);

     } else {
       if (fork()!=0)    /* Padre lee de ls y reparte a t1 y t2 */
       { 
         close(t4[1]); 
         close(t3[1]); close(t3[0]);
         close(t2[0]); 
         close(t1[0]);
        
         while (read(t4[0],&dato,sizeof(dato))>0){
            write(t1[1], &dato,sizeof(dato));
            write(t2[1], &dato,sizeof(dato));
        }
  
         close(t4[0]);
         close(t1[1]);
         close(t2[1]);
         wait(NULL);
         

     } else {            /* Hijo ejecuta ls -l */
         close(t1[1]); close(t1[0]);
         close(t2[1]); close(t2[0]);
         close(t3[1]); close(t3[0]);

         close(1);   
         dup(t4[1]);
         close(t4[1]);
         close(t4[0]);

         close(2);
         open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,0777); 

         execlp("ls","ls","-l",NULL);
         perror("Error en ejecucion de ls -l");
         exit(-1);
        }
    }
}
}
}