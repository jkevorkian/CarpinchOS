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
#include <utils/utils-client.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#define RAM_ACTIVADA 1
#define MONGO_ACTIVADO 1

//tripulante
typedef struct {
	int posicion[2];
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc

	pthread_t hilo;

	int socket_ram;
	int socket_mongo;

	sem_t sem_blocked;
	sem_t sem_running;

	int contador_ciclos;
	bool continuar;
}tripulante;

//estado
typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT,
	EMERGENCY
}estado;

//tareas
typedef enum {
	GENERAR_OXIGENO,
	CONSUMIR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA,
	ESPERAR
}tareas;

//parametros
typedef struct {
	int cantidad_tripulantes;
	int* posiciones_x;
	int* posiciones_y;
	int cantidad_tareas;
	char** tareas;
}parametros_iniciar_patota;

t_log* logger;
t_config* config;
bool salir;
bool hay_sabotaje;
bool continuar_planificacion;
bool analizar_quantum;
int ciclo_CPU;
int quantum;

pthread_t hilo_planificador;
pthread_t hilo_planificador_io;
pthread_t hilo_detector_sabotaje;

t_list* cola_ready;
t_list* cola_blocked;
t_list* cola_emergencia;
t_list* tripulantes_running;
t_list* lista_tripulantes;
tripulante* trip_block;

pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_tripulantes_running;
pthread_mutex_t mutex_cola_blocked;

sem_t activar_planificacion;
sem_t multiprocesamiento;
sem_t tripulantes_ready;
sem_t io_disponible;
sem_t tripulantes_blocked;

sem_t sabotaje_pausado;
sem_t finalizo_sabotaje;

#endif /* GLOBAL_H_ */
