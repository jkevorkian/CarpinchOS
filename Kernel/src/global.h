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
#include <commons/config.h>
#include <commons/temporal.h>
#include <utils/sockets.h>

//carpincho
typedef struct {
	int socket_memoria;
	int socket_mateLib;
	int rafaga_real_anterior;
	double estimacion_proxima_rafaga;
	char *tiempo_llegada;
}carpincho;

t_log *logger;
t_config *config;

char *ip_memoria, *puerto_memoria, *ip_kernel;
int socket_memoria, socket_kernel;

char *algoritmo_planificacion;
int grado_multiprogramacion, grado_multiprocesamiento, estimacion_inicial;
double alfa;

t_queue *cola_new, *cola_suspendidosReady, *cola_running;
t_list *lista_ready, *hilos_cpu;

sem_t carpinchos_new, carpinchos_ready, carpinchos_running;
sem_t multiprogramacion, multiprocesamiento;

pthread_mutex_t mutex_cola_new, mutex_lista_ready, mutex_lista_running, mutex_cola_suspendidosReady;

pthread_t hilo_planificador_largo_plazo, hilo_planificador_corto_plazo, hilo_planificador_mediano_plazo;

#endif /* GLOBAL_H_ */
