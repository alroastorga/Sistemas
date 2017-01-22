
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
  int enBoxes;
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

/**
 * box es la estructura que tiene un box.
 * id contiene el id que representa a cada box.
 * numero es el numero que ha sido asignado a cada box.
 * corredoresAtendidos lleva la cuenta de los corredores atendidos por el box.
 * siguiente contiene la direccion de memoria del siguiente box.
 */
struct box {

  char id[13];
  int numero;
  int corredoresAtendidos;
  struct box* siguiente;

} box;

/**
 * esperaBoxes es la estructura que tiene un corredor que quiere entrar a boxes.
 * corredorEnEspera contiene la direccion de memoria de un corredor.
 * siguiente contiene la direccion de memoria del corredor que entrara despues.
 */
struct esperaBoxes {

  struct corredor* corredorEnEspera;
  struct esperaBoxes* siguiente;

} esperaBoxes;


/**
 * listaBoxes lista de boxes creados
 * cabeza contiene la direccion de memoria del primer box.
 * cola contiene la direccion de memoria del ultimo box.

struct listaBoxes {

  struct box* cabeza;
  struct box* cola;

} listaBoxes;*/

/**
 * listaEsperaBoxes lista de corredores que quieren parar en boxes.
 * cabeza contiene la direccion de memoria del primer corredor en parar.
 * cola contiene la direccion de memoria del ultimo corredor que quiere parar.
 */
struct listaEsperaBoxes {

  struct esperaBoxes* cabeza;
  struct esperaBoxes* cola;

} listaEsperaBoxes;

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
 * numeroDeBox indica el numero que le corresponde a cada box
 */
int numeroDeBox;

/**
 * Indica la cantidad de corredores que estan dentro de la pista
 */
int cantidadDeCorredoresActivos;

/**
 *Indica el numero de boxes cerrados
 */
int numeroDeBoxesCerrados;

/**
 *Toma valores 0/1 si no se puede/se puede cerrar un box segun numeroDeBoxesCerrados que haya.
 */
int seCierra;

/**
 * mutexListaCorredores controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaCorredores;

/**
 * mutexListaBoxes controla las modificaciones de la lista

pthread_mutex_t mutexListaBoxes;*/

/**
 * mutexListaEsperaBoxes controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaEsperaBoxes;

/**
 * mutexListaCorredoresENEspera controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaCorredoresEnEspera;

/**
 * mutexListaBoxes controla las modificaciones de la variable global numeroDeBoxesCerrados y seCierra
 */
pthread_mutex_t mutexBoxesCerrados;


void init ();
void aniadirCorredor (struct corredor* nuevoCorredor);
void eliminarCorredor (struct corredor* corredorAEliminar);
void nuevoCorredor();
void* pista(void* );
void crearBox();
void* accionBox(void* );
/*void aniadirListaBoxes (struct box* nuevoBox);*/
void aniadirListaEsperaBoxes (struct corredor* nuevoCorredor);
struct corredor* atenderCorredor();
void sePuedeCerrarBox();
void abrirBox();
void cerrarBox();

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

  int i;
  numeroDeCorredor = 1;
  numeroDeBox = 1;
  cantidadDeCorredoresActivos = 0;
  mejorTiempo.tiempo = 100;
  numeroDeBoxesCerrados = 0;
  seCierra = 1;
  listaCorredores.cabeza = NULL;
  listaCorredores.cola = NULL;
  /*listaBoxes.cabeza = NULL;
  listaBoxes.cola = NULL;*/
  listaEsperaBoxes.cabeza = NULL;
  listaEsperaBoxes.cola = NULL;

  /*
  *Creamos los boxes especificados (y los añade a listaboxes(no necesario))
  */
  for(i=0;i<maxBoxes;i++){
    crearBox();
  }

}

/**
 * Añade en la cola un nuevo corredor pasador por parametro
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
 * Crea un box (y añadirlo a listaBoxes(no necesario))
 */
void crearBox () {

    pthread_t hBox;
    struct box* nBox;
    char id[13];
    int numero;
    char box_numero[3];

    nBox = (struct box*)malloc(sizeof(struct box));

    sprintf(box_numero, "%d", numeroDeBox);
    strcpy(id, "box_");
    strcat(id, box_numero);
    strcpy(nBox->id, id);

    nBox->numero = numeroDeBox;

    if(pthread_create (&hBox, NULL, accionBox, nBox) != 0){

      printf("Error al crear el hilo. %s\n", strerror(errno));

    }

    else {
    //printf("Box creado: %s\n ", nBox->id);
      numeroDeBox++;
    //  aniadirListaBoxes(nBox);

    }
}

