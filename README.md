# bolos

Produce un árbol de procesos (cambiándoles el nombre a los del dibujo) de manera:
```
         A
      B     C
    D    E     F 
 G    H     I     J 
 ```
 
 Deja los procesos vivios hasta que al programa se le manda una señal SIGTERM desde consola. Entonces, El proceso A enviará una señal a:
 - El proceso de su izquierda (B)
 - El proceso de su derecha (C)
 - Ambos procesos
 - Ningún proceso
 
 Está decisión la determinará el reloj del sistema. Los bolos a los que les llegue la señal la propagarán de la misma manera, hasta que el se llegue a la última fila o la señal haya muerto por el camino porque no se propaga.
 Se imprimirá por pantalla la situación, mostrando los bolos muertos y los que quedan de pie y a continuación se matará a todos. Por último se realiza un ps -fu para comprobar que todos los procesos han muerto con éxito.
