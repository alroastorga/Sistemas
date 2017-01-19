
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
 * mejorTiempo es la estructura que contiene la mejor marca de toda la carrera.
 * idCorredor contiene el id de corredor.
 * tiempo contiene el tiempo que marco el corredor.
 */
struct mejorTiempo {

  char idCorredor[13];
  int tiempo;

} mejorTiempo;

/**
 * corredor es la estructura que tiene un corredor.
 * id contiene el id que representa a cada corredor.
 * numero es el numero que ha sido asignado a cada corredor.
 * siguiente contiene la direccion de memoria del corredor que entrara despues.
 */
struct corredor {

  char id[13];
  int numero;
  struct corredor* siguiente;

} corredor;

/**
 * listaCorredores lista en la que se alojan los corredores.
 * cabeza contiene la direccion de memoria del primer corredor en entrar a la pista.
 * cola contiene la direccion de memoria del ultimo corredor que ha entrado en la pista.
 */
struct listaCorredores {

  struct corredor* cabeza;
  struct corredor* cola;
  
} listaCorredores;

struct listaCorredores;
struct mejorMarca;

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


/*
 *Obtiene el valor del corredor que será penalizado.
 */
char penalizador[13];



/*********************/
/********************/

int variable_comprobadora;

/*******************/
/******************/

/*Variable para que se comiencen a probar si ha sido sancionado.*/

int corredorCompruebaEntrada;
int sancionJuez;
/*El juez está listo para sancionar.*/

/*Se crea la condicion del hilo.*/
pthread_cond_t condicion;
/**
 * mutexListaCorredores controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaCorredores;
pthread_mutex_t mutexJuez;

void init ();
void aniadirCorredor (struct corredor* nuevoCorredor);
void eliminarCorredor (struct corredor* corredorAEliminar);
void nuevoCorredor();
void* pista(void* );
void* juez (void*);
void crearJuez();

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
  crearJuez();

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

  numeroDeCorredor = 1;
  cantidadDeCorredoresActivos = 0;
  mejorTiempo.tiempo = 100;
  listaCorredores.cabeza = NULL;
  listaCorredores.cola = NULL;

}

/**
 * aniadirCorredor añade en la cola un nuevo corredor pasador por parametro
 */
void aniadirCorredor (struct corredor* nuevoCorredor) {

  pthread_mutex_lock(&mutexListaCorredores);

  if (listaCorredores.cabeza == NULL) {

    listaCorredores.cabeza = nuevoCorredor;
    listaCorredores.cola = nuevoCorredor;
    nuevoCorredor->siguiente = NULL;

  }

  else {

    listaCorredores.cola->siguiente = nuevoCorredor;
    listaCorredores.cola = nuevoCorredor;
    nuevoCorredor->siguiente = NULL;

  }

  pthread_mutex_unlock(&mutexListaCorredores);

}

/**
 * eliminarCorredor elimina el corredor pasador por parametro de la lista de corredores
 */
void eliminarCorredor (struct corredor* corredorAEliminar) {

  pthread_mutex_lock(&mutexListaCorredores);

  struct corredor* aux;

  if (listaCorredores.cabeza == corredorAEliminar) {
    
    listaCorredores.cabeza = listaCorredores.cabeza->siguiente;

  }

  else if (listaCorredores.cola == corredorAEliminar) {

    aux = listaCorredores.cabeza;

    while (aux->siguiente != corredorAEliminar) {

      aux = aux->siguiente;

    }

    listaCorredores.cola = aux;
    aux->siguiente = NULL;

  }

  else {

    aux = listaCorredores.cabeza;

    while ((aux->siguiente != corredorAEliminar) && (aux->siguiente != NULL)) {
   
      aux = aux->siguiente;

    }

    aux->siguiente = corredorAEliminar->siguiente;

  }

  free(corredorAEliminar);

  pthread_mutex_unlock(&mutexListaCorredores);

}

/**
 * Crea un nuevo corredor asignandole el numero que le corresponda.
 * Si ya hay 5 corredores activos en pista ignora la senal.
 */
