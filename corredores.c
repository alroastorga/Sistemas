
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
 * la señal SIGALRM.
 * Para terminar el programa se debe de mandar la señal SIGINT.
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
  int tiempoPorVuelta;
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

/*El juez está listo para sancionar.*/
int sancionJuez;

/*Se crea la condicion del hilo.*/
pthread_cond_t condicion;
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

/*Mutex que controla al juez*/
pthread_mutex_t mutexJuez;

/**
 * mutexLog controla las entradas en el log.
 */
pthread_mutex_t mutexLog;

/**
 * puntero que apunta al fichero registroTiempos.log
 */
FILE *logFile;

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
void aumentarCorredores();
void aumentarBoxes ();
void aniadirCorredor (struct corredor* nuevoCorredor);
void eliminarCorredor (struct corredor* corredorAEliminar);
void nuevoCorredor();
void crearBox();
void* accionBox(void* );
void sePuedeCerrarBox();
void abrirBox();
void cerrarBox();
void aniadirListaEsperaBoxes (struct corredor* nuevoCorredor);
struct corredor* atenderCorredor();
void* pista(void* );
void crearJuez();
void* juez (void*);
void compruebaCorredorSancion();
void writeLogMessage(char *id, char *msg);
void finPrograma();


int main(int argc, char** argv){

  if (argc != 3) {

    printf("Error. Debe ingresar los argumenos numero_maximo_de_corredores y numero_de_boxes\n");
    exit(1);

  }

  else {

    maxCorredores = atoi(argv[1]);
    maxBoxes = atoi(argv[2]);

  }

  if (signal(SIGUSR1, nuevoCorredor) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

  if (signal(SIGUSR2, aumentarCorredores) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

  if (signal(SIGALRM, aumentarBoxes) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

  if (signal(SIGINT, finPrograma) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

  if (signal(SIGALRM, aumentarBoxes) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

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

  int i;
  numeroDeCorredor = 1;
  numeroDeBox = 1;
  cantidadDeCorredoresActivos = 0;
  mejorTiempo.tiempo = 1000;
  numeroDeBoxesCerrados = 0;
  seCierra = 1;
  listaCorredores.cabeza = NULL;
  listaCorredores.cola = NULL;
  listaEsperaBoxes.cabeza = NULL;
  listaEsperaBoxes.cola = NULL;

  /*
  *Creamos los boxes especificados (y los añade a listaboxes(no necesario))
  */
  for(i=0;i<maxBoxes;i++){
    crearBox();
  }

  /**
  * borra el fichero registroTiempos, si existe.
  */
  remove("registroTiempos.log");

}

void aumentarCorredores () {

  if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

  maxCorredores++;

  char mensaje[50], numCorredores[5];
  strcpy(mensaje, "Se ha modificado el nº de corredores a ");
  sprintf(numCorredores, "%d", maxCorredores);
  strcat(mensaje, numCorredores);

  writeLogMessage("Mensaje", mensaje);

  if (signal(SIGUSR2, aumentarCorredores) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

}

void aumentarBoxes () {

  if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

  maxBoxes++;

  char mensaje[50], numBoxes[5];
  strcpy(mensaje, "Se ha modificado el nº de boxes a ");
  sprintf(numBoxes, "%d", maxBoxes);
  strcat(mensaje, numBoxes);

  writeLogMessage("Mensaje", mensaje);

  crearBox();

  if (signal(SIGALRM, aumentarBoxes) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
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

  if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
  }

    if (cantidadDeCorredoresActivos < maxCorredores) {

      pthread_t hcorredor;
      struct corredor* nCorredor;
      char id[13];
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

  if (signal(SIGUSR1, nuevoCorredor) == SIG_ERR) {
    printf("Error: %s", strerror(errno));
  }

}

/**
 * Crea un box (y añadirlo a listaBoxes(no necesario))
 */
void crearBox () {

    pthread_t hBox;
    struct box* nBox;
    char id[13];
    //int numero;
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

    }
}

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

        //printf("%s ha entrado en boxes\n", nCorredor->id);
        char mensaje[40];
        strcpy(mensaje, "Entra en el ");
        strcat(mensaje, nBox->id);
        writeLogMessage(nCorredor->id, mensaje);

        corredoresAtendidos++;
        tiempoEnBoxes = rand()%3+1;
        sleep(tiempoEnBoxes);
        nCorredor->enBoxes = 0;
        nCorredor->tiempoPorVuelta = tiempoEnBoxes+ + nCorredor-> tiempoPorVuelta;

        char mensaje2[40];
        strcpy(mensaje2, "Sale del ");
        strcat(mensaje2, nBox->id);
        writeLogMessage(nCorredor->id, mensaje2);

        sePuedeCerrarBox();
        //printf("%s ha atendido a %s\n", nBox->id, nCorredor->id);
        //printf("%s ha atendido %d corredores\n", nBox->id, corredoresAtendidos);
        if((corredoresAtendidos>=3)&&(seCierra==1)){
          //printf("%s ha cerrado\n", nBox->id);
          writeLogMessage(nBox->id, "Cierra");

          cerrarBox();
          sleep(20);
          corredoresAtendidos = 0;
          abrirBox();
          //printf("%s ha abierto\n", nBox->id);
          writeLogMessage(nBox->id, "Abre");
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
   * tiempoTotal contiene el tiempo que ha tardado el corredor en dar las 5 vueltas.
   */
  int tiempoTotal = 0;

  /**
   * entrarEnBoxes si su valor es 1 entra en boxes, si no continua en pista.
   */
  int entrarEnBoxes = 0;

  int tieneProblemasGraves = 0;

  while (numeroDeVueltas < NUM_VUELTAS) {

    nCorredor->tiempoPorVuelta = rand()%4+2;
    tiempoTotal = tiempoTotal + nCorredor->tiempoPorVuelta;
    sleep(nCorredor->tiempoPorVuelta);

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
        //printf("El corredor %s tiene problemas graves y ha abandonado\n",nCorredor->id);
        writeLogMessage(nCorredor->id, "Abandona la carrera por problemas graves");
        eliminarCorredor(nCorredor);
        cantidadDeCorredoresActivos--;
        pthread_exit(NULL);

      }
    }
    numeroDeVueltas++;

    // El corredor termina una vuelta.

    char mensaje[50], tiempoVuelta[50], numVuelta[50];

    sprintf(numVuelta, "%d", numeroDeVueltas);
    sprintf(tiempoVuelta, "%d", nCorredor->tiempoPorVuelta);

    strcpy(mensaje, "Termina la vuelta ");
    strcat(mensaje, numVuelta);
    strcat(mensaje, " en ");
    strcat(mensaje, tiempoVuelta);
    strcat(mensaje, " segundos.");

    writeLogMessage(nCorredor->id, mensaje);
    //printf("%s: %s\n", nCorredor->id, mensaje);

  }

   // El corredor termina la carrera (da 5 vueltas)

    writeLogMessage(nCorredor->id, "Termina la carrera");


  if (tiempoTotal < mejorTiempo.tiempo) {

    strcpy(mejorTiempo.idCorredor, nCorredor->id);
    mejorTiempo.tiempo = tiempoTotal;

    char mensaje[50], tiempo[5];
    strcpy(mensaje, "Ha mejorado la marca con ");
    sprintf(tiempo, "%d", mejorTiempo.tiempo);
    strcat(mensaje, tiempo);
    strcat(mensaje, " segundos");

    //printf("%s ha mejorado la marca con %d segundos\n", mejorTiempo.idCorredor, mejorTiempo.tiempo);
    writeLogMessage(mejorTiempo.idCorredor, mensaje);

  }

  //printf("%s ha acabado la carrera.\n", nCorredor->id);

  eliminarCorredor(nCorredor);
  cantidadDeCorredoresActivos--;

  pthread_exit(NULL);

}

/*Comportamiento del juez*/

void crearJuez(){
  pthread_t hjuez;

  if (pthread_create (&hjuez, NULL, juez, NULL) != 0){
    printf("Error al crear el hilo juez %s\n ",strerror (errno));
  }
  	else  {

      printf("Se ha creado el juez.\n");

    }

}

/*
 *Parte del programa que penaliza a los corredores
 *Dormir en cuanto el juez despierte bloquear el mutex, crear el numero aleatorio
 *esperar a que todos los corredores estén esperando y que cada corredor compruebe si es su número.
 */
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

  //printf("El juez sanciona al corredor: %s", aux->id);
  writeLogMessage(aux->id, "Ha sido sancionado por el juez");

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

/**
* Funcion para escribir en el fichero de logs.
* char *id: cadena que repreenta el id del corredor o del box.
* char *msg: mensaje que indica la accion realizada.
*/
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

/**
* finPrograma captura la señal SIGINT cuando se pulsa Ctrl+C y
* escribe en el fichero de log el ganador de la carrera y el número de corredores
* que han participado.
*/
void finPrograma() {

  if (signal(SIGINT, finPrograma) == SIG_ERR) {
    printf("Error: %s\n", strerror(errno));
    exit(-1);
  }

  if (numeroDeCorredor > 0) {

    char corredores[15];
    sprintf(corredores, "%d", (numeroDeCorredor-1));

    //printf("Ganador se la carrera: %s\n", mejorTiempo.idCorredor);
    //printf("Número de corredires: %d\n", (numeroDeCorredor-1));
    writeLogMessage(mejorTiempo.idCorredor, "Ha ganado la carrera");
    writeLogMessage("Número de corredores", corredores);

  }

  exit(0);

}
