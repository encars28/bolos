#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>

// Estas variables nos ayudarán luego a imprimir la situación pro pantalla
#define FIL 4
#define COL 7

/** PROTOTIPOS DE LAS FUNCIONES **/

void nada();
int propagar_senal(pid_t, pid_t);
void imprimir_bolos(char *[]);
int esperar_bloqueante(pid_t);
int esperar(pid_t);
void printefe(char *);
void printeferr(char *);

/** PROGRAMA PRINCIPAL **/

int main(int argc, char *argv[])
{
    /** DECLARACION DE VARIABLES **/

    // Variables auxiliares
    int i, res, estado, err;
    pid_t pid, pidHijo, pid1, pid2;
    pid_t pidA[5] = {0};
    
    // LIsta que usaremos para asignar los nombres a los hijos de A
    char *hijos[] = {"I", "H", "E", "B", "C"};
    char nombre;

    // Variables auxiliares que usaremos para convertir los pids a string, para poder pasarlos
    // Como argumentos del programa al llamar execl
    char str1[20], str2[20], str3[20];

    // Vector con el que mostraremos la situación de los bolos por pantalla, 
    // después de que la señal se propague
    char* situacion[] = {".", "B", "C", "D", "E", "F", "G", "H", "I", "J"};

    // Esta es la máscara que van a tener todos los procesos. Bloquea todas las señales menos
    // SIGINT   -> Ctrl + C
    // SIGTSTP  -> Ctrl + Z
    // SIGTERM  -> Señal que propagaremos más adelante

    sigset_t mascara;
    if (sigfillset(&mascara) == -1)
    {
        perror("sigfillset ");
        exit(-1);
    }
    if (sigdelset(&mascara, SIGINT) == -1)
    {
        perror("sigdelset ");
        exit(-1);
    }
    if (sigdelset(&mascara, SIGTSTP) == -1)
    {
        perror("sigdelset ");
        exit(-1);
    }
    if (sigdelset(&mascara, SIGTERM) == -1)
    {
        perror("sigdelset ");
        exit(-1);
    }

    // Creamos una manejadora de la señal SIGTERM, para poder cambiar el comportamiento
    // de los procesos cuando la reciban
    struct sigaction manejadora_sigterm;

    // Aquí indicamos que cuando un proceso reciba la señal salte a la función nada()
    manejadora_sigterm.sa_handler = &nada;
    manejadora_sigterm.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &manejadora_sigterm, NULL) == -1)
    {
        perror("sigaction ");
        exit(-1);
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
            err = execl("bolos", "A", NULL);
            if(err == -1)
            {
                perror("Execl ");
                exit(-1);
            }
        }
        else if (pid == -1)
        {
            // Error en la creación del proceso hijo
            perror("fork ");
            exit(-1);
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
                    perror("fork ");
                    exit(-1);
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
                    sprintf(str1, "%d", pidA[0]);
                    sprintf(str2, "%d", pidA[1]);
                    sprintf(str3, "%d", pidA[2]);
                    err = execl("bolos", hijos[i], str1, str2, str3, NULL);
                    if(err == -1)
                    {
                        perror("Execl ");
                        exit(-1);
                    }
                    }
                }
                // El proceso A ha terminado de procrear
                // Le ponemos a hacer nada

                // Para ello usamos la funcion sigsuspend, que lo que hace es aplicar una
                // mascara al proceso y a continucación ejecutar la función pause().
                // De esta manera, el proceso estará bloqueado, sin consumir CPU hasta que le 
                // lleguen las señales deseadas
                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
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
                            situacion[8] = ".";
                            break;
                        case 1:
                            situacion[7] = ".";
                            break;
                        case 2:
                            situacion[4] = ".";
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
                    situacion[1] = ".";

                    // Aqui hay dos opciones, que B se haya muerto sin propagar la señal
                    // o que B haya matado a la vez a su hijo, D
                    // Para saber cuál de las dos es comprobamos el estado de salida de B
                    if (waitpid(pidA[3], &estado, 0) == -1)
                    {
                        perror("waitpid ");
                        exit(-1);
                    }

                    // WEXITSTATUS devuelve el estado de salida
                    if (WEXITSTATUS(estado))
                    {
                        // Si el estado de salida del hijo es distinto de 0,
                        // significa que D o G siguen todavia vivos
                        if (WEXITSTATUS(estado) == 1)
                        {
                            // Si solo hay un bolo vivo, este es G, y D esta muerto
                            situacion[3] = ".";
                        }

                        // La otra opción es que WEXITSTATUS(estado) sea 2, en ese caso tanto G como D
                        // estarían vivos. Pero ya hemos dicho antes, en este caso no se hace nada, porque 
                        // en el vector por defecto están todos los procesos vivos.
                    }
                    else 
                    {
                        // Si el estado de salida es 0, significa que todos los bolos de la rista
                        // estan muertos
                        situacion[3] = ".";
                        situacion[6] = ".";
                    }
                }

                // 2. Comprobamos la segunda arista

                // Esta comprobación es análoga a la anterior, nada más que ahora comprobamos que res sea 1 y no 2,
                    // res = 1 -> se envia la senal al proceso de la derecha

                if (res == 3 || res == 1)
                {
                    // C esta muerto
                    situacion[2] = ".";

                    if (waitpid(pidA[4], &estado, 0) == -1)
                    {
                        perror("waitpid ");
                        exit(-1);
                    }

                    if (WEXITSTATUS(estado))
                    {
                        // Si el estado de salida del hijo es distinto de 0,
                        // significa que F o J siguen todavia vivos
                        if (WEXITSTATUS(estado) == 1)
                        {
                            // Si solo hay un bolo vivo, este es J, y F esta muerto
                            situacion[5] = ".";
                        }
                    }
                    else 
                    {
                        // Si el estado de salida es 0, significa que todos los bolos de la rista
                        // estan muertos
                        situacion[5] = ".";
                        situacion[9] = ".";
                    }
                }

                // Imprimimos la situación de forma bonita
                imprimir_bolos(situacion);

                // Hacemos que A haga un ps
                // Para ello creamos un hijo nuevo, que llamará a ps
                pid = fork();

                if (pid == -1)
                {
                    // Error en la creación del proceso hijo
                    perror("wait ");
                    exit(-1);
                }

                if (pid == 0)
                {
                    // Es el hijo
                    char *argv[3] = {"ps", "-fu", NULL};
                    execv("/bin/ps", argv);
                }

                // Esperamos a que le ps termine
                if (waitpid(pid, NULL, 0) == -1)
                {
                    perror("waitpid ");
                    exit(-1);
                }

                // A procede a matar todo
                if (kill(0, SIGINT) == -1) {
                    perror("kill ");
                    exit(-1);
                }

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
                    // Es el padre

                    // Guardamos el pid de D para poder propagar la señal mas adelante
                    pidHijo = pid;
                    // Aquí guardamos el pid del proceso E
                    pid1 = (pid_t)atoi(argv[3]);
                    
                }
                else if (pid == 0)
                {
                    // Le pasamos a D el pid de H, que esta en argv[2] para que pueda propagar la señal
                    execl("bolos", "D", argv[2], NULL);
                    if(err == -1)
                    {
                        perror("Execl ");
                        exit(-1);
                    }
                }
                else if (pid == -1)
                {
                    // Error en la creación del proceso hijo
                    perror("fork ");
                    exit(-1);
                }

                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
                res = propagar_senal(pid1, pidHijo);

                // Para comprobar que bolos están vivos y están muertos en la rista, hacemos 
                // con B una cosa parecida a lo que hemos hecho con A

                // Si B le ha enviado una senal a D, significa que D ha muerto
                if (res == 3 || res == 2)
                {
                    // Aqui hay dos opciones, que D se haya muerto sin propagar la senal
                    // o que D haya matado a la vez a su hijo, G
                    if (esperar_bloqueante(pidHijo) == 0)
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

                // El proceso C crea una nueva rama
                pid = fork();
                if (pid > 0)
                {
                    // Es el padre

                    pidHijo = pid;
                    // Guardamos el pid de E para poder propagar la señal mas adelante
                    pid1 = (pid_t)atoi(argv[3]);
                   
                }
                if (pid == 0)
                {
                    // Le pasamos a F el pid de I, que esta en argv[1] para que propagie la señal mas adelante
                    execl("bolos", "F", argv[1], NULL);
                    if(err == -1)
                    {
                        perror("Execl ");
                        exit(-1);
                    }
                }
                else if (pid == -1)
                {
                    // Error en la creación del hijo
                    perror("fork ");
                    exit(-1);
                }

                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
                res = propagar_senal(pidHijo, pid1);
                
                // Si C le ha enviado una senal a F, significa que F ha muerto
                if (res == 3 || res == 1)
                {
                    // Aqui hay dos opciones, que F se haya muerto sin propagar la senal
                    // o que F haya matado a la vez a su hijo, J
                    if (esperar_bloqueante(pidHijo) == 0)
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

                /** PROCESO D **/

                // El proceso D crea una nueva rama
                pid = fork();
                if (pid > 0)
                {
                    // Guardamos el pid de G para poder propagar la señal mas adelante
                    pidHijo = pid;
                    pid1 = (pid_t)atoi(argv[1]);
                    
                }
                if (pid == 0)
                {
                    // G no tiene que propagar la señal, solo la recibe.
                    execl("bolos", "G", NULL);
                    if(err == -1)
                    {
                        perror("Execl ");
                        exit(-1);
                    }
                }
                else if (pid == -1)
                {
                    // Error en la creación de procesos hijos
                    perror("fork ");
                    exit(-1);
                }

                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
                res = propagar_senal(pid1, pidHijo);
                
                // Si D le ha enviado una senal a G, significa que G ha muerto
                if (res == 3 || res == 2)
                {
                    // D se muere tranquilo sabiendo que ha matado a su hijo
                    exit(0);
                }

                // D se muere indicando que hay un bolo vivo
                exit(1);

            case 'E':

                /** PROCESO E **/

                // El proceso E propaga la señal a los procesos H e I. por tanto le paso: 
                    // en argv[1] el pid del proceso I
                    // en argv[2] le paso el pid del proceso H.  

                pid1 = (pid_t)atoi(argv[1]);
                pid2 = (pid_t)atoi(argv[2]);
                
                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
                propagar_senal(pid1, pid2);
                exit(0);

            case 'F':

                /** PROCESO F **/

                // El proceso F crea una nueva rama
                pid = fork();
                if (pid > 0)
                {
                    // Guardamos el pid de J para poder propagar la señal mas adelante
                    pidHijo = pid;
                    // F propaga a I, así que guardamos el pid
                    pid1 = (pid_t)atoi(argv[1]);
                    
                }
                if (pid == 0)
                {
                    // J no propaga señal, solo la recibe
                    execl("bolos", "J", NULL);
                    if(err == -1)
                    {
                        perror("Execl ");
                        exit(-1);
                    }
                }
                else if (pid == -1)
                {
                    // Error en la creación de un proceso hijo
                    perror("fork ");
                    exit(-1);
                }

                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
                res = propagar_senal(pidHijo, pid1);
                
                // Si F le ha enviado una senal a J, significa que J ha muerto
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
                
                /** PROCESOS G, H, I, J **/

                sigsuspend(&mascara);
                if (errno == EFAULT)
                {
                    printeferr("Problema con la memoria a la que apunta la mascara");
                    exit(-1);
                }
                propagar_senal(0, 0);
                exit(0);

            default:
                return -1;
            }
        }

        return 0;
    }


    /** FUNCIONES **/

    /*
     * Función:  nada
     * --------------------
     * El nombre de la función lo explica por sí solo
     */
    void nada() {}
    
    /*
     * Función:  printefe 
     * --------------------
     * Imprime una cadena de caracteres a pantalla
     *
     *  cadena: string a imprimir
     */
    void printefe(char *cadena)
    {
        int err;
        err = write(1, cadena, strlen(cadena));
        if(err == -1)
        {
            perror("write ");
            exit(-1);
        }
    }

    /*
     * Función:  printeferr
     * --------------------
     * Imprime una cadena de caracteres a la consola de errores
     *
     *  cadena: string a imprimir
     */
    void printeferr(char *cadena)
    {
        int err;
        err = write(2, cadena, strlen(cadena));
        if(err == -1)
        {
            perror("write ");
            exit(-1);
        }
    }


    /*
     * Función:  propagar_senal 
     * --------------------
     * Propaga la señal Sigterm a otros procesos después de haber sido recibida
     * según lo que indicque el relojo del sistema
     *
     *  pidDer: pid del proceso abajo a la derecha
     *  pidIzq: pid del proceso abajo a la izquierda
     *
     *  devuelve: que es lo que ha hecho el proceso cuando ha recibido la señal
     *                  0 -> Nada
     *                  1 -> Propagar la señal al bolo de abajo a la izquierda
     *                  2 -> Propagar la señal al bolo de abajo a la derecha
     *                  3 -> Propagar la señal a ambos bolos
     */
    int propagar_senal(pid_t pidDer, pid_t pidIzq)
    {
        int random, i;
        struct timeval tiempo;  // Declaracion necesaria para usar la funcion gettimeofday
        if (pidDer == 0 && pidIzq == 0)
        {
            // Aquí entran los procesos que ya no tienen que propagar la señal -> I, H, G y J
            random = 0;
        }
        else
        {
            if(gettimeofday(&tiempo, NULL) == -1) {
                perror("gettimeofday ");
                exit(-1);
            }
            random = tiempo.tv_usec % 4; // tiempo.tv_usec nos da los milisegundos que cuenta el sistema. Se accede asi por nomenclatura de la biblioteca
        }

        switch (random)
        {
        case 0:
            // No hacemos nada
            break;
        case 1:
            // Propagamos la señal al bolo que esta abajo a la derecha
            if (kill(pidDer, SIGTERM) == -1) {
                perror("kill ");
                exit(-1);
            }

            break;
        case 2:

            // Propagamos la señal al bolo que esta abajo a la izquierda
            if (kill(pidIzq, SIGTERM) == -1) {
                perror("kill ");
                exit(-1);
            }
            break;
        case 3:
            // Propagamos el bolo a los 2 de abajo
            if (kill(pidDer, SIGTERM) == -1) {
                perror("kill ");
                exit(-1);
            }

            if (kill(pidIzq, SIGTERM) == -1) {
                perror("kill ");
                exit(-1);
            }
            
            break;
        }

        return random;
    }

    /*
     * Función:  esperar
     * --------------------
     * Hace un waitpid no bloqueante. De esta manera vemos si el hijo está vivo
     *
     *  hijo: proceso hijo que queremos comprobar
     *
     *  devuelve: 
     *                  1 -> El proceso sigue vivo
     *                  0 -> El proceso ha muerto
     */
    int esperar(pid_t hijo)
    {
        int comprobacion = waitpid(hijo, NULL, WNOHANG);
        if (comprobacion == -1)
        {
            perror("waitpid() ");
            exit(-1);
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
    
    /*
     * Función:  esperar_bloqueante
     * --------------------
     * Hace un waitpid bloqueante. Usamos esta función cuando ya sabemos que
     * el hijo está muerto. De esta manera, obtenemos el valor de salida del proceso muerto
     *
     *  hijo: proceso hijo que ha muerto
     *
     *  devuelve: 
     *                  1 -> El proceso nieto está vivo
     *                  0 -> No hay ningún proceso vivo
     */
    int esperar_bloqueante(pid_t hijo)
    {
        int estado;
        if (waitpid(hijo, &estado, 0) == -1)
        {
            perror("waitpid() ");
            exit(-1);
        }

        if (WEXITSTATUS(estado))
        {
            // Si el estado es 1, significa que hay un bolo vi
            return 1;
        }

        // No hay bolos vivos
        return 0;
    }

    /*
     * Función:  imprimir_bolos
     * --------------------
     * Imprime los procesos de forma bonita (como bolos), indicando cuales están vivos
     */
    void imprimir_bolos(char *situacion[])
    {
        int i, j, k = 0;
        printefe("\n\n\n");
        // relleno los bolos
        for (i = 0; i < FIL; i++)
        {
            printefe("              ");
            for (j = 0; j < COL; j++)
            {
                switch (i)
                {
                case 0:
                    if (j == 3)
                    {
                        printefe(situacion[k]);
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
                        printefe(situacion[k]);
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
                        printefe(situacion[k]);
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
                        printefe(situacion[k]);
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
        printefe("\n\n\n");
    }
