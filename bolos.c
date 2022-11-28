#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <error.h>

#define PROGRAMA argv[0]

// int pidDer, int pidIzq
void a(){}

void trataSenal(int pidDer, int pidIzq)
{
   
  printf("pidDer: %d -- pidIzq: %d\n", pidDer, pidIzq);
  // Esta funcion sera en la que realizaremos el tratamiento de la señal SIGTERM recogida por cada bolo
   
  int err, random, i;
  struct timeval tiempo; // Declaracion necesaria para usar la funcion gettimeofday
  if(pidDer == 0 && pidIzq == 0)
  {
    random = 0;
  }
  else
  { 
    gettimeofday(&tiempo, NULL);  
    random = tiempo.tv_usec % 4; //tiempo.tv_usec nos da los milisegundos que cuenta el sistema. Se accede asi por nomenclatura de la biblioteca
  }
  switch(random)
  {
    case 0:
      
      // No hacemos nada
      printf("PID: %d. No hago nada\n", getpid());
      break;
    case 1:
      
      // Propagamos la señal al bolo que esta abajo a la derecha
      printf("PID: %d. Propago la señal al proceso de abajo derecha --> %d\n", getpid(), pidDer);
      err = kill(pidDer, SIGTERM);
      break;
    case 2:
      
      //Propagamos la señal al bolo que esta abajo a la izquierda
      printf("PID: %d. Propago la señal al proceso de abajo izquierda --> %d\n", getpid(), pidIzq);
      err = kill(pidIzq, SIGTERM);
      break;
    case 3:
      //Propagamos el bolo a los 2 de abajo
      printf("PID: %d. Propago la señal a los procesos de abajo --> %d  y  %d \n", getpid(), pidDer, pidIzq);
      err = kill(pidDer, SIGTERM);
      err = kill(pidIzq, SIGTERM);
      break;
  }

}

sigset_t crear_mascara() {
    sigset_t mascara;
    sigfillset(&mascara);
    sigdelset(&mascara, SIGINT);  // CTRL C
    sigdelset(&mascara, SIGTSTP); // CTRL Z
    sigdelset(&mascara, SIGTERM);

    return mascara;
}

