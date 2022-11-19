#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

//            A
//    B   H   E   I   C
//    D               F
//    G               J

int main(int argc, char *argv[])
{
    pid_t pid, pidP;
    if(strcmp(argv[0], "./bolos") == 0) {
        int i, j=0;
        
        char padres [5] = {'B','H','E','I','C'};
        char hijos  [5] = {'D',' ',' ',' ','F'};
        char nietos [5] = {'G',' ',' ',' ','J'};
        
        //Este salto de carro lo uso para que pueda ver todo mas comodo por consola
        printf("\n");
        
        //Creo el primer hijo 
        pid = fork();
        // el identificador del proceso padre es un numero positivo y el del hijo es 0, por tanto en este if solo entrará el proceso
        // inicial, al que nosotros llamamos P
        if(pid > 0)
        { 
            printf("Soy P y mi PID es : %d\n", getpid());
            // Me guardo el Pid para poder matarlo una vez creado el arbol
            pidP = getpid();
        }
        
        // Aqui entra el Hijo, al cual llamamos a
        else if (pid == 0) 
        {
            printf("Soy A y mi PID es : %d\n", getpid());
            
            //Queremos que A tenga 5 hijos, por tanto hay que hacer que se divida 5 veces
            for(i=0; i<5; i++)
            {
                pid = fork();
                if(pid > 0)  break; // A esta iteracion entra solo A, que ya esta creado, por tanto le decimos que vuelva a iterar el bucle for
             
                else if (pid == 0) 
                {
                    printf("Soy %c y mi PID es : %d\n", padres[i], getpid());
                    
                    // Usamos este if para crear las cadenas de BDG y CFJ
                    if(i == 0 || i == 4)
                    {
                        pid = fork();
                        if(pid > 0)  break;
                        
                        else if (pid == 0) 
                        {
                            printf("Soy %c y mi PID es : %d\n", hijos[i], getpid());
                            pid = fork();
                            if(pid > 0)  break;
                        
                            else if (pid == 0) 
                            {
                                printf("Soy %c y mi PID es : %d\n", nietos[i], getpid());
                                // Los hijos ya no procrean, aqui han terminado
                                // if (i == 0)
                                // {
                                //     // strcpy(argv[0], "G");
                                //     // execv("bolos", argv);
                                // } else
                                // {
                                //     // strcpy(argv[0], "J");
                                //     // execv("bolos", argv);
                                // }
                            }
                            // D y F ya han terminado de procrear
                            if(i == 0) {
                                // strcpy(argv[0], "D");
                                // execv("bolos", argv);
                            } else {
                                // strcpy(argv[0], "F");
                                // execv("bolos", argv);
                            }
                        }
                    }
                    // Todos los procesos han terminado de procrear
                    // switch (i)
                    // {
                    // case 0:
                    //     strcpy(argv[0], "B");
                    //     execv("bolos", argv);
                    //     break;
    
                    // case 1:
                    //     strcpy(argv[0], "H");
                    //     execv("bolos", argv);
                    //     break;
    
                    // case 2:
                    //     strcpy(argv[0], "E");
                    //     execv("bolos", argv);
                    //     break;
                    
                    // case 3:
                    //     strcpy(argv[0], "I");
                    //     execv("bolos", argv);
                    //     break;
                    
                    // case 4:
                    //     strcpy(argv[0], "C");
                    //     execv("bolos", argv);
                    //     break;
                    
                    // default:
                    //     fprintf(stderr, "Error desconocido");
                    //     break;
                    // }
                }
            }
            // Despues de crear los hijos le cambiamos le nombre a A
            // strcpy(argv[0], "A");
            // execv("bolos", argv);
        }
    
    } else {
        printf("%s\n", argv[0]);
        // switch (nombre)
        // {
        // case 'A':
        //   printf("Hola soy el proceso A\n");
        //   break;

        // case 'B':
        //   printf("Hola soy el proceso B\n");
        //   break;

        // case 'C':
        //   printf("Hola soy el proceso C\n");
        //   break;
        
        // case 'D':
        //   printf("Hola soy el proceso D\n");
        //   break;

        // case 'E':
        //   printf("Hola soy el proceso E\n");
        //   break;

        // case 'F':
        //   printf("Hola soy el proceso F\n");
        //   break;

        // case 'G':
        //   printf("Hola soy el proceso G\n");
        //   break;

        // case 'H':
        //   printf("Hola soy el proceso H\n");
        //   break;

        // case 'I':
        //   printf("Hola soy el proceso I\n");
        //   break;

        // case 'J':
        //   printf("Hola soy el proceso J\n");
        //   break;
        
        // default:
        //   fprintf(stderr, "Error desconocido");
        //   return -1;
        // }

    }
    // Matamos a P

    if(getpid() == pidP)
    {
        exit(0);
    }

    return 0;
}
        
            
