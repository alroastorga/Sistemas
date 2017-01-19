
/**
 * @author Alvaro Villar Marcos
 * @author Manuel Ugidos Fernandez
 * @author Juan Ramon Lastra Diaz
 * @author Juan Carlos Robles Fernandez
 */

/**
 * Para aumentar el numero maximo de corredores con el programa en ejecucion se ha modificado
 * la señal SIGUSR2.
 * Para aumentar el numero maximo de boxes con el programa en ejecucion se ha modificado la
 * la señal SIGVTALRM.
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

/**
 * mutexListaCorredores controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaCorredores;

/**
 * mutexLog controla las entradas en el log.
 */
pthread_mutex_t mutexLog;

/**
 * puntero que apunta al fichero registroTiempos.log
 */
FILE *logFile;

void init ();
void aniadirCorredor (struct corredor* nuevoCorredor);
void eliminarCorredor (struct corredor* corredorAEliminar);
void nuevoCorredor();
void* pista(void* );
void writeLogMessage(char *id, char *msg);
void aumentarCorredores();
void aumentarBoxes ();

int main(int argc, char** argv){

  if (argc != 3) {

    printf("Error. Debe ingresar los argumenos numero_maximo_de_corredores y numero_de_boxes\n");
    exit(1);
  }

  else {

    maxCorredores = atoi(argv[1]);
    maxBoxes = atoi(argv[2]);

  }
      
  signal(SIGUSR1, nuevoCorredor);
  signal(SIGUSR2, aumentarCorredores);
  signal(SIGVTALRM, aumentarBoxes);
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

  numeroDeCorredor = 1;
  cantidadDeCorredoresActivos = 0;
  mejorTiempo.tiempo = 100;
  listaCorredores.cabeza = NULL;
  listaCorredores.cola = NULL;

}

void aumentarCorredores () {

  signal(SIGUSR2, SIG_IGN);

  maxCorredores++;

  signal(SIGUSR2, aumentarCorredores);

}

void aumentarBoxes () {

  signal(SIGVTALRM, SIG_IGN);

  maxBoxes++;
  /*AÑADIR UN NUEVO BOX A LA LISTA*/

  signal(SIGVTALRM, aumentarBoxes);

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
        //printf("%s ha entrado en pista.\n", nCorredor->id);
        
        
        writeLogMessage(nCorredor->id, "Entra en el circuito");
        
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

      // añadimos a la cola de boxes

    }

    numeroDeVueltas++;  
    
    // El corredor termina una vuelta.
    
    char mensaje[50], tiempoVuelta[50], numVuelta[50];

    sprintf(numVuelta, "%d", numeroDeVueltas);
    sprintf(tiempoVuelta, "%d", tiempoPorVuelta);

    strcpy(mensaje, "Termina la vuelta ");
    strcat(mensaje, numVuelta);
    strcat(mensaje, " en ");
    strcat(mensaje, tiempoVuelta);
    strcat(mensaje, " segundos.");
    
    writeLogMessage(nCorredor->id, mensaje);
    
  }

   // El corredor termina la carrera (da 5 vueltas)
   
   writeLogMessage(nCorredor->id, "Abandona el circuito");


  if (tiempoTotal < mejorTiempo.tiempo) {

    strcpy(mejorTiempo.idCorredor, nCorredor->id);
    mejorTiempo.tiempo = tiempoTotal;

    printf("%s ha mejorado la marca con %d segundos\n", mejorTiempo.idCorredor, mejorTiempo.tiempo);

  }

  //printf("%s ha acabado la carrera.\n", nCorredor->id);
 
  eliminarCorredor(nCorredor);
  cantidadDeCorredoresActivos--;

}


void writeLogMessage(char *id, char *msg) {
   
   pthread_mutex_lock(&mutexLog);

   // Calculamos la hora actual
   time_t now = time(0);
   struct tm *tlocal = localtime(&now);
   char stnow[19];
   strftime(stnow, 19, "%d/%m/%y  %H:%M:%S", tlocal);

   // Escribimos en el log
   logFile = fopen("registroTiempos.log", "a");
   fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
   fclose(logFile);

   pthread_mutex_unlock(&mutexLog);
   
}