int main(int argc, char *argv[])
{
  int i, pid1, pid2;
  char str1[20], str2[20];
    
  struct sigaction manejadoraSigterm;
  manejadoraSigterm.sa_handler = &a;
  manejadoraSigterm.sa_flags = SA_RESTART;
  sigaction(SIGTERM, &manejadoraSigterm, NULL);

  sigset_t mascara1 = crear_mascara();
   
  pid_t pid, pidA[5], pidHijoB, pidHijoD, pidHijoC, pidHijoF;
    
  char* hijos[] = {"I","H","E","B","C"};

  // strcmp devuelve 0 si los strings son iguales
  if (strcmp(PROGRAMA, "./bolos") == 0) 
  {
    // Estamos en el programa principal
    // Lo primero que hacemos es crear el proceso A

    //Creo el primer hijo 
    pid = fork();
    // el identificador del proceso padre es un numero positivo y el del hijo es 0, por tanto en este if solo entrará el proceso
    // inicial, al que nosotros llamamos P

    if (pid == 0)
    {
            
      // Le cambiamos el nombre a A
      strcpy(PROGRAMA, "A");
      execl("bolos", PROGRAMA, NULL);
      // Lo que hace execl es que el programa entero se vuelva a ejecutar, nada mas que ahora
      // Como le hemos cambiado argv[0] a "A", en vez de entrar por esta rama del if
      // Entrara por la siguiente
    }
    else if(pid == -1)
    {
      perror("fork() ");
      exit(1);
    }
    exit(0);

  }
  else 
  {
    char nombre = argv[0][0];
    switch (nombre)
    {
      case 'A':
      
        //Queremos que A tenga 5 hijos, por tanto hay que hacer que se divida 5 veces
        for (i = 0; i < 5; i++) 
        {
          pid = fork();

          if(pid > 0)
          {
                     
            pidA[i] = pid; //Vamos a guardar el pid de los hijos de A para poder usarlos en el propagamiento de señal
            continue; // A esta iteracion entra solo A, que ya esta creado, por tanto le decimos que vuelva a iterar el bucle for
          }
          else if(pid == -1)
          {
            perror("fork() ");
            exit(1);
          }
          else if (pid == 0) 
          {
            //Para que cada proceso pueda propagar la señal les vamos a pasar los pids que necesitan en su
            //cadena de argumentos (argv[1], argv[2]...)
            switch(i)
            {
              // Los procesos H e I procesos de fin de propagacion, no tienen que propagar la señal
              case 0:
              case 1:
                strcpy(PROGRAMA, hijos[i]);
                execl("bolos", PROGRAMA, NULL);
              break;
              // El proceso E propaga la señal a los procesos H e I, por tanto le paso en argv[1] el pid
              // del proceso I y en argv[2] le paso el pid del proceso H. Ya que argv es un vector de 
              // strings y los pids son enteros, los transformo a cadenas con Sprintf.
              case 2:
                strcpy(PROGRAMA, hijos[i]);
                sprintf(str1,"%d", pidA[0]);
                sprintf(str2,"%d", pidA[1]);
                execl("bolos", PROGRAMA, str1, str2, NULL);
              break;
              // El proceso B propaga la señal a los procesos D y E, por tanto le paso en argv[2] el pid
              // del proceso E. El pid del proceso D lo tendrá cuando este sea generado, debido a que D es
              // hijo de B. En argv[1] le pasamos el pid de H para que B le pueda pasar este pid a D, que
              // lo necesita para que D le propague la señal a H.
              case 3:
                strcpy(PROGRAMA, hijos[i]);
                sprintf(str1,"%d", pidA[1]);
                sprintf(str2,"%d", pidA[2]);
                execl("bolos", PROGRAMA, str1, str2, NULL);
              break;
              // El proceso C propaga la señal a los procesos F y E, por tanto le paso en argv[2] el pid
              // del proceso E. El pid del proceso F lo tendrá cuando este sea generado, debido a que F es
              // hijo de C. En argv[1] le pasamos el pid de I para que C le pueda pasar este pid a F, que
              // lo necesita para que F le propague la señal a I.
              case 4:
                strcpy(PROGRAMA, hijos[i]);
                sprintf(str1,"%d", pidA[0]);
                sprintf(str2,"%d", pidA[2]);
                execl("bolos", PROGRAMA, str1, str2, NULL);
              break;
                          
            }
          }
        }                
        // El proceso A ha terminado de procrear
        // Le ponemos a hacer nada
        printf("Hola soy el proceso A: %d.  Voy a propagar a B: %d  y  C: %d.\n", getpid(), pidA[3], pidA[4]);

        sigsuspend(&mascara1);
        trataSenal(pidA[4], pidA[3]);
                
        sleep(4);
        //captas quien vive
        //presentas por pantalla
        //haces el ps
        //matas todo     
      break;

      case 'B':
        // El proceso B crea una nueva rama
        pid = fork();
        if(pid > 0)
        {
          //Guardamos el pid de D para poder propagar la señal mas adelante
          pidHijoB = pid;
          pid1 = atoi(argv[2]);
          printf("Hola soy el proceso B: %d.  Voy a propagar a D: %d  y  E: %d.\n", getpid(), pidHijoB, pid1);
        }        
        else if (pid == 0) 
        {
          // Cambiamos de nombre
          strcpy(PROGRAMA, "D");
          //Le pasamos a D el pid de H, que esta en argv[1] para que pueda propagar la señal
          execl("bolos", PROGRAMA, argv[1], NULL);
        }
        else if(pid == -1)
        {
          perror("fork() ");
          exit(1);
        }

        sigsuspend(&mascara1);
        trataSenal(pid1, pidHijoB);
      break;

      case 'C':

        pid = fork();
        if(pid > 0)
        {
          //Guardamos el pid de F para poder propagar la señal mas adelante
          pidHijoC = pid;
          pid1 = atoi(argv[2]);
          printf("Hola soy el proceso C: %d.  Voy a propagar a F: %d  y  E: %d.\n", getpid(), pidHijoC, pid1);
        }       
        if (pid == 0) 
        {
          // Cambiamos de nombre
          strcpy(PROGRAMA, "F");
          //Le pasamos a F el pid de I, que esta en argv[1] para que propagie la señal mas adelante
          execl("bolos", PROGRAMA, argv[1], NULL);
        }
        else if(pid == -1)
        {
          perror("fork() ");
          exit(1);
        }

        sigsuspend(&mascara1);
        trataSenal(pidHijoC, pid1);
      break;

      case 'D':

        pid = fork();
        if(pid > 0)
        {
          //Guardamos el pid de G para poder propagar la señal mas adelante
          pidHijoD = pid;
          pid1 = atoi(argv[1]);
          printf("Hola soy el proceso D: %d.  Voy a propagar a G: %d  y  H: %d.\n", getpid(), pidHijoD, pid1);
        }
        if (pid == 0) 
        {
          strcpy(PROGRAMA, "G");
          //G no tiene que propagar la señal, solo la recibe.
          execl("bolos", PROGRAMA,  NULL);
        }
        else if(pid == -1)
        {
          perror("fork() ");
          exit(1);
        }
                
        sigsuspend(&mascara1);
        trataSenal(pid1, pidHijoD);
      break;

      case 'E':
        pid1 = atoi(argv[1]);
        pid2 = atoi(argv[2]);
        printf("Hola soy el proceso E: %d.  Voy a propagar a H: %d  y  I: %d.\n", getpid(), pid2, pid1);
        sigsuspend(&mascara1);
        trataSenal(pid1, pid2);
      break;

      case 'F':        
        pid = fork();
        if(pid > 0)
        {
          //Guardamos el pid de J para poder propagar la señal mas adelante
          pidHijoF = pid;
          pid1 = atoi(argv[1]);
          printf("Hola soy el proceso F: %d.  Voy a propagar a J: %d  y  I: %d.\n", getpid(), pidHijoF, pid1);
        }
        if (pid == 0) 
        {
          strcpy(PROGRAMA, "J");
          //J no propaga señal, solo la recibe
          execl("bolos", PROGRAMA, NULL);
        }
        else if(pid == -1)
        {
          perror("fork() ");
          exit(1);
        }       
         
        sigsuspend(&mascara1);
        trataSenal(pidHijoF, pid1);        
      break;

      case 'G':
        printf("Hola soy el proceso G: %d.\n", getpid());
        sigsuspend(&mascara1);
        pid1=0;
        trataSenal(pid1, pid1);
      break;

      case 'H':
        printf("Hola soy el proceso H: %d.\n", getpid());
        sigsuspend(&mascara1);
        pid1=0;
        trataSenal(pid1, pid1);        
      break;

      case 'I':
        printf("Hola soy el proceso I: %d.\n", getpid());
        sigsuspend(&mascara1);        
        pid1=0;
        trataSenal(pid1, pid1);        
      break;

      case 'J':
        printf("Hola soy el proceso J: %d.\n", getpid());
        sigsuspend(&mascara1);        
        pid1=0;
        trataSenal(pid1, pid1);        
      break;

      default:
        fprintf(stderr, "Error desconocido");
      return -1;
    }
  }
  
  return 0;
}
