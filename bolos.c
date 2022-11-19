#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>



int main(){
  
  int i, j=0;
  pid_t pid, pidP;
  
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
            }
          }
        }
      }
    }
  }
   

  // Matamos a P
  if(getpid() == pidP)
  {
    exit(0);
  }
  sleep(20);
  return 0;
}
      
      
