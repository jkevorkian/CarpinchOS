#ifndef KERNEL_H_
#define KERNEL_H_

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
#include <utils/sockets.h>

//carpincho
typedef struct {
	int socket_memoria;
	int socket_mateLib;
	int rafaga_real_anterior;
	int estimacion_proxima_rafaga;
}carpincho;

t_log* logger;
t_config* config;

char* ip_memoria, puerto_memoria, ip_kernel;
int socket_memoria, socket_kernel;

char* algoritmo_planificacion;
int grado_multiprogramacion, grado_multiprocesamiento, alfa, estimacion_inicial;

t_queue* cola_new_carpinchos;

#endif /* KERNEL_H_ */
