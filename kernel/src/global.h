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


#define LOGUEAR_MENSAJES_INICIALIZADOR 0
#define LOGUEAR_MENSAJES_DEADLOCK 1

#define INFORMADOR_LISTAS 1

int MEMORIA_ACTIVADA, DEADLOCK_ACTIVADO, LOGUEAR_MENSAJES_COLAS;
int mate_init, mate_close;

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
	//cosas del deadlock
	t_list *semaforos_asignados;
	int id_semaforo_bloqueante; //es -1 cuando no esta siendo bloqueado por espera de un semaforo
	bool debe_morir;
	bool esperar_cliente;
}carpincho;


//semaforo
typedef struct {
	char *nombre;
	int id; //para manejo inequivoco de semaforos en el deadlock
	int instancias_iniciadas;
	t_queue *cola_espera;
	pthread_mutex_t mutex_espera;
}semaforo;

//DEADLOCK: struct que contiene un semaforo y la cantidad de veces que se lo asignó a un mismo carpincho
typedef struct {
	semaforo* sem;
	int cantidad_asignada;
}sem_deadlock;

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
int grado_multiprogramacion, grado_multiprocesamiento;
double alfa, estimacion_inicial;

int id_proximo_carpincho, id_proximo_semaforo;

t_queue *cola_new, *cola_suspendidosReady, *cola_running;
t_list *lista_ready, *hilos_cpu, *lista_blocked, *lista_suspendidosBlocked, *lista_semaforos, *lista_IO;

sem_t carpinchos_new, carpinchos_ready, carpinchos_running;
sem_t multiprogramacion, multiprocesamiento;

pthread_mutex_t mutex_cola_new, mutex_lista_ready, mutex_lista_running, mutex_lista_blocked, mutex_cola_suspendidosReady, mutex_lista_suspendidosBlocked, mutex_lista_semaforos;

pthread_t hilo_planificador_largo_plazo, hilo_planificador_corto_plazo;

/////////////DEADLOCK/////////////
int tiempo_deadlock;
pthread_t detector;

t_list *carpinchos_en_deadlock;
t_list *lista_a_evaluar;

/////////////INFORMADOR_LISTAS/////////////
pthread_t hilo_informador;


#endif /* GLOBAL_H_ */
