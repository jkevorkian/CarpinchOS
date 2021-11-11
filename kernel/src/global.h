#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include <utils/sockets.h>

#define MEMORIA_ACTIVADA 0

#define LOGUEAR_MENSAJES_INICIALIZADOR 0
#define LOGUEAR_MENSAJES_COLAS 1

//carpincho
typedef struct {
	int id;
	int socket_memoria;
	int socket_mateLib;
	int rafaga_real_anterior;
	double estimacion_proxima_rafaga;
	char *tiempo_llegada;
	bool esta_suspendido;
	bool responder;
}carpincho;

//semaforo
typedef struct {
	char *nombre;
	int instancias_iniciadas;
	t_queue *cola_espera;
	pthread_mutex_t mutex_espera;
}semaforo;

//IO
typedef struct {
	char *nombre;
	int duracion;
	t_queue *cola_espera;
	pthread_mutex_t mutex_espera;
	sem_t carpinchos_esperando;
	pthread_t hilo_IO;
}IO;

t_log *logger, *logger_colas;
t_config *config;

char *ip_memoria, *puerto_memoria, *ip_kernel;
int socket_memoria, socket_kernel;

int id_proximo_carpincho;

/////////////PLANIFICACION/////////////

char *algoritmo_planificacion;
int grado_multiprogramacion, grado_multiprocesamiento, estimacion_inicial;
double alfa;

t_queue *cola_new, *cola_suspendidosReady, *cola_running;
t_list *lista_ready, *lista_blocked, *lista_suspendidosBlocked;

sem_t carpinchos_new, carpinchos_ready, carpinchos_running;
sem_t multiprogramacion, multiprocesamiento;

pthread_mutex_t mutex_cola_new, mutex_cola_suspendidosReady;
pthread_mutex_t mutex_lista_ready, mutex_lista_running, mutex_lista_blocked, mutex_lista_suspendidosBlocked;

pthread_t hilo_planificador_largo_plazo, hilo_planificador_corto_plazo;

/////////////CPU/////////////

t_list *hilos_cpu, *lista_semaforos, *lista_IO;
pthread_mutex_t mutex_lista_semaforos;

#endif /* GLOBAL_H_ */
