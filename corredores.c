#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

void nuevoCorredor();
void* accionesCorredor(void* );

/*Array de 5 hilos uno por corredor maximo que se puede crear*/
pthread_t corredor[5];
/*Contador del numero actual de corredores*/
int numeroDeCorredor;

int main(){

  /*Asociar la señal SIGUSR1 a la función que crea un nuevo corredor*/
  signal(SIGUSR1, nuevoCorredor);

  /*Inicializar corredores*/
  numeroDeCorredor = 0;

  /*(ELIMINAR)IMPRIMIR PID DEL PROCESO Y PAUSAR PROCESO PARA MANDARLE LA SEÑAL(ELIMINAR)*/
  printf("pid %d\n", getpid());
  pause();

}


void nuevoCorredor(){
  /*Mientras se crea un corredor se ignora la señal de crear uno nuevo*/
  signal(SIGUSR1, SIG_IGN);

  /*(EN CONSTRUCCION)COMPROBAR QUE NO HAYA YA 5 CORREDORES Y CREAR UN NUEVO CORREDOR(EN CONSTRUCCION)*/
  if(numeroDeCorredor < 4){
    /*Crear un hilo que simula un corredor y actualizar contador de corredores*/
    corredor[numeroDeCorredor] = numeroDeCorredor;
    numeroDeCorredor++;

    int ret = pthread_create (&corredor[numeroDeCorredor], NULL, accionesCorredor, NULL);
    if(ret!=0){
      printf("Error al crear el hilo\n");
    }else{
      printf("Hilo creado\n");
    }

  }

  /*Asociamos de nuevo la señal SIGUSR1 con la función nuevoCorredor*/
  signal(SIGUSR1, nuevoCorredor);

}

/*(ERROR)NO LLEGA A EJECUTAR LA FUNCION PERO EL HILO SE CREA...(ERROR)*/
void *accionesCorredor(void* parametro){
  printf("Soy un corredor");
  return 0;
}
