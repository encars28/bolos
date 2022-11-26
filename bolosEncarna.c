#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#define PROGRAMA argv[0]

int main(int argc, char *argv[])
{
    int i, j;
    pid_t pid;
    char padres [5] = {'B','H','E','I','C'};
    char hijos  [5] = {'D',' ',' ',' ','F'};
    char nietos [5] = {'G',' ',' ',' ','J'};

    // strcmp devuelve 0 si los strings son iguales
    if (strcmp(PROGRAMA, "./bolos") == 0) {
        // Estamos en el programa principal
        // Lo primero que hacemos es crear el proceso A

        //Este salto de carro lo uso para que pueda ver todo mas comodo por consola
        printf("\n");

        //Creo el primer hijo 
        pid = fork();
        // el identificador del proceso padre es un numero positivo y el del hijo es 0, por tanto en este if solo entrará el proceso
        // inicial, al que nosotros llamamos P

        if (pid == 0)
        {
            // Aqui entra el Hijo, al cual llamamos A
            sleep(1);
            // Le meto un mini sleep para que me imprima el soy A con el resto de procesos y no con el P
            printf("\nSoy A y mi PID es : %d\n", getpid());
            // Le cambiamos el nombre a A
            strcpy(PROGRAMA, "A");
            execv("bolos", argv);
            // Lo que hace execv es que el programa entero se vuelva a ejecutar, nada mas que ahora
            // Como le hemos cambiado argv[0] a "A", en vez de entrar por esta rama del if
            // Entrara por la siguiente
        }

        printf("Soy P y mi PID es : %d\n", getpid());
        exit(0);

    } else {
        char nombre = argv[0][0];
        switch (nombre)
        {
            case 'A':
                printf("Hola soy el proceso A\n");
                
                //Queremos que A tenga 5 hijos, por tanto hay que hacer que se divida 5 veces
                for (i = 0; i < 5; i++) {
                    pid = fork();

                    if(pid > 0)  {
                        continue; // A esta iteracion entra solo A, que ya esta creado, por tanto le decimos que vuelva a iterar el bucle for
                    }

                    else if (pid == 0) 
                    {
                        printf("Soy %c y mi PID es : %d\n", padres[i], getpid());

                        // Comprobamos cual de los 5 hijos es y le cambiamos el nombre
                        switch (i)
                        {
                            case 0:
                                strcpy(PROGRAMA, "B");
                                execv("bolos", argv);
                                break;

                            case 1:
                                strcpy(PROGRAMA, "H");
                                execv("bolos", argv);
                                break;

                            case 2:
                                strcpy(PROGRAMA, "E");
                                execv("bolos", argv);
                                break;

                            case 3:
                                strcpy(PROGRAMA, "I");
                                execv("bolos", argv);
                                break;

                            case 4:
                                strcpy(PROGRAMA, "C");
                                execv("bolos", argv);
                                break;

                            default:
                                fprintf(stderr, "Error desconocido");
                                break;
                        }

                    }
                }


                // El proceso A ha terminado de procrear
                // Le ponemos a hacer nada
                while (1)
                {
                    pause();
                }

                break;

            case 'B':
                printf("Hola soy el proceso B\n");
                // El proceso B crea una nueva rama
                pid = fork();
                        
                if (pid == 0) 
                {
                    printf("Soy %c y mi PID es : %d\n", hijos[i], getpid());

                    // Cambiamos de nombre
                    strcpy(PROGRAMA, "D");
                    execv("bolos", argv);
                }

                while (1)
                {
                    pause();
                }


                break;

            case 'C':
                printf("Hola soy el proceso C\n");
                pid = fork();
                        
                if (pid == 0) 
                {
                    printf("Soy %c y mi PID es : %d\n", hijos[i], getpid());

                    // Cambiamos de nombre
                    strcpy(PROGRAMA, "F");
                    execv("bolos", argv);
                }

                while (1)
                {
                    pause();
                }

                break;

            case 'D':
                printf("Hola soy el proceso D\n");
                pid = fork();
                
                if (pid == 0) 
                {
                    printf("Soy %c y mi PID es : %d\n", nietos[i], getpid());
                    strcpy(PROGRAMA, "G");
                    execv("bolos", argv);
                }

                while (1)
                {
                    pause();
                }

                break;

            case 'E':
                printf("Hola soy el proceso E\n");
                while (1)
                {
                    pause();
                }
                break;

            case 'F':
                printf("Hola soy el proceso F\n");
                pid = fork();
                
                if (pid == 0) 
                {
                    printf("Soy %c y mi PID es : %d\n", nietos[i], getpid());
                    strcpy(PROGRAMA, "J");
                    execv("bolos", argv);
                }

                while (1)
                {
                    pause();
                }
                
                break;

            case 'G':
                printf("Hola soy el proceso G\n");
                while (1)
                {
                    pause();
                }
                break;

            case 'H':
                printf("Hola soy el proceso H\n");
                while (1)
                {
                    pause();
                }
                break;

            case 'I':
                printf("Hola soy el proceso I\n");
                while (1)
                {
                    pause();
                }
                break;

            case 'J':
                printf("Hola soy el proceso J\n");
                while (1)
                {
                    pause();
                }
                break;

            default:
                fprintf(stderr, "Error desconocido");
                return -1;
        }
    }


    return 0;
}
