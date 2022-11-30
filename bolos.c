#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <wait.h>

// Estas variables nos ayudarán luego a imprimir la situación pro pantalla
#define FIL 4
#define COL 7

/** PROTOTIPOS DE LAS FUNCIONES **/

void nada();
int propagar_senal(pid_t, pid_t);
void imprimir_bolos(char *);
int esperar_bloqueante(pid_t);
int esperar(pid_t);
void printefe(char *);

/** PROGRAMA PRINCIPAL **/

int main(int argc, char *argv[])
{
    /** DECLARACION DE VARIABLES **/

    // Variables auxiliares
    int i, res, estado, comprobacion;
    pid_t pid, pidHijoB, pidHijoD, pidHijoC, pidHijoF, pid1, pid2;
    pid_t pidA[5] = {0};
    
    // LIsta que usaremos para asignar los nombres a los hijos de A
    char *hijos[] = {"I", "H", "E", "B", "C"};
    char nombre;

    // Variables auxiliares que usaremos para convertir los pids a string, para poder pasarlos
    // Como argumentos del programa al llamar execl
    char str1[20], str2[20], str3[20];

    // Vector con el que mostraremos la situación de los bolos por pantalla, 
    // después de que la señal se propague
    char situacion[] = {'.', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};

    // Esta es la máscara que van a tener todos los procesos. Bloquea todas las señales menos
    // SIGINT   -> Ctrl + C
    // SIGTSTP  -> Ctrl + Z
    // SIGTERM  -> Señal que propagaremos más adelante

    sigset_t mascara;
    sigfillset(&mascara);
    sigdelset(&mascara, SIGINT);
    sigdelset(&mascara, SIGTSTP);
    sigdelset(&mascara, SIGTERM);

    // Creamos una manejadora de la señal SIGTERM, para poder cambiar el comportamiento
    // de los procesos cuando la reciban
    struct sigaction manejadora_sigterm;

    // Aquí indicamos que cuando un proceso reciba la señal salte a la función nada()
    manejadora_sigterm.sa_handler = &nada;
    manejadora_sigterm.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &manejadora_sigterm, NULL) == -1)
    {
        perror("sigaction() ");
        exit(1);
    }

    // strcmp devuelve 0 si los strings son iguales
    if (strcmp(argv[0], "./bolos") == 0)
    {
        /** PROCESO PADRE **/

        // Lo primero que hacemos es crear el proceso A
        pid_t pid = fork();

        // El identificador del proceso padre es un numero positivo y el del hijo es 0, por tanto en este if
        // solo entrará el proceso inicial, al que nosotros llamamos P
        if (pid == 0)
        {
            /** PROCESO A **/

            // Le cambiamos el nombre a A
            // Para ello volvemos a ejecutar el programa, pero con argv[0] cambiado al nombre que queremos.
            // Esto hará que en vez de entrar por esta rama del if entre por la siguiente
            strcpy(argv[0], "A");
            execl("bolos", argv[0], NULL);
        }
        else if (pid == -1)
        {
            // Error en la creación del proceso hijo
            perror("fork ");
            exit(1);
        }

        // TODO: esperar a que todo se haya creado para matar
        exit(0);
    }
    else
    {
        // Como el nombre del programa es un string, esta formado por el caracter del nombre y '\0'
        // Por eso, para poder hacer el switch cogemos el primer caracter
        nombre = argv[0][0];
        switch (nombre)
        {
        case 'A':

            /** PROCESO A **/

            // Queremos que A tenga 5 hijos, por tanto hay que hacer que se divida 5 veces
            for (i = 0; i < 5; i++)
            {
                pid = fork();

                if (pid > 0)
                {
                    // Aquí solo entra solo A

                    // Vamos a guardar el pid de los hijos de A para poder usarlos en el propagamiento de señal
                    pidA[i] = pid; 
                    // Ahora le decimos a A que vuelva a iterar, para que siga creando hijos
                    continue;      
                }
                else if (pid == -1)
                {
                    // Error en la creación del proceso hijo
                    perror("fork() ");
                    exit(1);
                }
                else if (pid == 0)
                {
                    /** PROCESOS HIJOS **/

                    // Para que cada proceso pueda propagar la señal les vamos a pasar los pids que necesitan en su
                    // cadena de argumentos (argv[1], argv[2]...)

                    // Para no repetir codigo les pasamos a todos los procesos todos los pids que se necesitan
                    // entre todos. Luego ya cada proceso cogerá los pids que necesite para propagar la señal
                        // I e H -> No propagan
                        // E     -> propraga a H y I 
                        // B     -> propraga a D y E    (D es hijo de B)
                        // C     -> propraga a F y E    (F es hijo de C)
                    
                    // Por lo tanto, pasamos como argumento los pids de I, H y E 

                    // Ya que argv es un vector de strings y los pids son enteros, 
                    // los transformamos a cadenas con sprintf.
                    strcpy(argv[0], hijos[i]);
                    sprintf(str1, "%d", pidA[0]);
                    sprintf(str2, "%d", pidA[1]);
                    sprintf(str3, "%d", pidA[2]);
                    execl("bolos", argv[0], str1, str2, str3, NULL);
                    
                    }
                }
                // El proceso A ha terminado de procrear
                // Le ponemos a hacer nada

                // Para ello usamos la funcion sigsuspend, que lo que hace es aplicar una
                // mascara al proceso y a continucación ejecutar la función pause().
                // De esta manera, el proceso estará bloqueado, sin consumir CPU hasta que le 
                // lleguen las señales deseadas
                sigsuspend(&mascara);

                // El proceso se sale del sigsuspend, porque le ha llegado una señal sigterm
                // Esta señal no matará al proceso si no que no hará nada (función nada()), por 
                // lo que nada más recibir la señal el proceso ejecutara la función propagar_señal
                res = propagar_senal(pidA[4], pidA[3]);

                sleep(4);

                // Después de la minisiesta vemos quién esta vivo

                // 1. Comprobamos el estado de E, H e I

                // Para ello usamos waitpid que en su verison no bloqueante devuelve:
                    // 0 si el proceso especificado sigue vivo o no existe
                    // -1 si ha habido algun error
                    // El pid del proceso muerto, en caso de muerte

                // Iremos indicando los bolos muertos con puntos en el vector de situacion
                
                for (i = 0; i < 3; i++)
                {
                    if (esperar(pidA[i]) == 0)
                    {
                        // El bolo esta muerto
                        switch (i)
                        {
                        case 0:
                            situacion[8] = '.';
                            break;
                        case 1:
                            situacion[7] = '.';
                            break;
                        case 2:
                            situacion[4] = '.';
                            break;
                        }
                    }

                    // Si el bolo esta vivo no hacemos nada, ya que en el vector donde tenemos 
                    // la situación los bolos están vivos por defecto, así que solo hay que 
                    // modificarlo para indicar que están muertos
                }

                // 2. Comprobamos la primera arista

                // Si A le ha enviado una senal a C, significa que B ha muerto
                    // res = 3 -> se envia la senal a los dos procesos de abajo
                    // res = 2 -> se envia la senal al proceso de la izquierda

                if (res == 3 || res == 2)
                {
                    // C esta muerto
                    situacion[1] = '.';

                    // Aqui hay dos opciones, que B se haya muerto sin propagar la señal
                    // o que B haya matado a la vez a su hijo, D
                    // Para saber cuál de las dos es comprobamos el estado de salida de B
                    comprobacion = waitpid(pidA[3], &estado, 0);
                    if (comprobacion == -1)
                    {
                        perror("waitpid ");
                        exit(1);
                    }

                    // WEXITSTATUS devuelve el estado de salida
                    if (WEXITSTATUS(estado))
                    {
                        // Si el estado de salida del hijo es distinto de 0,
                        // significa que D o G siguen todavia vivos
                        if (WEXITSTATUS(estado) == 1)
                        {
                            // Si solo hay un bolo vivo, este es G, y D esta muerto
                            situacion[3] = '.';
                        }

                        // La otra opción es que WEXITSTATUS(estado) sea 2, en ese caso tanto G como D
                        // estarían vivos. Pero ya hemos dicho antes, en este caso no se hace nada, porque 
                        // en el vector por defecto están todos los procesos vivos.
                    }
                    else 
                    {
                        // Si el estado de salida es 0, significa que todos los bolos de la rista
                        // estan muertos
                        situacion[3] = '.';
                        situacion[6] = '.';
                    }
                }

                // 2. Comprobamos la segunda arista

                // Esta comprobación es análoga a la anterior, nada más que ahora comprobamos que res sea 1 y no 2,
                    // res = 1 -> se envia la senal al proceso de la derecha

                if (res == 3 || res == 1)
                {
                    // C esta muerto
                    situacion[2] = '.';

                    // Aqui hay dos opciones, que C se haya muerto sin propagar la senal
                    // o que C haya matado a la vez a su hijo, F

                    // Sabiendo que C ha muerto podemos usar waitpid en su version bloqueante
                    int estado;
                    int comprobacion = waitpid(pidA[4], &estado, 0);

                    if (comprobacion == -1)
                    {
                        perror("waitpid() ");
                        exit(1);
                    }

                    if (WEXITSTATUS(estado))
                    {
                        // Si el estado de salida del hijo es distinto de 0,
                        // significa que F o J siguen todavia vivos

                        if (WEXITSTATUS(estado) == 1)
                        {
                            // Si solo hay un bolo vivo, este es J, y F esta muerto
                            situacion[5] = '.';
                        }
                    }
                    else if (WEXITSTATUS(estado) == 0)
                    {
                        // Si el estado de salida es 0, significa que todos los bolos de la rista
                        // estan muertos
                        situacion[5] = '.';
                        situacion[9] = '.';
                    }
                }

                imprimir_bolos(situacion);

                pid = fork();

                if (pid == -1)
                {
                    perror("wait() ");
                    exit(1);
                }

                if (pid == 0)
                {
                    // Es el hijo
                    char *argv[3] = {"ps", "-f", NULL};
                    execv("/bin/ps", argv);
                }

                // A procede a matar todo
                if (waitpid(pid, NULL, 0) == -1)
                {
                    perror("waitpid() ");
                    exit(1);
                }

                kill(0, SIGINT);

                break;

            case 'B':
                
                /** PROCESO B **/

                // El proceso B propaga la señal a los procesos D y E. Por lo tanto: 
                    // Le pasamos en argv[3] el pid del proceso E. 
                    // El pid del proceso D lo tendrá cuando este sea generado, debido a que D es hijo de B. 
                
                // En argv[2] le pasamos el pid de H para que B le pueda pasar este pid a D, que
                // lo necesita para que D le propague la señal a H.

                // El proceso B crea una nueva rama
                pid = fork();
                if (pid > 0)
                {
                    // Guardamos el pid de D para poder propagar la señal mas adelante
                    pidHijoB = pid;
                    pid1 = (pid_t)atoi(argv[3]);
                    
                }
                else if (pid == 0)
                {
                    // Cambiamos de nombre
                    strcpy(argv[0], "D");
                    // Le pasamos a D el pid de H, que esta en argv[1] para que pueda propagar la señal
                    execl("bolos", argv[0], argv[2], NULL);
                }
                else if (pid == -1)
                {
                    perror("fork() ");
                    exit(1);
                }

                sigsuspend(&mascara);
                res = propagar_senal(pid1, pidHijoB);
                // Si B le ha enviado una senal a D, significa que D ha muerto
                // res = 3 -> se envia la senal a los dos procesos de abajo
                // res = 2 -> se envia la senal al proceso de la izquierda

                if (res == 3 || res == 2)
                {
                    // Aqui hay dos opciones, que D se haya muerto sin propagar la senal
                    // o que D haya matado a la vez a su hijo, G
                    int espera = esperar_bloqueante(pidHijoB);
                    if (espera == 0)
                    {
                        // No hay volos vivos
                        exit(0);
                    }

                    // Hay un volo vivo
                    exit(1);
                }

                // B no le ha enviado al senal a D, por lo que tanto D como G siguen vivos
                // B muere indicando que hay 2 bolos vivos
                exit(2);

            case 'C':

                /** PROCESO C **/

                // El proceso C propaga la señal a los procesos F y E. Por tanto: 
                    // le paso en argv[3] el pid del proceso E. 
                    // El pid del proceso F lo tendrá cuando este sea generado, debido a que F es hijo de C. 
                    
                // En argv[1] le pasamos el pid de I para que C le pueda pasar este pid a F, que
                // lo necesita para que F le propague la señal a I.

                pid = fork();
                if (pid > 0)
                {
                    // Guardamos el pid de F para poder propagar la señal mas adelante
                    pidHijoC = pid;
                    pid1 = (pid_t)atoi(argv[3]);
                   
                }
                if (pid == 0)
                {
                    // Cambiamos de nombre
                    strcpy(argv[0], "F");
                    // Le pasamos a F el pid de I, que esta en argv[1] para que propagie la señal mas adelante
                    execl("bolos", argv[0], argv[1], NULL);
                }
                else if (pid == -1)
                {
                    perror("fork() ");
                    exit(1);
                }

                sigsuspend(&mascara);
                res = propagar_senal(pidHijoC, pid1);
                // Si C le ha enviado una senal a F, significa que F ha muerto
                // res = 3 -> se envia la senal a los dos procesos de abajo
                // res = 1 -> se envia la senal al proceso de la derecha

                if (res == 3 || res == 1)
                {
                    // Aqui hay dos opciones, que F se haya muerto sin propagar la senal
                    // o que F haya matado a la vez a su hijo, J
                    int espera = esperar_bloqueante(pidHijoC);
                    if (espera == 0)
                    {
                        // No hay volos vivos
                        exit(0);
                    }

                    // Hay un volo vivo
                    exit(1);
                }

                // C no le ha enviado al senal a F, por lo que tanto F como J siguen vivos
                // C muere indicando que hay 2 bolos vivos
                exit(2);

            case 'D':

                pid = fork();
                if (pid > 0)
                {
                    // Guardamos el pid de G para poder propagar la señal mas adelante
                    pidHijoD = pid;
                    pid1 = (pid_t)atoi(argv[1]);
                    
                }
                if (pid == 0)
                {
                    strcpy(argv[0], "G");
                    // G no tiene que propagar la señal, solo la recibe.
                    execl("bolos", argv[0], NULL);
                }
                else if (pid == -1)
                {
                    perror("fork() ");
                    exit(1);
                }

                sigsuspend(&mascara);
                res = propagar_senal(pid1, pidHijoD);
                // Si D le ha enviado una senal a G, significa que G ha muerto
                // res = 3 -> se envia la senal a los dos procesos de abajo
                // res = 2 -> se envia la senal al proceso de la izquierda

                if (res == 3 || res == 2)
                {
                    // D se muere tranquilo sabiendo que ha matado a su hijo
                    exit(0);
                }

                // D se muere indicando que hay un bolo vivo
                exit(1);

            case 'E':
                // El proceso E propaga la señal a los procesos H e I. por tanto le paso: 
                    // en argv[1] el pid del proceso I
                    // en argv[2] le paso el pid del proceso H.  

                pid1 = (pid_t)atoi(argv[1]);
                pid2 = (pid_t)atoi(argv[2]);
                
                sigsuspend(&mascara);
                propagar_senal(pid1, pid2);
                exit(0);

            case 'F':
                pid = fork();
                if (pid > 0)
                {
                    // Guardamos el pid de J para poder propagar la señal mas adelante
                    pidHijoF = pid;
                    pid1 = (pid_t)atoi(argv[1]);
                    
                }
                if (pid == 0)
                {
                    strcpy(argv[0], "J");
                    // J no propaga señal, solo la recibe
                    execl("bolos", argv[0], NULL);
                }
                else if (pid == -1)
                {
                    perror("fork() ");
                    exit(1);
                }

                sigsuspend(&mascara);
                res = propagar_senal(pidHijoF, pid1);
                // Si F le ha enviado una senal a J, significa que J ha muerto
                // res = 3 -> se envia la senal a los dos procesos de abajo
                // res = 1 -> se envia la senal al proceso de la derecha

                if (res == 3 || res == 1)
                {
                    // F se muere tranquilo sabiendo que ha matado a su hijo
                    exit(0);
                }

                // F se muere indicando que hay un bolo vivo
                exit(1);

            // Estos son los procesos de fin de propagación, que solo se mueren cuando
            // reciben la señal, no la siguen propagando
            case 'G':
            case 'H':
            case 'I':
            case 'J':
                sigsuspend(&mascara);
                propagar_senal(0, 0);
                exit(0);

            default:
                fprintf(stderr, "Error desconocido");
                return -1;
            }
        }

        return 0;
    }

    // FUNCIONES

    // El nombre de la funcion es explicativo
    void nada() {}
    
    // Escribe por pantalla usando la llamada al sistema write
    void printefe(char *cadena)
    {
      write(1, cadena, strlen(cadena));
    }

    int propagar_senal(pid_t pidDer, pid_t pidIzq)
    {
        
        // Esta funcion sera en la que realizaremos el tratamiento de la señal SIGTERM recogida por cada bolo

        int err, random, i;
        struct timeval tiempo; // Declaracion necesaria para usar la funcion gettimeofday
        if (pidDer == 0 && pidIzq == 0)
        {
            random = 0;
        }
        else
        {
            gettimeofday(&tiempo, NULL);
            random = tiempo.tv_usec % 4; // tiempo.tv_usec nos da los milisegundos que cuenta el sistema. Se accede asi por nomenclatura de la biblioteca
        }
        switch (random)
        {
        case 0:
            // No hacemos nada
            break;
        case 1:

            // Propagamos la señal al bolo que esta abajo a la derecha
            err = kill(pidDer, SIGTERM);
            break;
        case 2:

            // Propagamos la señal al bolo que esta abajo a la izquierda
            
            err = kill(pidIzq, SIGTERM);
            break;
        case 3:
            // Propagamos el bolo a los 2 de abajo
           
            err = kill(pidDer, SIGTERM);
            err = kill(pidIzq, SIGTERM);
            break;
        }

        return random;
    }

    int esperar(pid_t hijo)
    {
        int comprobacion = waitpid(hijo, NULL, WNOHANG);

        if (comprobacion == -1)
        {
            perror("waitpid() ");
            exit(1);
        }

        if (comprobacion == 0)
        {
            // El proceso sigue vivo
            return 1;
        }
        else
        {
            // El proceso ha muerto
            return 0;
        }
    }

    int esperar_bloqueante(pid_t hijo)
    {
        int estado;
        int comprobacion = waitpid(hijo, &estado, 0);

        if (comprobacion == -1)
        {
            perror("waitpid() ");
            exit(1);
        }

        if (WEXITSTATUS(estado))
        {
            // Si el estado es 1, significa que hay un bolo vi
            return 1;
        }

        // No hay bolos vivos
        return 0;
    }

    void imprimir_bolos(char *situacion)
    {
        int i, j, k = 0;
        char cadena;
        printefe("\n\n");
        // relleno los bolos
        for (i = 0; i < FIL; i++)
        {
            for (j = 0; j < COL; j++)
            {
                switch (i)
                {
                case 0:
                    if (j == 3)
                    {
                        cadena = situacion[k];
                        printefe(&cadena);
                        k++;
                    }
                    else
                    {
                        printefe(" ");
                    }
                    break;

                case 1:
                    if (j == 2 || j == 4)
                    {
                        cadena = situacion[k];
                        printefe(&cadena);
                        k++;
                    }
                    else
                    {
                        printefe(" ");
                    }

                    break;

                case 2:
                    if (j == 1 || j == 3 || j == 5)
                    {
                        cadena = situacion[k];
                        printefe(&cadena);
                        k++;
                    }
                    else
                    {
                        printefe(" ");
                    }

                    break;

                case 3:
                    if (j == 0 || j == 2 || j == 4 || j == 6)
                    {
                        cadena = situacion[k];
                        printefe(&cadena);
                        k++;
                    }
                    else
                    {
                        printefe(" ");
                    }

                    break;

                default:
                    printefe(" ");
                    break;
                }
            }

            printefe("\n");
        }
        printefe("\n\n");
    }