void nuevoCorredor(){
    
  signal(SIGUSR1, SIG_IGN);  

    if (cantidadDeCorredoresActivos < maxCorredores) {

      pthread_t hcorredor;
      struct corredor* nCorredor;
      char id[13];
      int numero;
      char c_numero[3];

      nCorredor = (struct corredor*)malloc(sizeof(struct corredor));

      sprintf(c_numero, "%d", numeroDeCorredor);
      strcpy(id, "corredor_");
      strcat(id, c_numero);
      strcpy(nCorredor->id, id);

      nCorredor->numero = numeroDeCorredor;

      if(pthread_create (&hcorredor, NULL, pista, nCorredor) != 0){

        printf("Error al crear el hilo. %s\n", strerror(errno));

      }

      else {
        
        
        numeroDeCorredor++;
        cantidadDeCorredoresActivos++;
        aniadirCorredor(nCorredor);
        printf("%s ha entrado en pista.\n", nCorredor->id);

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
   * nCorredor contiene el corredor que le ha sido asignado al hilo
   */
  struct corredor* nCorredor = (struct corredor*)parametro;

  /**
   * Contiene el numero de vuetas que lleva el corredor.
   */
  int numeroDeVueltas = 0;

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

  /**
   * entrarEnBoxes si su valor es 1 entra en boxes, si no continua en pista.
   */
  int entrarEnBoxes = 0;

  while (numeroDeVueltas < NUM_VUELTAS) {

    tiempoPorVuelta = rand()%3+2;
    tiempoTotal = tiempoTotal + tiempoPorVuelta;
    sleep(tiempoPorVuelta);

    entrarEnBoxes = rand()%2;
    if (entrarEnBoxes == 1) {

      /* añadimos a la cola de boxes*/

    }

    numeroDeVueltas++;
    
  }

  if (tiempoTotal < mejorTiempo.tiempo) {

    strcpy(mejorTiempo.idCorredor, nCorredor->id);
    mejorTiempo.tiempo = tiempoTotal;

    printf("%s ha mejorado la marca con %d segundos\n", mejorTiempo.idCorredor, mejorTiempo.tiempo);

  }

  printf("%s ha acabado la carrera.\n", nCorredor->id);
  eliminarCorredor(nCorredor);
  cantidadDeCorredoresActivos--;

}
  /*
   *Parte del programa que penaliza a los corredores
   *Dormir en cuanto el juez despierte bloquear el mutex, crear el numero aleatorio
   *esperar a que todos los corredores estén esperando y que cada corredor compruebe si es su número.
   */


/*Comportamiento del juez*/

void crearJuez(){
  pthread_t hjuez;

  if (pthread_create (&hjuez, NULL, juez, NULL) != 0){
    printf("Error al crear el hilo juez %s\n ",strerror (errno));
  }
  	else ("Se ha creado el juez: \n");


}

void *juez(void* parametro){
 while(1){ 
 	
 	struct corredor *aux;
 	/*Cada corredor comprueba si ha sido sancionado.*/
 	sancionJuez = 0;
 	corredorCompruebaEntrada = 0;


 /*Se inicializan las cosas de inicializar*/


  sleep(10);


  pthread_cond_wait (&condicion, &mutexJuez);



 /*int a = *(int*)parametro;*/
  int enteroConvertible = rand()%cantidadDeCorredoresActivos+1;
  int i;
  
  /*recorres lista*/

  aux=listaCorredores.cabeza;
  for(i=1;i<cantidadDeCorredoresActivos;i++){
    aux=aux->siguiente; 

  }
 variable_comprobadora = aux->numero;


  }
}


void compruebaCorredorSancion(){
	pthread_mutex_lock(&mutexJuez);
	

	if (++corredorCompruebaEntrada >= cantidadDeCorredoresActivos){
		pthread_cond_signal (&condicion);
	}
	/*Los corredores comprueban si su número coincide con el que ha
	 *sancionado el juez, de ser así el corredor que ha sido sancionado tendrá dormir tres segundos*/

		pthread_mutex_unlock (&mutexJuez);
}





