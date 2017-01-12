
/**
 * @author Alvaro Villar Marcos
 * @author Manuel Ugidos Fernandez
 * @author Juan Ramon Lastra Diaz
 * @author Juan Carlos Robles Fernandez
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

/**
 * Numero de vueltas que se tienen que dar a la pista
 */
#define NUM_VUELTAS 5

/**
 * maxCorredores indica la capacidad maxima que tiene la pista para alojar los corredores a la vez.
 */
int maxCorredores;

/**
 * maxBoxes indica el numero maximo de boxes que contiene la pista.
 */
int maxBoxes;

/**
 * numeroDeCorredor indica el numero que le corresponde a cada corredor
 */
int numeroDeCorredor;

/**
 * Indica la cantidad de corredores que estan dentro de la pista
 */
int cantidadDeCorredoresActivos;

/**
 * mejorTimepo[] contiene el tiempo mas rapido en hacer las 5 vueltas en segundos 
 * y el numero del hilo que lo consiguio.  
 */
int mejorTiempo[2];

void init ();
void nuevoCorredor();
void* pista(void* );

int main(int argc, char** argv){

  if (argc != 3) {

    printf("Error. Debe ingresar los argumenos numero_maximo_de_corredores y numero_de_boxes");
    exit(1);
  }

  else {

    maxCorredores = atoi(argv[1]);
    maxBoxes = atoi(argv[2]);

  }
      
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
  mejorTiempo[0] = 30;
  mejorTiempo[1] = 0;

}

/**
 * Crea un nuevo corredor asignandole el numero que le corresponda.
 * Si ya hay 5 corredores activos en pista ignora la senal.
 */
void nuevoCorredor(){
    
  signal(SIGUSR1, SIG_IGN);  
  pthread_t corredor;

    if (cantidadDeCorredoresActivos < maxCorredores) {

      if(pthread_create (&corredor, NULL, pista, &numeroDeCorredor) != 0){

        printf("Error al crear el hilo. %s\n", strerror(errno));

      }

      else {

        cantidadDeCorredoresActivos++;
        numeroDeCorredor++;
        printf("El corredor numero %d ha entrado en pista.\n", numeroDeCorredor);

      }

    }

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
   * El tiempo de vuelta por pista se calcula cogiendo un numero aleatorio entre 2 y 5.
   * Una vez acabada la carrera, se mira si ha rebajado la marca establecida
   */
  int tiempoPorVuelta = 0;

  /**
   * tiempoTotal contiene el tiempo que ha tardado el corredor en dar las 5 vueltas.
   */
  int tiempoTotal = 0;

  while (numeroDeVueltas < NUM_VUELTAS) {

    tiempoPorVuelta = rand()%3+2;
    tiempoTotal = tiempoTotal + tiempoPorVuelta;
    sleep(tiempoPorVuelta);
    numeroDeVueltas++;
    
  }

  if (tiempoTotal < mejorTiempo[0]) {

    mejorTiempo[0] = tiempoTotal;
    mejorTiempo[1] = numeroCorredor;

    printf("Ha mejorado la marca el corredor %d con %d segundos\n", mejorTiempo[1], mejorTiempo[0]);

  }

  printf("El corredor numero %d ha acabado la carrera.\n", numeroCorredor);
  cantidadDeCorredoresActivos--;
    
}