/**
 * aniadirListaBoxes añade en la cola cada box que se cree

void aniadirListaBoxes (struct box* nuevoBox) {

  pthread_mutex_lock(&mutexListaBoxes);

  if (listaBoxes.cabeza == NULL) {

    listaBoxes.cabeza = nuevoBox;
    listaBoxes.cola = nuevoBox;
    nuevoBox->siguiente = NULL;

  }

  else {

    listaBoxes.cola->siguiente = nuevoBox;
    listaBoxes.cola = nuevoBox;
    nuevoBox->siguiente = NULL;

  }
  printf("%s añadido a listaBoxes\n", nuevoBox->id);
  pthread_mutex_unlock(&mutexListaBoxes);

}*/

/**
 *
 */
void *accionBox(void* parametro){

  struct box* nBox = (struct box*)parametro;
  struct corredor* nCorredor;

  int corredoresAtendidos = 0;

  int tiempoEnBoxes = 0;

  while(1) {
    if (listaEsperaBoxes.cabeza == NULL) {
      sleep(1);
    }else{
        nCorredor = atenderCorredor();
        printf("%s ha entrado en boxes\n", nCorredor->id);
        corredoresAtendidos++;
        tiempoEnBoxes = rand()%3+1;
        sleep(tiempoEnBoxes);
        nCorredor->enBoxes = 0;
        sePuedeCerrarBox();
        //printf("%s ha atendido a %s\n", nBox->id, nCorredor->id);
        //printf("%s ha atendido %d corredores\n", nBox->id, corredoresAtendidos);
        if((corredoresAtendidos>=3)&&(seCierra==1)){
          printf("%s ha cerrado\n", nBox->id);
          cerrarBox();
          sleep(20);
          corredoresAtendidos = 0;
          abrirBox();
          printf("%s ha abierto\n", nBox->id);
        }
    }
  }
}

/**
 *Comprueba si se puede cerrar el box o no
 *para asegurar que haya al menos uno abierto.
 */
void sePuedeCerrarBox(){
  pthread_mutex_lock(&mutexBoxesCerrados);

  if(numeroDeBoxesCerrados<maxBoxes-1){
    seCierra = 1;
  }else{
    seCierra = 0;
  }
  pthread_mutex_unlock(&mutexBoxesCerrados);
}

/**
 *Decrementa el contador de boxes cerrados.
 */
void abrirBox(){
  pthread_mutex_lock(&mutexBoxesCerrados);
  numeroDeBoxesCerrados--;
  pthread_mutex_unlock(&mutexBoxesCerrados);
}

/**
 *Incrementa el contador de boxes cerrados
 */
void cerrarBox(){
  pthread_mutex_lock(&mutexBoxesCerrados);
  numeroDeBoxesCerrados++;
  pthread_mutex_unlock(&mutexBoxesCerrados);
}


/**
 * aniadirListaEsperaBoxes añade en la cola un nuevo corredor que tiene que parar en boxes
 */
void aniadirListaEsperaBoxes (struct corredor* nuevoCorredor) {

  pthread_mutex_lock(&mutexListaEsperaBoxes);

  struct esperaBoxes* nesperaboxes = (struct esperaBoxes*)malloc(sizeof(struct esperaBoxes));
  nesperaboxes->corredorEnEspera = nuevoCorredor;

  if (listaEsperaBoxes.cabeza == NULL) {

    listaEsperaBoxes.cabeza = nesperaboxes;
    listaEsperaBoxes.cola = nesperaboxes;
    nesperaboxes->siguiente = NULL;

  }

  else {

    listaEsperaBoxes.cola->siguiente = nesperaboxes;
    listaEsperaBoxes.cola = nesperaboxes;
    nesperaboxes->siguiente = NULL;

  }

  //printf("%s quiere entrar en boxes\n", nesperaboxes->corredorEnEspera->id);

  pthread_mutex_unlock(&mutexListaEsperaBoxes);

}

/**
 * atenderCorredor elimina el primer corredor de la lista de espera
 */
struct corredor* atenderCorredor() {

  pthread_mutex_lock(&mutexListaCorredoresEnEspera);

  struct corredor* corredorAtendido = listaEsperaBoxes.cabeza->corredorEnEspera;

  listaEsperaBoxes.cabeza = listaEsperaBoxes.cabeza->siguiente;

  pthread_mutex_unlock(&mutexListaCorredoresEnEspera);

  return corredorAtendido;

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

  int tieneProblemasGraves = 0;

  while (numeroDeVueltas < NUM_VUELTAS) {

    tiempoPorVuelta = rand()%3+2;
    tiempoTotal = tiempoTotal + tiempoPorVuelta;
    sleep(tiempoPorVuelta);

    entrarEnBoxes = rand()%2;
    if (entrarEnBoxes == 1) {

      // añadimos a la cola de boxes
      aniadirListaEsperaBoxes(nCorredor);
      nCorredor->enBoxes=1;
      while(nCorredor->enBoxes==1) {

        sleep(1);

      }

      tieneProblemasGraves = rand()%10+1;
      if(tieneProblemasGraves>7){
        printf("El corredor %s tiene problemas graves y ha abandonado\n",nCorredor->id);
        eliminarCorredor(nCorredor);
        cantidadDeCorredoresActivos--;
        pthread_exit((void*) "me mori");
      }
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
