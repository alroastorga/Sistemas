#include <stdio.h>
#include <pthread.h>
#include <signal.h>

void nuevoCorredor();

/*Array de 5 hilos uno por corredor maximo que se puede crear*/
pthread_t corredor[5];
/*Contador del numero actual de corredores*/
int numeroDeCorredor;

int main(){

  /*Asociar la señal SIGUSR1 a la función que crea un nuevo corredor*/
  signal(SIGUSR1, nuevoCorredor);

  /*(ELIMINAR)IMPRIMIR PID DEL PROCESO Y PAUSAR PROCESO PARA MANDARLE LA SEÑAL(ELIMINAR)*/
  printf("pid %d\n", getpid());
  pause();
}


void nuevoCorredor(){
  /*Mientras se crea un corredor se ignora la señal de crear uno nuevo*/
  signal(SIGUSR1, SIG_IGN);
  /*
  (EN CONSTRUCCION)COMPROBAR QUE NO HAYA YA 5 CORREDORES Y CREAR UN NUEVO CORREDOR(EN CONSTRUCCION)
  if(numeroDeCorredor < 5){
    pthread_create(&corredor[i], NULL, )
  }
  */

    /*(ELIMINAR)IMPRIMIR MENSAJE INFORMANDO DE LA RECEPCION DE LA SEÑAL(ELIMINAR)*/
  printf("Señal recibida");

  /*Asociamos de nuevo la señal SIGUSR1 con la función nuevoCorredor*/
  signal(SIGUSR1, nuevoCorredor);

}
