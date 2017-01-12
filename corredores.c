#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/**
 * Capacidad maxima que tiene la pista para alojar los corredores a la vez.
 */
#define TAM_MAX_CORREDOR 5

/**
 * Contador del numero actual de corredores
 */
int numeroDeCorredor;

/**
 * Indica la cantidad de corredores que estan dentro de la pista
 */
int cantidadDeCorredoresActivos=0;

void init ();
void nuevoCorredor();
void* pista(void* );

int main(){

      
  signal(SIGUSR1, nuevoCorredor);
  srand(time(NULL));
  init();
      
  while(1){
        
        
    sleep(2);

  }      

  pthread_exit(NULL);

  return 0;

}

/**
 * Inicializa todas las variables globales.
 */
void init () {

  numeroDeCorredor = 0;
  cantidadDeCorredoresActivos = 0;

}

/**
 * Crea un nuevo corredor asignandole el numero que le corresponda.
 * Si ya hay 5 corredores activos en pista ignora la senal.
 */
void nuevoCorredor(){
    
  signal(SIGUSR1, SIG_IGN);  
  pthread_t corredor;

    if (cantidadDeCorredoresActivos < TAM_MAX_CORREDOR) {


      if(pthread_create (&corredor, NULL, pista, &numeroDeCorredor) != 0){

        printf("Error al crear el hilo\n");

      }

      else {

        cantidadDeCorredoresActivos++;
        numeroDeCorredor++;
        printf("El corredor numero %d ha entrado en pista.\n", numeroDeCorredor);

      }

    }

  /*Asociamos de nuevo la señal SIGUSR1 con la función nuevoCorredor*/
  signal(SIGUSR1, nuevoCorredor);

}

/**
 * Cada corredor guarda su numero asignado, da 5 vueltas a la pista y sale
 * dejando un hueco libre.
 */
void *pista(void* parametro){

      
  /**
   * Contiene el numero de vuetas que lleva el corredor.
   */
  int numeroDeVueltas = 0;

  /**
   * Contiene el nuero que ha sido asignado al corredor. 
   */
    int numeroCorredor = *(int*)parametro;

  /**
   * Contiene el tiempo que tarda en dar la vuelta en la que esta actualmente. 
   */
  int tiempoPorVuelta;

  while (numeroDeVueltas<5) {

    tiempoPorVuelta = rand()%3+2;
    sleep(tiempoPorVuelta);
    numeroDeVueltas++;
    
  }

  printf("El corredor numero %d ha acabado la carrera.\n", numeroCorredor);
  cantidadDeCorredoresActivos--;
    
}

